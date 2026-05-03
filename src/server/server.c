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

#include "server.h"

#include <database/database.h>
#include <server/handle.h>

#include <stdio.h>

#include <microhttpd.h>

int
reachard_server_init(
    struct reachard_server *server,
    struct reachard_db *db,
    uint16_t port
) {
    server->db = db;
    server->port = port;

    return 0;
}

int
reachard_server_start(struct reachard_server *server) {
    server->daemon = MHD_start_daemon(
        MHD_USE_EPOLL_INTERNAL_THREAD, server->port,
        0, 0,
        &reachard_handle, server->db,
        MHD_OPTION_NOTIFY_COMPLETED, &reachard_handle_complete, 0,
        MHD_OPTION_END
    );
    if (!server->daemon) {
        fprintf(stderr, "failed to start the daemon\n");
        return 1;
    }

    return 0;
}

void
reachard_server_stop(struct reachard_server *server) {
    MHD_stop_daemon(server->daemon);
}
