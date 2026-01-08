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

#include "handle.h"

static char *
reachard_targets_list_print(struct reachard_targets_list *list) {
    char *result;
    size_t result_size = 0;
    FILE *stream = open_memstream(&result, &result_size);

    struct reachard_targets_list_item *current;
    for (current = list->head; current; current = current->next) {
        fprintf(stream, "%d -> ", current->id);
    }
    fprintf(stream, "\n");
    fclose(stream);

    return result;
}

static void
reachard_targets_list_add(struct reachard_targets_list *list, int id) {
    struct reachard_targets_list_item *item =
        calloc(1, sizeof(struct reachard_targets_list_item));
    item->id = id;

    if (!list->head) {
        list->head = item;
        list->tail = item;
    } else {
        list->tail->next = item;
        list->tail = item;
    }
}

static void
reachard_targets_list_delete(struct reachard_targets_list *list, int id) {
    struct reachard_targets_list_item *prev, *current;
    for (current = list->head; current; current = current->next) {
        if (current->id == id) {
            prev->next = current->next;
            free(current);
            break;
        }
        prev = current;
    }
}

void
reachard_targets_list_destroy(struct reachard_targets_list list) {
    struct reachard_targets_list_item *current, *next;
    for (current = list.head; current; current = next) {
        next = current->next;
        free(current);
    }
}

struct reachard_connection_info {
    reachard_handler handle;
    struct MHD_PostProcessor *postprocessor;
};

static void
reachard_connection_info_destroy(struct reachard_connection_info *conn_info) {
    if (!conn_info) {
        return;
    }

    if (conn_info->postprocessor) {
        MHD_destroy_post_processor(conn_info->postprocessor);
    }

    free(conn_info);
    conn_info = NULL;
}

static enum MHD_Result
reachard_handle_targets_get(struct reachard_request *request) {
    struct reachard_targets_list *targets_list = request->cls;
    char *target_list_printed = reachard_targets_list_print(targets_list);

    return reachard_request_respond_with_free(request, target_list_printed, MHD_HTTP_OK);
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

static enum MHD_Result
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

static enum MHD_Result
reachard_handle_first_call(struct reachard_request *request) {
    if (strcmp(request->url, "/targets/") == 0) {
        return reachard_handle_targets_first_call(request);
    }

    return reachard_request_respond(request, "url not allowed", MHD_HTTP_BAD_REQUEST);
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
