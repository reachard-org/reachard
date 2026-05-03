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

#include "client.h"

#include <stdio.h>
#include <threads.h>

#include <uv.h>

#include <client/state.h>
#include <client/targets.h>

int
reachard_client_init(struct reachard_client *client, struct reachard_db *db) {
    if (uv_loop_init(&client->loop)) {
        fprintf(stderr, "failed to initialize the client loop\n");
        return 1;
    }

    if (reachard_client_state_init(&client->state, db, &client->loop)) {
        fprintf(stderr, "failed to initialize curl\n");
        return 1;
    }

    return 0;
}

// Runs in the client's thread
static int
start(void *arg) {
    struct reachard_client *client = arg;

    uv_thread_setname("client");

    reachard_client_state_prepare(&client->state);

    reachard_client_target_init(&client->state, &client->target, 1);

    uv_run(&client->loop, UV_RUN_DEFAULT);

    fprintf(stderr, "client finished!\n");

    return 0;
}

// Runs in the main thread
int
reachard_client_start(struct reachard_client *client) {
    if (thrd_create(&client->thrd, start, client)) {
        fprintf(stderr, "failed to create the client thread\n");
        return 1;
    };

    return 0;
}

// Runs in the client's thread
static void
stop(uv_async_t *async) {
    struct reachard_client *client = async->data;

    reachard_client_target_deinit(&client->target);

    reachard_client_state_clear(&client->state);

    uv_close((uv_handle_t *)async, 0);
}

// Runs in the main thread
void
reachard_client_stop(struct reachard_client *client) {
    int loop_alive = uv_loop_alive(&client->loop);

    uv_async_t async;
    uv_async_init(&client->loop, &async, stop);
    async.data = client;
    uv_async_send(&async);

    if (!loop_alive) {
        uv_run(&client->loop, UV_RUN_DEFAULT);
    }

    thrd_join(client->thrd, 0);
}

void
reachard_client_deinit(struct reachard_client *client) {
    reachard_client_state_deinit(&client->state);
    uv_loop_close(&client->loop);
}
