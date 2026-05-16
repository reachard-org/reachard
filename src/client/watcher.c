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

#include "watcher.h"

#include <client/state.h>
#include <client/targets.h>
#include <database/database.h>

#include <stdio.h>
#include <stdlib.h>

#include <uthash.h>
#include <uv.h>

// Runs in the main thread
int
reachard_client_watcher_init(
    struct reachard_client_watcher *watcher,
    struct reachard_db db,
    uv_loop_t *loop
) {
    watcher->targets = 0;

    if (reachard_client_state_init(&watcher->state, db, loop)) {
        fprintf(stderr, "failed to initialize the state\n");
        return 1;
    }

    return 0;
}

// Runs in the client's thread
void
reachard_client_watcher_start(struct reachard_client_watcher *watcher) {
    reachard_client_state_prepare(&watcher->state);
}

// Runs in the client's thread
int
reachard_client_watcher_add(
    struct reachard_client_watcher *watcher, int id
) {
    struct reachard_client_target *target =
        malloc(sizeof(struct reachard_client_target));

    if (reachard_client_target_init(&watcher->state, target, id)) {
        fprintf(stderr, "failed to add a target to the watcher\n");
        free(target);
        return 1;
    }

    HASH_ADD_INT(watcher->targets, id, target);

    return 0;
}

// Runs in the client's thread
void
reachard_client_watcher_delete(
    struct reachard_client_watcher *watcher, int id
) {
    struct reachard_client_target *target = 0;

    HASH_FIND_INT(watcher->targets, &id, target);
    if (target) {
        HASH_DEL(watcher->targets, target);
        reachard_client_target_deinit(target);
        free(target);
    }
}

// Runs in the client's thread
void
reachard_client_watcher_stop(struct reachard_client_watcher *watcher) {
    struct reachard_client_target *target, *tmp;

    HASH_ITER(hh, watcher->targets, target, tmp) {
        HASH_DEL(watcher->targets, target);
        reachard_client_target_deinit(target);
        free(target);
    }

    reachard_client_state_clear(&watcher->state);
}

// Runs in the main thread
void
reachard_client_watcher_deinit(struct reachard_client_watcher *watcher) {
    reachard_client_state_deinit(&watcher->state);
}
