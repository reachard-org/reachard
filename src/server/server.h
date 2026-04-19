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

#pragma once

#include <stdint.h>

#include <database/database.h>

struct reachard_server {
    struct reachard_db *db;
    uint16_t port;
    struct MHD_Daemon *daemon;
};

int
reachard_server_init(
    struct reachard_server *server,
    struct reachard_db *db,
    uint16_t port
);

int
reachard_server_start(struct reachard_server *server);

void
reachard_server_stop(struct reachard_server *server);
