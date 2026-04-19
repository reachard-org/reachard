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

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <microhttpd.h>

#include <database/database.h>
#include <database/migrate.h>
#include <server/handle.h>

#include "server.h"

static void
reachard_cleanup(struct reachard_server *server) {
    reachard_db_cleanup(&server->db);
}

bool
reachard_init(struct reachard_server *server, const char *db_url) {
    if (!reachard_db_init(&server->db, db_url)) {
        fprintf(stderr, "failed to connect to the database\n");
        goto failure;
    }

    if (!reachard_db_migrate(&server->db)) {
        fprintf(stderr, "failed to apply migrations to the database\n");
        goto failure;
    }

    return true;

failure:
    reachard_cleanup(server);
    return false;
}

static void
reachard_interrupt(int sig, siginfo_t *info, void *ucontext) {
    printf("\rShutting down! [%d]\n", sig);
}

bool
reachard_serve(struct reachard_server *server, const int port) {
    bool result = false;

    struct MHD_Daemon *daemon = MHD_start_daemon(
        MHD_USE_EPOLL_INTERNAL_THREAD, port,
        0, 0,
        &reachard_handle, &server->db,
        MHD_OPTION_NOTIFY_COMPLETED, &reachard_handle_complete, 0,
        MHD_OPTION_END
    );
    if (!daemon) {
        fprintf(stderr, "failed to start the daemon\n");
        goto cleanup;
    }

    const struct sigaction act = {
        .sa_sigaction = &reachard_interrupt,
        .sa_flags = SA_SIGINFO
    };
    sigaction(SIGINT, &act, 0);
    sigaction(SIGTERM, &act, 0);

    printf("Listening on :%d\n", port);
    pause();

    MHD_stop_daemon(daemon);
    result = true;

cleanup:
    reachard_cleanup(server);
    return result;
}
