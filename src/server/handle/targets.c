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
#include <server/request.h>
#include <server/response.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <cjson/cJSON.h>
#include <microhttpd.h>

static void
add_location_header(
    struct reachard_request *request,
    struct reachard_response *response,
    int id
) {
    const char *scheme =
        reachard_request_get_header(request, "X-Forwarded-Proto");
    if (!scheme) {
        scheme = "http";
    }

    const char *host =
        reachard_request_get_header(request, "X-Forwarded-Host");
    if (!host) {
        host = reachard_request_get_header(request, MHD_HTTP_HEADER_HOST);
    }

    if (host) {
        char location[256];
        snprintf(
            location, sizeof(location),
            "%s://%s/targets/%d/", scheme, host, id
        );
        reachard_response_add_header(
            response, MHD_HTTP_HEADER_LOCATION, location
        );
    }
}

static enum MHD_Result
reachard_handle_targets_delete(struct reachard_request *request) {
    struct reachard_db *db = request->cls;
    struct reachard_request_state *request_state = *request->state;

    auto result = reachard_db_targets_delete(db, request_state->id);
    switch (result) {
    case REACHARD_DELETE_DELETED:
        return reachard_respond_plain(request, "", MHD_HTTP_NO_CONTENT);
    case REACHARD_DELETE_NOT_FOUND:
        return reachard_respond_plain(request, "", MHD_HTTP_NOT_FOUND);
    case REACHARD_DELETE_ERROR:
        return reachard_respond_plain(
            request,
            "failed to delete the target",
            MHD_HTTP_INTERNAL_SERVER_ERROR
        );
    };

    unreachable();
}

static enum MHD_Result
reachard_handle_targets_get(struct reachard_request *request) {
    struct reachard_db *db = request->cls;
    struct reachard_request_state *request_state = *request->state;

    struct reachard_db_target target;
    auto result = reachard_db_targets_get(db, &target, request_state->id);
    switch (result) {
    case REACHARD_GET_NOT_FOUND:
        return reachard_respond_plain(request, "", MHD_HTTP_NOT_FOUND);
    case REACHARD_GET_ERROR:
        return reachard_respond_plain(
            request,
            "failed to get the target",
            MHD_HTTP_INTERNAL_SERVER_ERROR
        );
    case REACHARD_GET_FOUND:
    };

    cJSON *object = cJSON_CreateObject();
    cJSON_AddNumberToObject(object, "id", target.id);
    cJSON_AddStringToObject(object, "name", target.name);
    cJSON_AddStringToObject(object, "url", target.url);
    cJSON_AddNumberToObject(object, "interval", target.interval);

    char *body = cJSON_PrintUnformatted(object);

    cJSON_Delete(object);
    reachard_db_target_free(&target);
    return reachard_respond_json(request, body, MHD_HTTP_OK);
}

