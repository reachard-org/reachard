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

#pragma once

#include <libpq-fe.h>

// See https://en.wikipedia.org/wiki/X_macro

union reachard_db_value {
    char *string;
    int number;
    bool boolean;
};

#define DEFINE_STRUCT_FIELD(idx, name, NAME, type, TYPE, ...) \
    type name;

#define DEFINE_ENUM_FIELD(idx, name, NAME, type, TYPE, ...) \
    NAME,

#define DEFINE_FIELD_NAME(idx, name, NAME, type, TYPE, ...) \
    #name,

char *
parse_string(PGresult *res, int col, int row);

int
parse_number(PGresult *res, int col, int row);

bool
parse_boolean(PGresult *res, int col, int row);

#define DEFINE_PARSE_CALL(idx, name, NAME, type, TYPE, ...) \
    s->name = parse_##TYPE(res, col, idx);

char *
encode_string(union reachard_db_value value, char buffer[]);

char *
encode_number(union reachard_db_value value, char buffer[]);

char *
encode_boolean(union reachard_db_value value, char buffer[]);

#define DEFINE_ENCODE_CALL(idx, name, NAME, type, TYPE, ...)  \
    case NAME:                                                \
        char buffer_##name[REACHARD_INT_STR_LEN];             \
        paramValues[i] = encode_##TYPE(value, buffer_##name); \
        break;
