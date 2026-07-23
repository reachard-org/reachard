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

#include <utarray.h>
#include <utstring.h>

struct reachard_db_update {
    struct reachard_db *db;
    int id;
    UT_string *query;
    UT_array *params;
};

void
reachard_db_update_init(
    struct reachard_db_update *update,
    struct reachard_db *db,
    char *table,
    int id
);

void
reachard_db_update_string(
    struct reachard_db_update *update,
    char *name,
    char *string
);

void
reachard_db_update_number(
    struct reachard_db_update *update,
    char *name,
    int number
);

void
reachard_db_update_boolean(
    struct reachard_db_update *update,
    char *name,
    bool boolean
);

int
reachard_db_update_commit(struct reachard_db_update *update);

void
reachard_db_update_deinit(struct reachard_db_update *update);
