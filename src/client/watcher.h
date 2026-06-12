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

#include <client/state.h>
#include <database/database.h>

#include <uv.h>

struct reachard_client_watcher {
    struct reachard_client_state state;
    // A hash table where `id` maps to `target`
    struct reachard_client_target *targets;
};

int
reachard_client_watcher_init(
    struct reachard_client_watcher *watcher,
    struct reachard_db db,
    uv_loop_t *loop
);

int
reachard_client_watcher_start(struct reachard_client_watcher *watcher);

int
reachard_client_watcher_add(
    struct reachard_client_watcher *watcher, int id
);

void
reachard_client_watcher_delete(
    struct reachard_client_watcher *watcher, int id
);

void
reachard_client_watcher_stop(struct reachard_client_watcher *watcher);

void
reachard_client_watcher_deinit(struct reachard_client_watcher *watcher);
