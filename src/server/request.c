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

#include <stdlib.h>
#include <string.h>

#include <microhttpd.h>

#include "request.h"

static enum MHD_Result
reachard_request_expect_content_type(struct reachard_request *request, const char *content_type) {
    const char *actual_content_type = MHD_lookup_connection_value(
        request->conn, MHD_HEADER_KIND, MHD_HTTP_HEADER_CONTENT_TYPE
    );

    if (strcmp(actual_content_type, content_type)) {
        return reachard_request_respond_plain(
            request, "unsupported media type", MHD_HTTP_UNSUPPORTED_MEDIA_TYPE
        );
    }

    return MHD_YES;
}

enum MHD_Result
reachard_request_expect_json(struct reachard_request *request) {
    return reachard_request_expect_content_type(request, "application/json");
}

enum MHD_Result
reachard_request_respond_plain(
    struct reachard_request *request,
    const char *body,
    const unsigned int status_code
) {
    struct MHD_Response *response =
        MHD_create_response_from_buffer_static(strlen(body), body);
    MHD_add_response_header(
        response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/plain"
    );
    const enum MHD_Result result =
        MHD_queue_response(request->conn, status_code, response);

    MHD_destroy_response(response);

    return result;
}

enum MHD_Result
reachard_request_respond_json(
    struct reachard_request *request,
    char *body,
    const unsigned int status_code
) {
    struct MHD_Response *response =
        MHD_create_response_from_buffer_with_free_callback_cls(
            strlen(body), body, &free, body
        );
    MHD_add_response_header(
        response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json"
    );

    const enum MHD_Result result =
        MHD_queue_response(request->conn, status_code, response);

    MHD_destroy_response(response);

    return result;
}
