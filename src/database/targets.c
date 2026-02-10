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

#include <stdlib.h>
#include <string.h>

#include <libpq-fe.h>

#include "database.h"

#include "targets.h"

void
reachard_db_targets_free(struct reachard_db_target *targets, size_t count) {
    for (size_t i = 0; i < count; i++) {
        free(targets[i].name);
    }
    free(targets);
}

int
reachard_db_targets_add(struct reachard_db *db, const char *name) {
    PGresult *res = 0;

    const char *paramValues[1] = {name};
    res = PQexecParams(
        db->conn, "INSERT INTO targets VALUES (DEFAULT, $1) RETURNING id",
        1, 0, paramValues, 0, 0, 0
    );
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "failed to add a target\n");
        PQclear(res);
        return 0;
    }

    const char *id_str = PQgetvalue(res, 0, 0);
    const int id = atoi(id_str);
    PQclear(res);

    return id;
}

bool
reachard_db_targets_delete(struct reachard_db *db, const int id) {
    PGresult *res = 0;

    char id_str[20] = {0};
    snprintf(id_str, sizeof(id_str), "%d", id);

    const char *paramValues[1] = {id_str};
    res = PQexecParams(
        db->conn, "DELETE FROM targets WHERE id = $1",
        1, 0, paramValues, 0, 0, 0
    );
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "failed to delete a target\n");
        PQclear(res);
        return false;
    }
    PQclear(res);

    return true;
}

bool
reachard_db_targets_get(
    struct reachard_db *db,
    struct reachard_db_target **targets,
    size_t *count
) {
    PGresult *res = 0;

    res = PQexec(db->conn, "SELECT id, name FROM targets");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "failed to get targets\n");
        PQclear(res);
        return 1;
    }

    const int ntargets = PQntuples(res);
    if (!ntargets) {
        PQclear(res);
        return 0;
    }

    *count = ntargets;
    *targets = calloc(ntargets, sizeof(struct reachard_db_target));

    for (int i = 0; i < ntargets; i++) {
        (*targets)[i].id = atoi(PQgetvalue(res, i, 0));
        (*targets)[i].name = strdup(PQgetvalue(res, i, 1));
    }

    PQclear(res);
    return 0;
}
