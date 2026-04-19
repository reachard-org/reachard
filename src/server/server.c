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

#include <stdio.h>

#include <microhttpd.h>

#include <database/database.h>
#include <database/migrate.h>
#include <server/handle.h>

#include "server.h"

void
reachard_server_cleanup(struct reachard_server *server) {
    reachard_db_cleanup(&server->db);
}

int
reachard_server_init(struct reachard_server *server, const char *db_url) {
    if (!reachard_db_init(&server->db, db_url)) {
        fprintf(stderr, "failed to connect to the database\n");
        goto failure;
    }

    if (!reachard_db_migrate(&server->db)) {
        fprintf(stderr, "failed to apply migrations to the database\n");
        goto failure;
    }

    return 0;

failure:
    reachard_server_cleanup(server);
    return 1;
}

int
reachard_server_start(struct reachard_server *server, const int port) {
    server->daemon = MHD_start_daemon(
        MHD_USE_EPOLL_INTERNAL_THREAD, port,
        0, 0,
        &reachard_handle, &server->db,
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
