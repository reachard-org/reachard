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

#include "targets.h"

#include <database/targets.h>
#include <server/handle.h>
#include <server/request.h>

#include <string.h>

#include <cjson/cJSON.h>
#include <microhttpd.h>

static enum MHD_Result
reachard_handle_targets_delete(struct reachard_request *request) {
    struct reachard_db *db = request->cls;
    struct reachard_connection_info *conn_info = *request->req_cls;

    if (reachard_db_targets_delete(db, conn_info->id)) {
        return reachard_request_respond_plain(
            request,
            "failed to delete the target",
            MHD_HTTP_INTERNAL_SERVER_ERROR
        );
    };

    return reachard_request_respond_plain(request, "", MHD_HTTP_OK);
}

static enum MHD_Result
reachard_handle_targets_get(struct reachard_request *request) {
    struct reachard_db *db = request->cls;
    struct reachard_connection_info *conn_info = *request->req_cls;

    struct reachard_db_target target;
    if (reachard_db_targets_get(db, &target, conn_info->id)) {
        return reachard_request_respond_plain(
            request,
            "failed to get the target",
            MHD_HTTP_INTERNAL_SERVER_ERROR
        );
    };

    cJSON *object = cJSON_CreateObject();
    cJSON_AddNumberToObject(object, "id", target.id);
    cJSON_AddStringToObject(object, "name", target.name);
    cJSON_AddStringToObject(object, "url", target.url);
    cJSON_AddNumberToObject(object, "interval", target.interval);

    char *body = cJSON_PrintUnformatted(object);

    cJSON_Delete(object);
    reachard_db_target_free(&target);
    return reachard_request_respond_json(request, body, MHD_HTTP_OK);
}

static enum MHD_Result
reachard_handle_targets_get_all(struct reachard_request *request) {
    struct reachard_db *db = request->cls;

    size_t count;
    struct reachard_db_target *targets;
    if (reachard_db_targets_get_all(db, &targets, &count)) {
        return reachard_request_respond_plain(
            request,
            "failed to get targets",
            MHD_HTTP_INTERNAL_SERVER_ERROR
        );
    };

    cJSON *array = cJSON_CreateArray();
    for (size_t i = 0; i < count; i++) {
        cJSON *object = cJSON_CreateObject();
        cJSON_AddNumberToObject(object, "id", targets[i].id);
        cJSON_AddStringToObject(object, "name", targets[i].name);
        cJSON_AddStringToObject(object, "url", targets[i].url);
        cJSON_AddNumberToObject(object, "interval", targets[i].interval);
        cJSON_AddItemToArray(array, object);
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

    struct reachard_db_target target;
    reachard_db_target_init(&target);

    cJSON *object;
    cJSON *item;

    object = cJSON_ParseWithOpts(conn_info->upload_data, 0, true);
    if (!object) {
        return reachard_request_respond_plain(
            request, "failed to parse as JSON", MHD_HTTP_BAD_REQUEST
        );
    }

    item = cJSON_GetObjectItemCaseSensitive(object, "name");
    if (!cJSON_IsString(item)) {
        cJSON_Delete(object);
        return reachard_request_respond_plain(
            request, "failed to parse target name", MHD_HTTP_BAD_REQUEST
        );
    }
    target.name = item->valuestring;

    item = cJSON_GetObjectItemCaseSensitive(object, "url");
    if (!cJSON_IsString(item)) {
        cJSON_Delete(object);
        return reachard_request_respond_plain(
            request, "failed to parse target url", MHD_HTTP_BAD_REQUEST
        );
    }
    target.url = item->valuestring;

    item = cJSON_GetObjectItemCaseSensitive(object, "interval");
    if (item) {
        if (!cJSON_IsNumber(item)) {
            cJSON_Delete(object);
            return reachard_request_respond_plain(
                request,
                "failed to parse target interval", MHD_HTTP_BAD_REQUEST
            );
        }
        target.interval = item->valueint;
    }

    const int id = reachard_db_targets_add(db, target);
    cJSON_Delete(object);

    object = cJSON_CreateObject();
    cJSON_AddNumberToObject(object, "id", id);
    char *body = cJSON_PrintUnformatted(object);
    cJSON_Delete(object);

    return reachard_request_respond_json(request, body, MHD_HTTP_OK);
}

enum MHD_Result
reachard_handle_targets_first_call(struct reachard_request *request) {
    struct reachard_connection_info *conn_info = *request->req_cls;

    if (strcmp(request->method, "DELETE") == 0 && conn_info->id) {
        conn_info->handle = &reachard_handle_targets_delete;
        return MHD_YES;
    }
    if (strcmp(request->method, "GET") == 0) {
        if (conn_info->id) {
            conn_info->handle = &reachard_handle_targets_get;
        } else {
            conn_info->handle = &reachard_handle_targets_get_all;
        }
        return MHD_YES;
    }
    if (strcmp(request->method, "POST") == 0) {
        conn_info->handle = &reachard_handle_targets_post;
        return MHD_YES;
    }

    return reachard_request_respond_plain(
        request, "method not allowed", MHD_HTTP_BAD_REQUEST
    );
}
