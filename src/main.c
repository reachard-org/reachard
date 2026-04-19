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
#include <stdlib.h>
#include <unistd.h>

#include "server/server.h"

static void
reachard_interrupt(int sig, siginfo_t *info, void *ucontext) {
    printf("\rShutting down! [%d]\n", sig);
}

static void
reachard_wait() {
    const struct sigaction act = {
        .sa_sigaction = &reachard_interrupt,
        .sa_flags = SA_SIGINFO
    };
    sigaction(SIGINT, &act, 0);
    sigaction(SIGTERM, &act, 0);

    pause();
}

int
main() {
    int result = 1;

    char *port_str = getenv("REACHARD_PORT");
    int port = 7272;

    if (port_str) {
        port = atoi(port_str);
        if (!port) {
            fprintf(stderr, "couldn't parse `REACHARD_PORT` as a number\n");
            return 1;
        }
    }

    char *db_url = getenv("REACHARD_DB_URL");

    if (!db_url) {
        db_url = "postgresql://reachard@/reachard";
    }

    struct reachard_server *server = &(struct reachard_server){0};
    if (reachard_server_init(server, db_url)) {
        fprintf(stderr, "failed to initialize the server\n");
        goto cleanup;
    }

    reachard_server_start(server, port);

    printf("Listening on :%d\n", port);
    reachard_wait();

    reachard_server_stop(server);

    result = 0;

cleanup:
    reachard_server_cleanup(server);
    return result;
}
