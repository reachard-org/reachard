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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "client/client.h"
#include "database/database.h"
#include "database/migrate.h"
#include "server/server.h"

struct reachard_env {
    uint16_t port;
    char *db_url;
};

static int
reachard_env_init(struct reachard_env *env) {
    char *port_str = getenv("REACHARD_PORT");
    int port = 7272;
    if (port_str) {
        port = atoi(port_str);
        if (!port) {
            fprintf(stderr, "couldn't parse `REACHARD_PORT` as a number\n");
            return 1;
        }
    }
    env->port = port;

    char *db_url = getenv("REACHARD_DB_URL");
    if (!db_url) {
        db_url = "postgresql://reachard@/reachard";
    }
    env->db_url = db_url;

    return 0;
}

static void
reachard_interrupt(int sig, siginfo_t *info, void *ucontext) {
    printf("\rShutting down! [%d]\n", sig);
}

static void
reachard_pause() {
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

    struct reachard_env env;
    reachard_env_init(&env);

    struct reachard_db db;

    if (reachard_db_init(&db, env.db_url)) {
        fprintf(stderr, "failed to initialize the database\n");
        goto cleanup_db;
    }

    if (reachard_db_migrate(&db)) {
        fprintf(stderr, "failed to apply migrations to the database\n");
        goto cleanup_db;
    }

    struct reachard_server server;
    reachard_server_init(&server, &db, env.port);

    struct reachard_client client;
    if (reachard_client_init(&client)) {
        fprintf(stderr, "failed to initialize the client");
        goto cleanup_client;
    };

    if (reachard_server_start(&server)) {
        fprintf(stderr, "failed to start the server\n");
        goto cleanup;
    };

    if (reachard_client_start(&client)) {
        fprintf(stderr, "failed to start the client");
        goto cleanup;
    }

    printf("Listening on :%d\n", env.port);
    reachard_pause();

    reachard_server_stop(&server);
    reachard_client_stop(&client);

    result = 0;

cleanup:
cleanup_client:
    reachard_client_cleanup(&client);
cleanup_db:
    reachard_db_cleanup(&db);
    return result;
}
