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

#include "database.h"

struct reachard_db_target {
    int id;
    char *name;
};

void
reachard_db_targets_free(struct reachard_db_target *targets, size_t count);

int
reachard_db_targets_add(struct reachard_db *db, const char *name);

bool
reachard_db_targets_delete(struct reachard_db *db, const int id);

bool
reachard_db_targets_get(
    struct reachard_db *db,
    struct reachard_db_target **targets,
    size_t *count
);
