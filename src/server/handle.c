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

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <microhttpd.h>

#include "handle/targets.h"

#include "handle.h"

static void
reachard_connection_info_destroy(struct reachard_connection_info *conn_info) {
    if (!conn_info) {
        return;
    }

    free(conn_info->upload_data);
    free(conn_info);
    conn_info = NULL;
}

enum MHD_Result
reachard_handle_upload_data(struct reachard_request *request) {
    struct reachard_connection_info *conn_info = *request->req_cls;

    conn_info->upload_data = realloc(conn_info->upload_data, conn_info->upload_data_size + *request->upload_data_size + 1);
    memcpy(conn_info->upload_data, request->upload_data, *request->upload_data_size);
    conn_info->upload_data_size += *request->upload_data_size;
    conn_info->upload_data[conn_info->upload_data_size] = '\0';
    *request->upload_data_size = 0;

    return MHD_YES;
}

static enum MHD_Result
reachard_handle_first_call(struct reachard_request *request) {
    struct reachard_connection_info *conn_info =
        calloc(1, sizeof(struct reachard_connection_info));
    if (!conn_info) {
        return MHD_NO;
    }
    *request->req_cls = conn_info;

    if (strcmp(request->url, "/targets/") == 0) {
        return reachard_handle_targets_first_call(request);
    }

    return reachard_request_respond_plain(request, "url not allowed", MHD_HTTP_BAD_REQUEST);
}

enum MHD_Result
reachard_handle(
    void *cls, struct MHD_Connection *conn,
    const char *url, const char *method, const char *version,
    const char *upload_data, size_t *upload_data_size,
    void **req_cls
) {
    /* This function is called several times over the lifetime of a request */
    struct reachard_request request = {
        cls, conn,
        url, method, version,
        upload_data, upload_data_size,
        req_cls
    };

    struct reachard_connection_info *conn_info = *req_cls;

    /* First call has headers available only */
    if (!conn_info) {
        return reachard_handle_first_call(&request);
    }

    /* The following calls make body available as well */
    return conn_info->handle(&request);
}

void
reachard_handle_complete(
    void *cls, struct MHD_Connection *conn,
    void **req_cls,
    enum MHD_RequestTerminationCode toe
) {
    reachard_connection_info_destroy(*req_cls);
}
