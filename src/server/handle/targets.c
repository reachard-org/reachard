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

#include <cjson/cJSON.h>
#include <microhttpd.h>

#include "../handle.h"
#include "../state.h"

#include "targets.h"

static enum MHD_Result
reachard_handle_targets_get(struct reachard_request *request) {
    struct reachard_targets_list *targets_list = request->cls;

    cJSON *targets = cJSON_CreateArray();

    struct reachard_targets_list_item *current;
    for (current = targets_list->head; current; current = current->next) {
        cJSON *target = cJSON_CreateObject();
        cJSON_AddNumberToObject(target, "id", current->id);
        cJSON_AddItemToArray(targets, target);
    }

    char *body = cJSON_PrintUnformatted(targets);
    cJSON_Delete(targets);

    return reachard_request_respond_with_free(request, body, MHD_HTTP_OK);
}

static enum MHD_Result
reachard_handle_processing(struct reachard_request *request) {
    struct reachard_connection_info *conn_info = *request->req_cls;

    const enum MHD_Result result = MHD_post_process(
        conn_info->postprocessor,
        request->upload_data,
        *request->upload_data_size
    );
    if (result != MHD_YES) {
        return MHD_NO;
    }

    *request->upload_data_size = 0;

    return MHD_YES;
}

static enum MHD_Result
reachard_handle_targets_delete(struct reachard_request *request) {
    if (*request->upload_data_size > 0) {
        return reachard_handle_processing(request);
    }

    return reachard_request_respond(request, "hello from targets DELETE!", MHD_HTTP_OK);
}

enum MHD_Result
reachard_handle_targets_delete_data_iterator(
    void *cls,
    enum MHD_ValueKind kind, const char *key,
    const char *filename, const char *content_type, const char *transfer_encoding,
    const char *data, uint64_t off, size_t size
) {
    struct reachard_targets_list *targets_list = cls;

    if (strcmp(key, "id") == 0) {
        const int id = atoi(data);
        reachard_targets_list_delete(targets_list, id);
    }

    return MHD_YES;
}

static enum MHD_Result
reachard_handle_targets_post(struct reachard_request *request) {
    if (*request->upload_data_size > 0) {
        return reachard_handle_processing(request);
    }

    return reachard_request_respond(request, "hello from targets POST!", MHD_HTTP_OK);
}

enum MHD_Result
reachard_handle_targets_post_data_iterator(
    void *cls,
    enum MHD_ValueKind kind, const char *key,
    const char *filename, const char *content_type, const char *transfer_encoding,
    const char *data, uint64_t off, size_t size
) {
    struct reachard_targets_list *targets_list = cls;

    if (strcmp(key, "id") == 0) {
        const int id = atoi(data);
        reachard_targets_list_add(targets_list, id);
    }

    return MHD_YES;
}

enum MHD_Result
reachard_handle_targets_first_call(struct reachard_request *request) {
    struct reachard_connection_info *conn_info =
        calloc(1, sizeof(struct reachard_connection_info));
    if (!conn_info) {
        return MHD_NO;
    }

    if (strcmp(request->method, "GET") == 0) {
        conn_info->handle = &reachard_handle_targets_get;
    } else if (strcmp(request->method, "POST") == 0) {
        conn_info->handle = &reachard_handle_targets_post;

        conn_info->postprocessor = MHD_create_post_processor(
            request->conn,
            512,
            &reachard_handle_targets_post_data_iterator,
            request->cls
        );

        if (!conn_info->postprocessor) {
            free(conn_info);
            return MHD_NO;
        }
    } else if (strcmp(request->method, "DELETE") == 0) {
        conn_info->handle = &reachard_handle_targets_delete;

        conn_info->postprocessor = MHD_create_post_processor(
            request->conn,
            512,
            &reachard_handle_targets_delete_data_iterator,
            request->cls
        );

        if (!conn_info->postprocessor) {
            free(conn_info);
            return MHD_NO;
        }
    }

    if (!conn_info->handle) {
        free(conn_info);
        return reachard_request_respond(
            request, "method not allowed", MHD_HTTP_BAD_REQUEST
        );
    }

    *request->req_cls = conn_info;

    return MHD_YES;
}
