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

#include <microhttpd.h>

#include "request.h"

typedef enum MHD_Result (*reachard_handler)(struct reachard_request *request);

enum MHD_Result
reachard_handle(
    void *cls, struct MHD_Connection *conn,
    const char *url, const char *method, const char *version,
    const char *upload_data, size_t *upload_data_size,
    void **req_cls
);

void
reachard_handle_complete(
    void *cls, struct MHD_Connection *conn,
    void **req_cls,
    enum MHD_RequestTerminationCode toe
);

struct reachard_targets_list_item {
    struct reachard_targets_list_item *next;
    int id;
};

struct reachard_targets_list {
    struct reachard_targets_list_item *head, *tail;
};

void
reachard_targets_list_destroy(struct reachard_targets_list list);
