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

#define _POSIX_C_SOURCE 199309L

#include <microhttpd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 7272

struct reachard_request
{
  void *cls;
  struct MHD_Connection *connection;
  const char *url;
  const char *method;
  const char *version;
  const char *upload_data;
  size_t *upload_data_size;
  void **req_cls;
};

typedef enum MHD_Result (*reachard_handler) (struct reachard_request *request);

struct reachard_connection_info
{
  reachard_handler handle;
};

static enum MHD_Result
reachard_respond (struct reachard_request *request, const char *content,
                  const unsigned int status_code)
{
  struct MHD_Response *response
      = MHD_create_response_from_buffer_static (strlen (content), content);
  const enum MHD_Result result
      = MHD_queue_response (request->connection, status_code, response);

  MHD_destroy_response (response);

  return result;
}

static enum MHD_Result
reachard_handle_targets_get (struct reachard_request *request)
{
  return reachard_respond (request, "hello from targets GET!", MHD_HTTP_OK);
}

static enum MHD_Result
reachard_handle_targets_post (struct reachard_request *request)
{
  return reachard_respond (request, "hello from targets POST!", MHD_HTTP_OK);
}

static enum MHD_Result
reachard_handle_next_calls_with (struct reachard_request *request,
                                 reachard_handler handler)
{
  if (!*request->req_cls)
    *request->req_cls = malloc (sizeof (struct reachard_connection_info));

  struct reachard_connection_info *conn_info = *request->req_cls;
  if (!conn_info)
    return MHD_NO;

  conn_info->handle = handler;

  return MHD_YES;
}

static enum MHD_Result
reachard_handle_first_call_targets (struct reachard_request *request)
{
  if (strcmp (request->method, "GET") == 0)
    return reachard_handle_next_calls_with (request,
                                            &reachard_handle_targets_get);
  if (strcmp (request->method, "POST") == 0)
    return reachard_handle_next_calls_with (request,
                                            &reachard_handle_targets_post);

  return reachard_respond (request, "method not allowed",
                           MHD_HTTP_BAD_REQUEST);
}

static enum MHD_Result
reachard_handle_first_call (struct reachard_request *request)
{
  if (strcmp (request->url, "/targets/") == 0)
    return reachard_handle_first_call_targets (request);

  return reachard_respond (request, "url not allowed", MHD_HTTP_BAD_REQUEST);
}

static enum MHD_Result
reachard_handle (void *cls, struct MHD_Connection *conn, const char *url,
                 const char *method, const char *version,
                 const char *upload_data, size_t *upload_data_size,
                 void **req_cls)
{
  /* This function is called several times over the lifetime of a request */
  struct reachard_request request
      = { cls,    conn, url, method, version, upload_data, upload_data_size,
          req_cls };

  struct reachard_connection_info *conn_info = *req_cls;

  /* First call has headers available only */
  if (!conn_info)
    return reachard_handle_first_call (&request);

  /* Second call has body available as well */
  return conn_info->handle (&request);
}

void
reachard_complete (void *cls, struct MHD_Connection *conn, void **req_cls,
                   enum MHD_RequestTerminationCode toe)
{
  struct reachard_connection_info *conn_info = *req_cls;

  if (conn_info)
    free (conn_info);
}

static void
reachard_interrupt (int sig, siginfo_t *info, void *ucontext)
{
  printf ("\rShutting down! [%d]\n", sig);
}

int
main ()
{
  struct MHD_Daemon *daemon
      = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                          &reachard_handle, NULL, MHD_OPTION_NOTIFY_COMPLETED,
                          &reachard_complete, NULL, MHD_OPTION_END);
  if (!daemon)
    return 1;

  const struct sigaction act
      = { .sa_sigaction = &reachard_interrupt, .sa_flags = SA_SIGINFO };
  sigaction (SIGINT, &act, NULL);
  sigaction (SIGTERM, &act, NULL);

  printf ("Listening on :%d\n", PORT);
  pause ();

  MHD_stop_daemon (daemon);

  return 0;
}
