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

#pragma once

#include <server/request.h>

#include <microhttpd.h>

struct reachard_response {
    struct MHD_Response *resp;
    unsigned int status_code;
};

void
reachard_response_prepare_plain(
    struct reachard_response *response,
    char *body,
    unsigned int status_code
);

void
reachard_response_prepare_json(
    struct reachard_response *response,
    char *body,
    unsigned int status_code
);

void
reachard_response_add_header(
    struct reachard_response *response,
    char *header,
    char *content
);

enum MHD_Result
reachard_respond(
    struct reachard_request *request,
    struct reachard_response *response
);

enum MHD_Result
reachard_respond_plain(
    struct reachard_request *request,
    char *body,
    unsigned int status_code
);

enum MHD_Result
reachard_respond_json(
    struct reachard_request *request,
    char *body,
    unsigned int status_code
);
