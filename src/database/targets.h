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

#include <database/database.h>
#include <database/macros.h>

#include <stddef.h>

#define FOR_FIELDS(DO)                     \
    DO(0, id, ID, int, number)             \
    DO(1, name, NAME, char *, string)      \
    DO(2, url, URL, char *, string)        \
    DO(3, interval, INTERVAL, int, number) \
    DO(4, up, UP, bool, boolean)

struct reachard_db_target {
    FOR_FIELDS(DEFINE_STRUCT_FIELD)
};

// IWYU pragma: no_forward_declare reachard_db_target_k
enum reachard_db_target_k {
    FOR_FIELDS(DEFINE_ENUM_FIELD)
};

struct reachard_db_target_kv {
    enum reachard_db_target_k key;
    union reachard_db_value value;
};

void
reachard_db_target_init(struct reachard_db_target *target);

void
reachard_db_target_free(struct reachard_db_target *target);

void
reachard_db_targets_free(struct reachard_db_target *targets, size_t count);

int
reachard_db_targets_add(
    struct reachard_db *db,
    struct reachard_db_target target
);

int
reachard_db_targets_delete(struct reachard_db *db, const int id);

int
reachard_db_targets_get(
    struct reachard_db *db,
    struct reachard_db_target *target,
    int id
);

int
reachard_db_targets_get_all(
    struct reachard_db *db,
    struct reachard_db_target **targets,
    size_t *count
);

int
reachard_db_targets_update(
    struct reachard_db *db,
    int id,
    struct reachard_db_target_kv kvs[],
    size_t count
);
