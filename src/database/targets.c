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

#include "targets.h"

#include <database/database.h>
#include <database/macros.h>
#include <utils/constants.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libpq-fe.h>
#include <utstring.h>

static void
parse_target(struct reachard_db_target *s, PGresult *res, int col) {
    FOR_FIELDS(DEFINE_PARSE_CALL)
}

void
reachard_db_target_init(struct reachard_db_target *target) {
    memset(target, 0, sizeof(*target));
    target->interval = 5;
    target->up = true;
}

void
reachard_db_target_free(struct reachard_db_target *target) {
    free(target->name);
    free(target->url);
}

void
reachard_db_targets_free(struct reachard_db_target *targets, size_t count) {
    for (size_t i = 0; i < count; i++) {
        reachard_db_target_free(&targets[i]);
    }
    free(targets);
}

int
reachard_db_targets_add(
    struct reachard_db *db,
    struct reachard_db_target target
) {
    PGresult *res = 0;

    char interval[REACHARD_INT_STR_LEN];
    snprintf(interval, sizeof(interval), "%d", target.interval);

    const char *paramValues[] = {target.name, target.url, interval};
    const size_t paramValuesN = sizeof(paramValues) / sizeof(paramValues[0]);

    res = PQexecParams(
        db->conn, "INSERT INTO targets\n"
                  "VALUES (DEFAULT, $1, $2, $3, DEFAULT)\n"
                  "RETURNING id",
        paramValuesN, 0, paramValues, 0, 0, 0
    );
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "%s", PQresultErrorMessage(res));
        fprintf(stderr, "failed to add a target\n");
        PQclear(res);
        return 1;
    }

    const char *id_str = PQgetvalue(res, 0, 0);
    const int id = atoi(id_str);
    PQclear(res);

    return id;
}

int
reachard_db_targets_delete(struct reachard_db *db, const int id) {
    PGresult *res = 0;

    char id_str[REACHARD_INT_STR_LEN];
    snprintf(id_str, sizeof(id_str), "%d", id);

    const char *paramValues[1] = {id_str};
    res = PQexecParams(
        db->conn, "DELETE FROM targets WHERE id = $1",
        1, 0, paramValues, 0, 0, 0
    );
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "%s", PQresultErrorMessage(res));
        fprintf(stderr, "failed to run the delete query\n");
        PQclear(res);
        return 1;
    }
    PQclear(res);

    return 0;
}

int
reachard_db_targets_get(
    struct reachard_db *db,
    struct reachard_db_target *target,
    int id
) {
    PGresult *res = 0;

    char id_str[REACHARD_INT_STR_LEN];
    snprintf(id_str, sizeof(id_str), "%d", id);

    const char *paramValues[1] = {id_str};
    res = PQexecParams(
        db->conn, "SELECT * FROM targets WHERE id = $1",
        1, 0, paramValues, 0, 0, 0
    );
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "%s", PQresultErrorMessage(res));
        fprintf(stderr, "failed to run the select query\n");
        PQclear(res);
        return 1;
    }

    const int ntargets = PQntuples(res);
    if (!ntargets) {
        fprintf(stderr, "there is no such target\n");
        PQclear(res);
        return 1;
    }

    parse_target(target, res, 0);

    PQclear(res);
    return 0;
}

int
reachard_db_targets_get_all(
    struct reachard_db *db,
    struct reachard_db_target **targets,
    size_t *count
) {
    PGresult *res = 0;

    res = PQexec(db->conn, "SELECT * FROM targets ORDER BY id");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "%s", PQresultErrorMessage(res));
        fprintf(stderr, "failed to run the select query\n");
        PQclear(res);
        return 1;
    }

    const int ntargets = PQntuples(res);
    if (!ntargets) {
        *targets = 0;
        *count = 0;
        PQclear(res);
        return 0;
    }

    *count = ntargets;
    *targets = calloc(ntargets, sizeof(struct reachard_db_target));
    if (!*targets) {
        fprintf(stderr, "failed to allocate memory for targets\n");
        return 1;
    }

    for (int i = 0; i < ntargets; i++) {
        parse_target(&(*targets)[i], res, i);
    }

    PQclear(res);
    return 0;
}

int
reachard_db_targets_update(
    struct reachard_db *db,
    int id,
    struct reachard_db_target_kv kvs[],
    size_t count
) {
    enum reachard_db_target_k key;
    union reachard_db_value value;

    UT_string *query;
    utstring_new(query);

    utstring_printf(query, "UPDATE targets SET ");

    char *field_names[] = {
        FOR_FIELDS(DEFINE_FIELD_NAME)
    };
    for (size_t i = 0; i < count; i++) {
        key = kvs[i].key;
        utstring_printf(query, "%s = $%ld, ", field_names[key], i + 1);
    }

    // Replace the last comma with a space
    (query->d)[query->i - 2] = 32;

    utstring_printf(query, "WHERE id = %d", id);

    const char *paramValues[count];
    for (size_t i = 0; i < count; i++) {
        key = kvs[i].key;
        value = kvs[i].value;

        switch (key) {
            FOR_FIELDS(DEFINE_ENCODE_CALL)
        }
    }

    PGresult *res = PQexecParams(
        db->conn, utstring_body(query),
        count, 0, paramValues, 0, 0, 0
    );
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "%s", PQresultErrorMessage(res));
        fprintf(stderr, "failed to update a target\n");
        PQclear(res);
        utstring_free(query);
        return 1;
    }

    PQclear(res);
    utstring_free(query);

    return 0;
}
