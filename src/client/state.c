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

#include "state.h"

#include <database/database.h>

#include <stdio.h>
#include <stdlib.h>

// See the following documentation:
// - https://curl.se/libcurl/c/libcurl-multi.html
// - https://curl.se/libcurl/c/curl_multi_socket_action.html
// - https://curl.se/libcurl/c/CURLMOPT_SOCKETFUNCTION.html
// - https://curl.se/libcurl/c/CURLMOPT_TIMERFUNCTION.html
// - https://everything.curl.dev/transfers/drive/multi-socket.html

// Runs in the main thread
int
reachard_client_state_init(
    struct reachard_client_state *state,
    struct reachard_db *db,
    uv_loop_t *loop
) {
    if (curl_global_init(CURL_GLOBAL_ALL)) {
        fprintf(stderr, "failed to initialize curl\n");
        return 1;
    }

    state->loop = loop;
    state->db = db;

    return 0;
}

static void
check_completed(CURLM *multi) {
    CURLMsg *message;
    int pending;
    while ((message = curl_multi_info_read(multi, &pending))) {
        if (message->msg != CURLMSG_DONE) {
            continue;
        }

        CURL *easy = message->easy_handle;

        CURLcode err = message->data.result;
        if (err) {
            fprintf(stderr, "%s\n", curl_easy_strerror(err));
            fprintf(stderr, "transfer failed with error code %d\n", err);
        } else {
            int status;
            curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &status);
            fprintf(
                stderr, "finished the transfer with status code %d\n", status
            );
        }

        curl_multi_remove_handle(multi, easy);
        curl_easy_cleanup(easy);
    }
}

// Used as a baton
struct socket {
    uv_poll_t poll;
    curl_socket_t s;
};

static void
socket_deinit(uv_handle_t *handle) {
    struct socket *socket = (struct socket *)handle;

    free(socket);
}

static void
on_poll(uv_poll_t *poll, int status, int events) {
    struct reachard_client_state *state = poll->data;

    struct socket *socket = (struct socket *)poll;

    int ev_bitmask = 0;
    if (status) {
        ev_bitmask = CURL_CSELECT_ERR;
    } else {
        if (events & UV_READABLE) {
            ev_bitmask |= CURL_CSELECT_IN;
        }
        if (events & UV_WRITABLE) {
            ev_bitmask |= CURL_CSELECT_OUT;
        }
    }

    curl_multi_socket_action(state->multi, socket->s, ev_bitmask, 0);
    check_completed(state->multi);
}

// Curl will inform this application about sockets to monitor via this
// function. Curl expects this application to then notify about changes
// on such sockets via calls to `curl_multi_socket_action`.
static int
socket_function(
    CURL *easy, curl_socket_t s, int what, void *statep, void *socketp
) {
    struct reachard_client_state *state = statep;

    struct socket *socket;
    if (socketp) {
        socket = socketp;
    } else {
        socket = malloc(sizeof(struct socket));
        uv_poll_init_socket(state->loop, &socket->poll, s);
        socket->poll.data = state;
        socket->s = s;
        curl_multi_assign(state->multi, s, socket);
    }

    switch (what) {
    case CURL_POLL_IN:
        uv_poll_start(&socket->poll, UV_READABLE, on_poll);
        break;
    case CURL_POLL_OUT:
        uv_poll_start(&socket->poll, UV_WRITABLE, on_poll);
        break;
    case CURL_POLL_INOUT:
        uv_poll_start(&socket->poll, UV_READABLE | UV_WRITABLE, on_poll);
        break;
    case CURL_POLL_REMOVE:
        uv_poll_stop(&socket->poll);
        curl_multi_assign(state->multi, s, 0);

        // Note that this will run later, thus we free the socket later as well
        uv_close((uv_handle_t *)&socket->poll, socket_deinit);

        break;
    }

    return 0;
}

static void
on_timeout(uv_timer_t *timer) {
    struct reachard_client_state *state = timer->data;

    curl_multi_socket_action(state->multi, CURL_SOCKET_TIMEOUT, 0, 0);
    check_completed(state->multi);
}

// Curl will inform this application about timeouts on various activities
// via this function. These activities include socket timeouts, calling the
// progress callback, starting over a retry or failing a transfer that takes
// too long, etc. Note that there is a single timeout for one multi handle.
static int
timer_function(CURLM *multi, long timeout_ms, void *statep) {
    struct reachard_client_state *state = statep;

    if (timeout_ms >= 0) {
        uv_timer_start(&state->timer, on_timeout, timeout_ms, 0);
    } else {
        uv_timer_stop(&state->timer);
    }

    return 0;
}

// Runs in the client's thread
void
reachard_client_state_prepare(struct reachard_client_state *state) {
    uv_timer_init(state->loop, &state->timer);
    state->timer.data = state;

    state->multi = curl_multi_init();
    curl_multi_setopt(state->multi, CURLMOPT_SOCKETFUNCTION, socket_function);
    curl_multi_setopt(state->multi, CURLMOPT_SOCKETDATA, state);
    curl_multi_setopt(state->multi, CURLMOPT_TIMERFUNCTION, timer_function);
    curl_multi_setopt(state->multi, CURLMOPT_TIMERDATA, state);
}

// Runs in the client's thread
void
reachard_client_state_clear(struct reachard_client_state *state) {
    // Detach and free all easy handles, thus interrupting all ongoing transfers
    CURL **handles = curl_multi_get_handles(state->multi);
    if (handles) {
        for (int i = 0; handles[i]; i++) {
            curl_multi_remove_handle(state->multi, handles[i]);
            curl_easy_cleanup(handles[i]);
        }
        curl_free(handles);
    }

    // Note that the socket function is likely to be called during the cleanup
    // of the multi handle, allocating and immediately freeing up sockets,
    // adding work to the event loop
    curl_multi_cleanup(state->multi);

    uv_close((uv_handle_t *)&state->timer, 0);
}

// Runs in the main thread
void
reachard_client_state_deinit(struct reachard_client_state *state) {
    curl_global_cleanup();
}
