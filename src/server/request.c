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

enum MHD_Result
reachard_request_respond(
    struct reachard_request *request,
    const char *content,
    const unsigned int status_code
) {
    struct MHD_Response *response =
        MHD_create_response_from_buffer_static(strlen(content), content);
    const enum MHD_Result result =
        MHD_queue_response(request->conn, status_code, response);

    MHD_destroy_response(response);

    return result;
}

enum MHD_Result
reachard_request_respond_with_free(
    struct reachard_request *request,
    char *content,
    const unsigned int status_code
) {
    struct MHD_Response *response =
        MHD_create_response_from_buffer_with_free_callback_cls(
            strlen(content), content, &free, content
        );
    const enum MHD_Result result =
        MHD_queue_response(request->conn, status_code, response);

    MHD_destroy_response(response);

    return result;
}
