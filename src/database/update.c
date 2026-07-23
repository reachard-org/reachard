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

#include "update.h"

#include <database/database.h>
#include <utils/constants.h>

#include <libpq-fe.h>
#include <utarray.h>
#include <utstring.h>

#include <stdio.h>

void
reachard_db_update_init(
    struct reachard_db_update *update,
    struct reachard_db *db,
    char *table,
    int id
) {
    update->db = db;
    update->id = id;

    utstring_new(update->query);

    // This `icd` dupes all strings
    utarray_new(update->params, &ut_str_icd);
    utstring_printf(update->query, "UPDATE %s SET ", table);
}

void
reachard_db_update_string(
    struct reachard_db_update *update,
    char *name,
    char *string
) {
    utarray_push_back(update->params, &string);
    utstring_printf(
        update->query,
        "%s = $%d, ", name, utarray_len(update->params)
    );
}

void
reachard_db_update_number(
    struct reachard_db_update *update,
    char *name,
    int number
) {
    char buffer[REACHARD_INT_STR_LEN];
    snprintf(buffer, sizeof(buffer), "%d", number);

    utarray_push_back(update->params, &(char *){buffer});
    utstring_printf(
        update->query,
        "%s = $%d, ", name, utarray_len(update->params)
    );
}

void
reachard_db_update_boolean(
    struct reachard_db_update *update,
    char *name,
    bool boolean
) {
    char *string = boolean ? "t" : "f";

    utarray_push_back(update->params, &string);
    utstring_printf(
        update->query,
        "%s = $%d, ", name, utarray_len(update->params)
    );
}

int
reachard_db_update_commit(struct reachard_db_update *update) {
    if (utarray_len(update->params) == 0) {
        return 0;
    }

    // Replace the last comma with a space
    (update->query->d)[update->query->i - 2] = 32;

    utstring_printf(update->query, "WHERE id = %d", update->id);

    PGresult *res = PQexecParams(
        update->db->conn, utstring_body(update->query),
        utarray_len(update->params), 0, utarray_front(update->params), 0, 0, 0
    );
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "%s", PQresultErrorMessage(res));
        fprintf(stderr, "failed to commit an update\n");
        PQclear(res);
        return 1;
    }

    PQclear(res);
    return 0;
}

void
reachard_db_update_deinit(struct reachard_db_update *update) {
    utstring_free(update->query);
    utarray_free(update->params);
}
