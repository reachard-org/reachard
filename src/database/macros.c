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

#include "macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libpq-fe.h>

inline char *
parse_string(PGresult *res, int col, int row) {
    char *value = PQgetvalue(res, col, row);
    return strdup(value);
}

inline int
parse_number(PGresult *res, int col, int row) {
    char *value = PQgetvalue(res, col, row);
    return atoi(value);
}

inline bool
parse_boolean(PGresult *res, int col, int row) {
    char *value = PQgetvalue(res, col, row);
    return value[0] == 't';
}

inline char *
encode_string(union reachard_db_value value, char buffer[]) {
    return value.string;
}

char *
encode_number(union reachard_db_value value, char buffer[]) {
    snprintf(buffer, sizeof(*buffer), "%d", value.number);
    return buffer;
}

inline char *
encode_boolean(union reachard_db_value value, char buffer[]) {
    return value.boolean ? "t" : "f";
}
