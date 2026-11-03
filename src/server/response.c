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

#include "response.h"

#include <server/handle.h>

#include <stdlib.h>
#include <string.h>

#include <microhttpd.h>

void
reachard_response_prepare_plain(
    struct reachard_response *response,
    char *body,
    unsigned int status_code
) {
    struct MHD_Response *resp =
        MHD_create_response_from_buffer_static(strlen(body), body);
    MHD_add_response_header(
        resp, MHD_HTTP_HEADER_CONTENT_TYPE, "text/plain"
    );

    response->resp = resp;
    response->status_code = status_code;
}

void
reachard_response_prepare_json(
    struct reachard_response *response,
    char *body,
    unsigned int status_code
) {
    struct MHD_Response *resp =
        MHD_create_response_from_buffer_with_free_callback_cls(
            strlen(body), body, &free, body
        );
    MHD_add_response_header(
        resp, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json"
    );

    response->resp = resp;
    response->status_code = status_code;
}

enum MHD_Result
reachard_respond(
    struct reachard_request *request,
    struct reachard_response *response
) {
    enum MHD_Result result =
        MHD_queue_response(
            request->conn, response->status_code, response->resp
        );

    MHD_destroy_response(response->resp);

    return result;
}

enum MHD_Result
reachard_respond_plain(
    struct reachard_request *request,
    char *body,
    unsigned int status_code
) {
    struct reachard_response response;
    reachard_response_prepare_plain(&response, body, status_code);
    return reachard_respond(request, &response);
}

enum MHD_Result
reachard_respond_json(
    struct reachard_request *request,
    char *body,
    unsigned int status_code
) {
    struct reachard_response response;
    reachard_response_prepare_json(&response, body, status_code);
    return reachard_respond(request, &response);
}
