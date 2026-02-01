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
#include <stdlib.h>

#include "server/server.h"

int
main() {
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

    return reachard_serve(port, db_url);
}
