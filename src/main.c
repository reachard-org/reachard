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

#include <client/client.h>
#include <database/database.h>
#include <database/migrate.h>
#include <env/env.h>
#include <server/server.h>

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

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
    reachard_db_init(&db, &env);

    if (reachard_db_connect(&db)) {
        fprintf(stderr, "failed to connect to the database\n");
        return 1;
    }

    if (reachard_db_migrate(&db)) {
        fprintf(stderr, "failed to apply migrations to the database\n");
        goto deinit_db;
    }

    struct reachard_client client;
    if (reachard_client_init(&client, db)) {
        fprintf(stderr, "failed to initialize the client");
        goto deinit_client;
    };

    struct reachard_server server;
    if (reachard_server_init(&server, db, &env)) {
        fprintf(stderr, "failed to initialize the server");
        goto deinit_server;
    };

    if (reachard_client_start(&client)) {
        fprintf(stderr, "failed to start the client");
        goto deinit;
    }

    if (reachard_server_start(&server)) {
        fprintf(stderr, "failed to start the server\n");
        goto deinit;
    };

    printf("Listening on :%d\n", env.port);
    reachard_pause();

    reachard_server_stop(&server);
    reachard_client_stop(&client);

    result = 0;

deinit:
deinit_server:
    reachard_server_deinit(&server);
deinit_client:
    reachard_client_deinit(&client);
deinit_db:
    reachard_db_deinit(&db);
    return result;
}
