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
#include <threads.h>

#include <uv.h>

#include "client.h"

int
reachard_client_init(struct reachard_client *client) {
    if (uv_loop_init(&client->loop)) {
        fprintf(stderr, "failed to initialize the client loop\n");
        return 1;
    }

    return 0;
}

static void
timer_print(uv_timer_t *timer) {
    fprintf(stderr, "hello from the timer!\n");
}

static int
reachard_client_run(void *arg) {
    struct reachard_client *client = arg;

    uv_thread_setname("client");

    uv_timer_t timer;
    uv_timer_init(&client->loop, &timer);
    uv_timer_start(&timer, timer_print, 0, 1000);

    uv_run(&client->loop, UV_RUN_DEFAULT);

    fprintf(stderr, "client finished!\n");

    return 0;
}

int
reachard_client_start(struct reachard_client *client) {
    if (thrd_create(&client->thrd, reachard_client_run, client)) {
        fprintf(stderr, "failed to create the client thread\n");
        return 1;
    };

    return 0;
}

static void
reachard_client_close_handle(uv_handle_t *handle, void *arg) {
    uv_close(handle, 0);
}

void
reachard_client_stop(struct reachard_client *client) {
    // Close every active handle
    uv_walk(&client->loop, &reachard_client_close_handle, 0);

    // Wake the event loop up immediately
    uv_async_t async;
    uv_async_init(&client->loop, &async, 0);
    uv_async_send(&async);
    uv_close((uv_handle_t *)&async, 0);

    // Wait for the event loop to finish
    thrd_join(client->thrd, 0);
}

void
reachard_client_cleanup(struct reachard_client *client) {
    uv_loop_close(&client->loop);
}
