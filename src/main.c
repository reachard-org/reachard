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

#include <microhttpd.h>
#include <stdio.h>
#include <string.h>

#define PORT 7272

enum MHD_Result
reachard_handler (void *cls, struct MHD_Connection *connection,
                  const char *url, const char *method, const char *version,
                  const char *upload_data, size_t *upload_data_size,
                  void **req_cls)
{
  const char *content = "<html><body>Hello there!</body></html>";

  struct MHD_Response *response
      = MHD_create_response_from_buffer_static (strlen (content), content);
  const enum MHD_Result result
      = MHD_queue_response (connection, MHD_HTTP_OK, response);

  MHD_destroy_response (response);

  return result;
};

int
main ()
{
  struct MHD_Daemon *daemon
      = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                          reachard_handler, NULL, MHD_OPTION_END);
  if (!daemon)
    return 1;

  printf ("Server started. Press Enter to exit.\n");
  getchar ();
  printf ("\e[1F\e[2K");
  fflush (stdout);

  MHD_stop_daemon (daemon);

  return 0;
}
