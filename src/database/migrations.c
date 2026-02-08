/*
 * Reachard
 * Copyright 2026 Pavel Sobolev
 *
 * This file is part of the Reachard project, located at
 * <https://reachard.paveloom.dev/>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>

#include "database.h"

#include "migrations.h"

static int
reachard_db_get_schema_version(struct reachard_db *db) {
    PGresult *res = NULL;

    res = PQexec(
        db->conn,
        "SELECT 1 FROM information_schema.tables WHERE "
        "table_schema = '" REACHARD_DB_SCHEMA "' AND "
        "table_name = '" REACHARD_DB_VERSION_TABLE "'"
    );
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(
            stderr,
            "failed to check the existence of the version table\n"
        );
        fprintf(stderr, "%s", PQresultErrorMessage(res));
        goto failure;
    }

    if (PQntuples(res) == 0) {
        PQclear(res);
        return 0;
    }
    PQclear(res);

    res = PQexec(db->conn, "SELECT value from version");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(
            stderr,
            "failed to retrieve the schema version from the version table\n"
        );
        fprintf(stderr, "%s", PQresultErrorMessage(res));
        goto failure;
    }

    const char *version_str = PQgetvalue(res, 0, 0);
    const int version = atoi(version_str);
    PQclear(res);

    return version;

failure:
    PQclear(res);
    return -1;
}

static bool
reachard_db_apply_migration(
    struct reachard_db *db,
    const int version,
    const char *migration
) {
    PGresult *res = NULL;

    res = PQexec(db->conn, "BEGIN");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "failed to start the transaction\n");
        goto failure;
    }
    PQclear(res);

    res = PQexec(db->conn, migration);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "failed to apply the migration\n");
        fprintf(stderr, "%s", PQresultErrorMessage(res));
        goto failure;
    }
    PQclear(res);

    char version_str[20] = {0};
    snprintf(version_str, sizeof(version_str), "%d", version);

    const char *paramValues = {version_str};
    res = PQexecParams(
        db->conn, "UPDATE " REACHARD_DB_VERSION_TABLE " SET value = $1",
        1, NULL, &paramValues, NULL, NULL, 0
    );
    PQclear(res);

    res = PQexec(db->conn, "END");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "%s", "failed to end the transaction\n");
        goto failure;
    }
    PQclear(res);

    return true;

failure:
    PQclear(res);
    return false;
}

bool
reachard_db_migrate(struct reachard_db *db) {
    const int current_version = reachard_db_get_schema_version(db);
    if (current_version < 0) {
        fprintf(stderr, "failed to get the schema version\n");
        return false;
    }

    const char *migrations[] = {
        [1] = "CREATE SCHEMA " REACHARD_DB_SCHEMA ";\n"
              "CREATE TABLE " REACHARD_DB_VERSION_TABLE " (\n"
              "    value INTEGER PRIMARY KEY\n"
              ");\n"
              "INSERT INTO " REACHARD_DB_VERSION_TABLE " (value)\n"
              "VALUES (1)"
    };
    const int n_migrations = sizeof(migrations) / sizeof(migrations[0]);

    for (
        int version = current_version + 1; version < n_migrations; version++
    ) {
        const char *migration = migrations[version];
        if (!reachard_db_apply_migration(db, version, migration)) {
            fprintf(
                stderr,
                "failed to apply migration to version %d\n",
                version
            );
            return false;
        }
    }

    return true;
}