static enum MHD_Result
reachard_handle_targets_get_all(struct reachard_request *request) {
    struct reachard_db *db = request->cls;

    size_t count;
    struct reachard_db_target *targets;
    if (reachard_db_targets_get_all(db, &targets, &count)) {
        return reachard_respond_plain(
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
    return reachard_respond_json(request, body, MHD_HTTP_OK);
}

static enum MHD_Result
reachard_handle_targets_patch(struct reachard_request *request) {
    struct reachard_db *db = request->cls;
    struct reachard_request_state *request_state = *request->state;

    if (*request->body_size > 0) {
        return reachard_request_read_body(request);
    }

    struct reachard_db_target target = {0};
    struct reachard_db_target_mask target_mask = {0};

    cJSON *object;
    cJSON *item;

    object = cJSON_ParseWithOpts(request_state->body, 0, true);
    if (!object) {
        return reachard_respond_plain(
            request, "failed to parse as JSON", MHD_HTTP_BAD_REQUEST
        );
    }

    item = cJSON_GetObjectItemCaseSensitive(object, "name");
    if (item) {
        if (cJSON_IsString(item)) {
            target.name = item->valuestring;
            target_mask.name = 1;
        } else {
            cJSON_Delete(object);
            return reachard_respond_plain(
                request,
                "expected a string for the name", MHD_HTTP_BAD_REQUEST
            );
        }
    }

    item = cJSON_GetObjectItemCaseSensitive(object, "url");
    if (item) {
        if (cJSON_IsString(item)) {
            target.url = item->valuestring;
            target_mask.url = 1;
        } else {
            cJSON_Delete(object);
            return reachard_respond_plain(
                request,
                "expected a string for the url", MHD_HTTP_BAD_REQUEST
            );
        }
    }

    item = cJSON_GetObjectItemCaseSensitive(object, "interval");
    if (item) {
        if (cJSON_IsNumber(item)) {
            target.interval = item->valueint;
            target_mask.interval = 1;
        } else {
            cJSON_Delete(object);
            return reachard_respond_plain(
                request,
                "expected a number for the interval", MHD_HTTP_BAD_REQUEST
            );
        }
    }

    if (reachard_db_targets_update(db, target, target_mask, request_state->id)) {
        cJSON_Delete(object);
        return reachard_respond_plain(
            request,
            "failed to update the target", MHD_HTTP_INTERNAL_SERVER_ERROR
        );
    };

    cJSON_Delete(object);
    return reachard_respond_plain(request, "", MHD_HTTP_OK);
}

static enum MHD_Result
reachard_handle_targets_post(struct reachard_request *request) {
    struct reachard_db *db = request->cls;
    struct reachard_request_state *request_state = *request->state;

    if (*request->body_size > 0) {
        return reachard_request_read_body(request);
    }

    struct reachard_db_target target;
    reachard_db_target_init(&target);

    cJSON *object;
    cJSON *item;

    object = cJSON_ParseWithOpts(request_state->body, 0, true);
    if (!object) {
        return reachard_respond_plain(
            request, "failed to parse as JSON", MHD_HTTP_BAD_REQUEST
        );
    }

    item = cJSON_GetObjectItemCaseSensitive(object, "name");
    if (!cJSON_IsString(item)) {
        cJSON_Delete(object);
        return reachard_respond_plain(
            request, "failed to parse target name", MHD_HTTP_BAD_REQUEST
        );
    }
    target.name = item->valuestring;

    item = cJSON_GetObjectItemCaseSensitive(object, "url");
    if (!cJSON_IsString(item)) {
        cJSON_Delete(object);
        return reachard_respond_plain(
            request, "failed to parse target url", MHD_HTTP_BAD_REQUEST
        );
    }
    target.url = item->valuestring;

    item = cJSON_GetObjectItemCaseSensitive(object, "interval");
    if (item) {
        if (!cJSON_IsNumber(item)) {
            cJSON_Delete(object);
            return reachard_respond_plain(
                request,
                "failed to parse target interval", MHD_HTTP_BAD_REQUEST
            );
        }
        target.interval = item->valueint;
    }

    int id;
    if (reachard_db_targets_add(db, target, &id)) {
        cJSON_Delete(object);
        return reachard_respond_plain(
            request,
            "failed to add the target",
            MHD_HTTP_INTERNAL_SERVER_ERROR
        );
    }
    cJSON_Delete(object);

    object = cJSON_CreateObject();
    cJSON_AddNumberToObject(object, "id", id);
    char *body = cJSON_PrintUnformatted(object);
    cJSON_Delete(object);

    struct reachard_response response;
    reachard_response_prepare_json(&response, body, MHD_HTTP_CREATED);
    add_location_header(request, &response, id);
    return reachard_respond(request, &response);
}

static enum MHD_Result
reachard_handle_targets_put(struct reachard_request *request) {
    struct reachard_db *db = request->cls;
    struct reachard_request_state *request_state = *request->state;

    if (*request->body_size > 0) {
        return reachard_request_read_body(request);
    }

    struct reachard_db_target target;
    reachard_db_target_init(&target);

    cJSON *object;
    cJSON *item;

    object = cJSON_ParseWithOpts(request_state->body, 0, true);
    if (!object) {
        return reachard_respond_plain(
            request, "failed to parse as JSON", MHD_HTTP_BAD_REQUEST
        );
    }

    item = cJSON_GetObjectItemCaseSensitive(object, "name");
    if (!cJSON_IsString(item)) {
        cJSON_Delete(object);
        return reachard_respond_plain(
            request, "failed to parse target name", MHD_HTTP_BAD_REQUEST
        );
    }
    target.name = item->valuestring;

    item = cJSON_GetObjectItemCaseSensitive(object, "url");
    if (!cJSON_IsString(item)) {
        cJSON_Delete(object);
        return reachard_respond_plain(
            request, "failed to parse target url", MHD_HTTP_BAD_REQUEST
        );
    }
    target.url = item->valuestring;

    item = cJSON_GetObjectItemCaseSensitive(object, "interval");
    if (!cJSON_IsNumber(item)) {
        cJSON_Delete(object);
        return reachard_respond_plain(
            request,
            "failed to parse target interval", MHD_HTTP_BAD_REQUEST
        );
    }
    target.interval = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(object, "up");
    if (!cJSON_IsBool(item)) {
        cJSON_Delete(object);
        return reachard_respond_plain(
            request,
            "failed to parse target up status", MHD_HTTP_BAD_REQUEST
        );
    }
    target.up = cJSON_IsTrue(item);

    auto result = reachard_db_targets_replace(db, target, request_state->id);
    cJSON_Delete(object);

    switch (result) {
    case REACHARD_REPLACE_INSERTED:
        struct reachard_response response;
        reachard_response_prepare_plain(&response, "", MHD_HTTP_CREATED);
        add_location_header(request, &response, request_state->id);
        return reachard_respond(request, &response);
    case REACHARD_REPLACE_REPLACED:
        return reachard_respond_plain(request, "", MHD_HTTP_OK);
    case REACHARD_REPLACE_ERROR:
        return reachard_respond_plain(
            request,
            "failed to replace the target",
            MHD_HTTP_INTERNAL_SERVER_ERROR
        );
    }

    unreachable();
}

enum MHD_Result
reachard_handle_targets_first_call(struct reachard_request *request) {
    struct reachard_request_state *request_state = *request->state;

    switch (request_state->type) {
    case REACHARD_REQUEST_TYPE_COLLECTION:
        if (strcmp(request->method, "GET") == 0) {
            request_state->handle = &reachard_handle_targets_get_all;
            return MHD_YES;
        }
        if (strcmp(request->method, "POST") == 0) {
            request_state->handle = &reachard_handle_targets_post;
            return MHD_YES;
        }
        break;
    case REACHARD_REQUEST_TYPE_RESOURCE:
        if (strcmp(request->method, "DELETE") == 0) {
            request_state->handle = &reachard_handle_targets_delete;
            return MHD_YES;
        }
        if (strcmp(request->method, "GET") == 0) {
            request_state->handle = &reachard_handle_targets_get;
            return MHD_YES;
        }
        if (strcmp(request->method, "PATCH") == 0) {
            request_state->handle = &reachard_handle_targets_patch;
            return MHD_YES;
        }
        if (strcmp(request->method, "PUT") == 0) {
            request_state->handle = &reachard_handle_targets_put;
            return MHD_YES;
        }
        break;
    }

    return reachard_respond_plain(
        request, "", MHD_HTTP_METHOD_NOT_ALLOWED
    );
}
