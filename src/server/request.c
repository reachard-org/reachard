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

#include "request.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <microhttpd.h>

const char *
reachard_request_get_header(struct reachard_request *request, char *header) {
    return MHD_lookup_connection_value(
        request->conn, MHD_HEADER_KIND, header
    );
}

enum MHD_Result
reachard_request_read_body(struct reachard_request *request) {
    struct reachard_request_state *request_state = *request->state;

    request_state->body = realloc(
        request_state->body,
        request_state->body_size + *request->body_size + 1
    );
    if (!request_state->body) {
        fprintf(stderr, "failed to reallocate memory for the request body\n");
        return MHD_NO;
    }

    memcpy(request_state->body, request->body, *request->body_size);
    request_state->body_size += *request->body_size;
    request_state->body[request_state->body_size] = '\0';
    *request->body_size = 0;

    return MHD_YES;
}

int
reachard_request_state_init(struct reachard_request_state **request_state) {
    *request_state = malloc(sizeof(struct reachard_request_state));
    if (!*request_state) {
        return 1;
    }

    (*request_state)->body = 0;
    (*request_state)->body_size = 0;

    return 0;
}

void
reachard_request_state_deinit(struct reachard_request_state *request_state) {
    if (!request_state) {
        return;
    }

    free(request_state->body);
    free(request_state);
    request_state = 0;
}
