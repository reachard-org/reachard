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
#include <database/targets.h>

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
int
reachard_client_watcher_start(struct reachard_client_watcher *watcher) {
    reachard_client_state_prepare(&watcher->state);

    struct reachard_db_target *db_targets;
    size_t count;
    if (reachard_db_targets_get_all(&watcher->state.db, &db_targets, &count)) {
        fprintf(stderr, "failed to get targets\n");
        return 1;
    }

    struct reachard_client_target *target;
    for (size_t i = 0; i < count; i++) {
        target = malloc(sizeof(struct reachard_client_target));
        if (!target) {
            fprintf(stderr, "failed to allocate memory for a target\n");
            reachard_db_targets_free(db_targets, count);
            return 1;
        }

        reachard_client_target_init(&watcher->state, target, &db_targets[i]);
        HASH_ADD_INT(watcher->targets, id, target);
    }

    reachard_db_targets_free(db_targets, count);
    return 0;
}

// Runs in the client's thread
int
reachard_client_watcher_add(
    struct reachard_client_watcher *watcher, int id
) {
    struct reachard_client_target *target =
        malloc(sizeof(struct reachard_client_target));
    if (!target) {
        fprintf(stderr, "failed to allocate memory for a target\n");
        return 1;
    }

    struct reachard_db_target db_target;
    if (reachard_db_targets_get(&watcher->state.db, &db_target, id)) {
        fprintf(stderr, "failed to get the target\n");
        free(target);
        return 1;
    };

    reachard_client_target_init(&watcher->state, target, &db_target);
    HASH_ADD_INT(watcher->targets, id, target);

    reachard_db_target_free(&db_target);
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
    }
}

// Runs in the client's thread
void
reachard_client_watcher_stop(struct reachard_client_watcher *watcher) {
    struct reachard_client_target *target, *tmp;

    HASH_ITER(hh, watcher->targets, target, tmp) {
        HASH_DEL(watcher->targets, target);
        reachard_client_target_deinit(target);
    }

    reachard_client_state_clear(&watcher->state);
}

// Runs in the main thread
void
reachard_client_watcher_deinit(struct reachard_client_watcher *watcher) {
    reachard_client_state_deinit(&watcher->state);
}
