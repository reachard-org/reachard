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

#include "targets.h"

#include <client/state.h>
#include <database/targets.h>

#include <stdio.h>

#include <curl/curl.h>
#include <uv.h>

static void
target_timer(uv_timer_t *timer) {
    struct reachard_client_state *state = timer->data;

    struct reachard_client_target *client_target =
        (struct reachard_client_target *)timer;

    struct reachard_db_target db_target;
    reachard_db_targets_get(state->db, &db_target, client_target->id);

    CURL *easy = curl_easy_init();
    curl_easy_setopt(easy, CURLOPT_URL, db_target.url);
    curl_easy_setopt(easy, CURLOPT_NOBODY, 1);
    curl_multi_add_handle(state->multi, easy);
}

int
reachard_client_target_init(
    struct reachard_client_state *state,
    struct reachard_client_target *target,
    int id
) {
    target->timer.data = state;
    target->id = id;

    struct reachard_db_target db_target;
    if (reachard_db_targets_get(state->db, &db_target, id)) {
        fprintf(stderr, "failed to get the target\n");
        return 1;
    };

    uv_timer_init(state->loop, &target->timer);
    uv_timer_start(&target->timer, target_timer, 0, db_target.interval * 1000);

    return 0;
}

void
reachard_client_target_deinit(struct reachard_client_target *target) {
    uv_timer_stop(&target->timer);
    uv_close((uv_handle_t *)&target->timer, 0);
}
