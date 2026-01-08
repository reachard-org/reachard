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

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>

#include <microhttpd.h>

#include "handle.h"

static void
reachard_interrupt(int sig, siginfo_t *info, void *ucontext) {
    printf("\rShutting down! [%d]\n", sig);
}

int
reachard_serve(const int port) {
    struct reachard_targets_list targets_list = {0};

    struct MHD_Daemon *daemon = MHD_start_daemon(
        MHD_USE_INTERNAL_POLLING_THREAD, port,
        NULL, NULL,
        &reachard_handle, &targets_list,
        MHD_OPTION_NOTIFY_COMPLETED, &reachard_handle_complete, NULL,
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

    printf("Listening on :%d\n", port);
    pause();

    MHD_stop_daemon(daemon);
    reachard_targets_list_destroy(targets_list);

    return 0;
}
