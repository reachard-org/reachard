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

#include <stddef.h>

#include <microhttpd.h>

struct reachard_request {
    void *cls;
    struct MHD_Connection *conn;
    const char *url, *method, *version;
    const char *body;
    size_t *body_size;
    void **state;
};

const char *
reachard_request_get_header(struct reachard_request *request, char *header);

enum MHD_Result
reachard_request_read_body(struct reachard_request *request);

enum reachard_request_type {
    REACHARD_REQUEST_TYPE_COLLECTION,
    REACHARD_REQUEST_TYPE_RESOURCE
};

// This struct contains data that should survive multiple calls of
// `reachard_handle`
struct reachard_request_state {
    enum MHD_Result (*handle)(struct reachard_request *request);
    enum reachard_request_type type;
    int id;
    char *body;
    size_t body_size;
};

int
reachard_request_state_init(struct reachard_request_state **request_state);

void
reachard_request_state_deinit(struct reachard_request_state *request_state);
