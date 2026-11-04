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

#include "handle.h"

#include "utils/constants.h"
#include <server/handle/targets.h>
#include <server/request.h>
#include <server/response.h>

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <microhttpd.h>

// "^/endpoint/(([0-9]{1,n})/)?$"
#define PATTERN(endpoint) \
    "^/" endpoint "/(([0-9]{1," str(REACHARD_INT_STR_LEN) "})/)?$"

static int
match(struct reachard_request *request, char *pattern) {
    struct reachard_request_state *request_state = *request->state;

    regex_t regex;
    regmatch_t pmatch[3];

    if (regcomp(&regex, pattern, REG_EXTENDED)) {
        fprintf(stderr, "failed to compile regex\n");
        regfree(&regex);
        return 0;
    }

    int matched = regexec(&regex, request->url, 3, pmatch, 0) == 0;

    if (matched) {
        regmatch_t match = pmatch[2];
        if (match.rm_so != -1) {
            char id_str[REACHARD_INT_STR_LEN] = {0};
            const char *src = request->url + match.rm_so;
            size_t len = match.rm_eo - match.rm_so;
            strncpy(id_str, src, len);
            char id = atoi(id_str);

            request_state->type = REACHARD_REQUEST_TYPE_RESOURCE;
            request_state->id = id;
        } else {
            request_state->type = REACHARD_REQUEST_TYPE_COLLECTION;
        };
    }

    regfree(&regex);
    return matched;
}

static enum MHD_Result
reachard_handle_first_call(struct reachard_request *request) {
    struct reachard_request_state *request_state;
    if (reachard_request_state_init(&request_state)) {
        return MHD_NO;
    }
    *request->state = request_state;

    if (match(request, PATTERN("targets"))) {
        return reachard_handle_targets_first_call(request);
    }

    return reachard_respond_plain(
        request, "url not allowed", MHD_HTTP_BAD_REQUEST
    );
}

enum MHD_Result
reachard_handle(
    void *cls, struct MHD_Connection *conn,
    const char *url, const char *method, const char *version,
    const char *upload_data, size_t *upload_data_size,
    void **req_cls
) {
    struct reachard_request request = {
        cls, conn,
        url, method, version,
        upload_data, upload_data_size,
        req_cls
    };

    // This function is called several times over the lifetime of a request,
    // therefore a separate struct is used maintain state
    struct reachard_request_state *state = *req_cls;

    /* First call has headers available only */
    if (!state) {
        return reachard_handle_first_call(&request);
    }

    /* The following calls make body available as well */
    return state->handle(&request);
}

void
reachard_handle_complete(
    void *cls, struct MHD_Connection *conn,
    void **req_cls,
    enum MHD_RequestTerminationCode toe
) {
    reachard_request_state_deinit(*req_cls);
}
