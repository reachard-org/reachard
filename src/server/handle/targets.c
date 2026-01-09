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

#include <string.h>

#include <cjson/cJSON.h>
#include <microhttpd.h>

#include "../handle.h"
#include "../state.h"

#include "targets.h"

static enum MHD_Result
reachard_handle_targets_delete(struct reachard_request *request) {
    struct reachard_targets_list *targets_list = request->cls;
    struct reachard_connection_info *conn_info = *request->req_cls;

    if (*request->upload_data_size > 0) {
        return reachard_handle_upload_data(request);
    }

    cJSON *object = cJSON_ParseWithOpts(conn_info->upload_data, NULL, true);
    if (object == NULL) {
        return reachard_request_respond_plain(request, "failed to parse as JSON", MHD_HTTP_BAD_REQUEST);
    }

    const cJSON *id_item = cJSON_GetObjectItemCaseSensitive(object, "id");
    if (!cJSON_IsNumber(id_item)) {
        cJSON_Delete(object);
        return reachard_request_respond_plain(request, "failed to parse target ID", MHD_HTTP_BAD_REQUEST);
    }

    const int id = id_item->valueint;
    reachard_targets_list_delete(targets_list, id);

    cJSON_Delete(object);
    return reachard_request_respond_plain(request, "", MHD_HTTP_OK);
}

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

    return reachard_request_respond_json(request, body, MHD_HTTP_OK);
}

static enum MHD_Result
reachard_handle_targets_post(struct reachard_request *request) {
    struct reachard_targets_list *targets_list = request->cls;
    struct reachard_connection_info *conn_info = *request->req_cls;

    if (*request->upload_data_size > 0) {
        return reachard_handle_upload_data(request);
    }

    cJSON *object = cJSON_ParseWithOpts(conn_info->upload_data, NULL, true);
    if (object == NULL) {
        return reachard_request_respond_plain(request, "failed to parse as JSON", MHD_HTTP_BAD_REQUEST);
    }

    const cJSON *id_item = cJSON_GetObjectItemCaseSensitive(object, "id");
    if (!cJSON_IsNumber(id_item)) {
        cJSON_Delete(object);
        return reachard_request_respond_plain(request, "failed to parse target ID", MHD_HTTP_BAD_REQUEST);
    }

    const int id = id_item->valueint;
    reachard_targets_list_add(targets_list, id);

    cJSON_Delete(object);
    return reachard_request_respond_plain(request, "", MHD_HTTP_OK);
}

enum MHD_Result
reachard_handle_targets_first_call(struct reachard_request *request) {
    struct reachard_connection_info *conn_info = *request->req_cls;

    if (strcmp(request->method, "DELETE") == 0) {
        conn_info->handle = &reachard_handle_targets_delete;
        return reachard_request_expect_json(request);
    }
    if (strcmp(request->method, "GET") == 0) {
        conn_info->handle = &reachard_handle_targets_get;
        return MHD_YES;
    }
    if (strcmp(request->method, "POST") == 0) {
        conn_info->handle = &reachard_handle_targets_post;
        return reachard_request_expect_json(request);
    }

    return reachard_request_respond_plain(request, "method not allowed", MHD_HTTP_BAD_REQUEST);
}
