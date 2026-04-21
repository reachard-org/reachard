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

thread_local int counter = 0;

void
wait_for_a_while(uv_idle_t *handle) {
    counter++;

    if (counter >= 10e6) {
        uv_idle_stop(handle);
        uv_close((uv_handle_t *)handle, 0);
    }
}

static int
reachard_client_run(void *arg) {
    struct reachard_client *client = arg;

    uv_idle_t *idle = &(uv_idle_t){};

    uv_idle_init(&client->loop, idle);
    uv_idle_start(idle, wait_for_a_while);

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

void
reachard_client_stop(struct reachard_client *client) {
    thrd_join(client->thrd, 0);
    uv_loop_close(&client->loop);
}
