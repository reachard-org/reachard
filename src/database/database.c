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

#include <stdio.h>

#include <libpq-fe.h>

#include "database.h"

// See https://www.postgresql.org/docs/current/libpq-connect.html
bool
reachard_db_connect(struct reachard_db *db, const char *connstring) {
    PGconn *conn = NULL;
    PGresult *res = NULL;

    conn = PQconnectdb(connstring);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "%s", PQerrorMessage(conn));
        goto failure;
    }

    res = PQexec(
        conn, "SELECT pg_catalog.set_config('search_path', 'v0', false)"
    );
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "%s", PQresultErrorMessage(res));
        goto failure;
    }
    PQclear(res);

    db->conn = conn;

    return true;

failure:
    PQclear(res);
    PQfinish(conn);
    return false;
}

void
reachard_db_disconnect(struct reachard_db *db) {
    PQfinish(db->conn);
}
