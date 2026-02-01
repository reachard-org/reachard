# Reachard
# Copyright 2026 Pavel Sobolev
#
# This file is part of the Reachard project, located at
# <https://reachard.paveloom.dev/>.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: AGPL-3.0-or-later

cmake_minimum_required(VERSION 4.0)

project(reachard VERSION 0.1.0 LANGUAGES C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(cJSON 1 REQUIRED)
find_package(libmicrohttpd 1 REQUIRED) # Must have `epoll` support enabled
find_package(PostgreSQL 18 REQUIRED)

add_executable(reachard)
target_compile_features(reachard PRIVATE c_std_23)
target_link_libraries(
  reachard
  PRIVATE cJSON::cJSON libmicrohttpd::MHD PostgreSQL::PostgreSQL
)

target_sources(
  reachard
  PRIVATE src/main.c
  PRIVATE src/database/database.c src/database/targets.c
  PRIVATE
    src/server/handle.c
    src/server/handle/targets.c
    src/server/request.c
    src/server/server.c
)
