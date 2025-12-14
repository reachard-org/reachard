/*
 * Copyright 2025 Pavel Sobolev
 *
 * This file is part of the Reachard project, located at
 *
 *     https://reachard.paveloom.dev
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define _POSIX_C_SOURCE 200809L

#include <microhttpd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 7272

struct reachard_request {
    void *cls;
    struct MHD_Connection *conn;
    const char *url, *method, *version;
    const char *upload_data;
    size_t *upload_data_size;
    void **req_cls;
};

typedef enum MHD_Result (*reachard_handler)(struct reachard_request *request);

struct reachard_targets_list_item {
    struct reachard_targets_list_item *next;
    int id;
};

struct reachard_targets_list {
    struct reachard_targets_list_item *head, *tail;
};

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

static void
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
reachard_respond(
    struct reachard_request *request,
    const char *content,
    const unsigned int status_code
) {
    struct MHD_Response *response =
        MHD_create_response_from_buffer_static(strlen(content), content);
    const enum MHD_Result result =
        MHD_queue_response(request->conn, status_code, response);

    MHD_destroy_response(response);

    return result;
}

static enum MHD_Result
reachard_respond_with_free(
    struct reachard_request *request,
    char *content,
    const unsigned int status_code
) {
    struct MHD_Response *response =
        MHD_create_response_from_buffer_with_free_callback_cls(
            strlen(content), content, &free, content
        );
    const enum MHD_Result result =
        MHD_queue_response(request->conn, status_code, response);

    MHD_destroy_response(response);

    return result;
}

static enum MHD_Result
reachard_handle_targets_get(struct reachard_request *request) {
    struct reachard_targets_list *targets_list = request->cls;
    char *target_list_printed = reachard_targets_list_print(targets_list);

    return reachard_respond_with_free(request, target_list_printed, MHD_HTTP_OK);
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

    return reachard_respond(request, "hello from targets DELETE!", MHD_HTTP_OK);
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

    return reachard_respond(request, "hello from targets POST!", MHD_HTTP_OK);
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
        return reachard_respond(
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

    return reachard_respond(request, "url not allowed", MHD_HTTP_BAD_REQUEST);
}

static enum MHD_Result
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
reachard_complete(
    void *cls, struct MHD_Connection *conn,
    void **req_cls,
    enum MHD_RequestTerminationCode toe
) {
    reachard_connection_info_destroy(*req_cls);
}

static void
reachard_interrupt(int sig, siginfo_t *info, void *ucontext) {
    printf("\rShutting down! [%d]\n", sig);
}

int
main() {
    struct reachard_targets_list targets_list = {0};

    struct MHD_Daemon *daemon = MHD_start_daemon(
        MHD_USE_INTERNAL_POLLING_THREAD, PORT,
        NULL, NULL,
        &reachard_handle, &targets_list,
        MHD_OPTION_NOTIFY_COMPLETED, &reachard_complete, NULL,
        MHD_OPTION_END
    );
    if (!daemon) {
        return 1;
    }

    const struct sigaction act = {
        .sa_sigaction = &reachard_interrupt,
        .sa_flags = SA_SIGINFO
    };
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    printf("Listening on :%d\n", PORT);
    pause();

    MHD_stop_daemon(daemon);
    reachard_targets_list_destroy(targets_list);

    return 0;
}
