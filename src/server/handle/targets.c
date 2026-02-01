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

#include "../../database/targets.h"
#include "../handle.h"

#include "targets.h"

static enum MHD_Result
reachard_handle_targets_delete(struct reachard_request *request) {
    struct reachard_db *db = request->cls;
    struct reachard_connection_info *conn_info = *request->req_cls;

    if (*request->upload_data_size > 0) {
        return reachard_handle_upload_data(request);
    }

    cJSON *object = cJSON_ParseWithOpts(conn_info->upload_data, NULL, true);
    if (!object) {
        return reachard_request_respond_plain(
            request, "failed to parse as JSON", MHD_HTTP_BAD_REQUEST
        );
    }

    const cJSON *id_item = cJSON_GetObjectItemCaseSensitive(object, "id");
    if (!cJSON_IsNumber(id_item)) {
        cJSON_Delete(object);
        return reachard_request_respond_plain(
            request, "failed to parse target ID", MHD_HTTP_BAD_REQUEST
        );
    }

    const int id = id_item->valueint;
    reachard_db_targets_delete(db, id);

    cJSON_Delete(object);
    return reachard_request_respond_plain(request, "", MHD_HTTP_OK);
}

static enum MHD_Result
reachard_handle_targets_get(struct reachard_request *request) {
    struct reachard_db *db = request->cls;

    size_t count;
    struct reachard_db_target *targets;
    reachard_db_targets_get(db, &targets, &count);

    cJSON *array = cJSON_CreateArray();
    for (size_t i = 0; i < count; i++) {
        cJSON *target = cJSON_CreateObject();
        cJSON_AddNumberToObject(target, "id", targets[i].id);
        cJSON_AddStringToObject(target, "name", targets[i].name);
        cJSON_AddItemToArray(array, target);
    }

    char *body = cJSON_PrintUnformatted(array);

    cJSON_Delete(array);
    reachard_db_targets_free(targets, count);
    return reachard_request_respond_json(request, body, MHD_HTTP_OK);
}

static enum MHD_Result
reachard_handle_targets_post(struct reachard_request *request) {
    struct reachard_db *db = request->cls;
    struct reachard_connection_info *conn_info = *request->req_cls;

    if (*request->upload_data_size > 0) {
        return reachard_handle_upload_data(request);
    }

    cJSON *object = cJSON_ParseWithOpts(conn_info->upload_data, NULL, true);
    if (!object) {
        return reachard_request_respond_plain(
            request, "failed to parse as JSON", MHD_HTTP_BAD_REQUEST
        );
    }

    const cJSON *name_item = cJSON_GetObjectItemCaseSensitive(object, "name");
    if (!cJSON_IsString(name_item)) {
        cJSON_Delete(object);
        return reachard_request_respond_plain(
            request, "failed to parse target name", MHD_HTTP_BAD_REQUEST
        );
    }

    const char *name = name_item->valuestring;
    reachard_db_targets_add(db, name);

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

    return reachard_request_respond_plain(
        request, "method not allowed", MHD_HTTP_BAD_REQUEST
    );
}
