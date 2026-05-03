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
find_package(CURL 8 REQUIRED)
find_package(libmicrohttpd 1 REQUIRED) # Must have `epoll` support enabled
find_package(libuv 1 REQUIRED)
find_package(PostgreSQL 18 REQUIRED)

add_executable(gen_migrations)
target_sources(gen_migrations PRIVATE generators/migrations.c)

file(GLOB MIGRATION_FILES CONFIGURE_DEPENDS src/database/migrations/*.sql)
set(MIGRATIONS_HEADER ${CMAKE_SOURCE_DIR}/src/database/migrations.h)
set(MIGRATION_FILES_LIST "${CMAKE_BINARY_DIR}/migrations.txt")

if(EXISTS "${MIGRATION_FILES_LIST}")
  file(READ "${MIGRATION_FILES_LIST}" OLD_MIGRATION_FILES)
endif()

if(NOT "${MIGRATION_FILES}" STREQUAL "${OLD_MIGRATION_FILES}")
  file(WRITE "${MIGRATION_FILES_LIST}" "${MIGRATION_FILES}")
endif()

add_custom_command(
  OUTPUT ${MIGRATIONS_HEADER}
  DEPENDS "${MIGRATION_FILES}" ${MIGRATION_FILES_LIST}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/database
  COMMAND gen_migrations migrations
)

add_executable(reachard)
target_compile_definitions(reachard PRIVATE _POSIX_C_SOURCE=202405L)
target_compile_features(reachard PRIVATE c_std_23)
target_link_libraries(
  reachard
  PRIVATE
    cJSON::cJSON
    CURL::libcurl
    libmicrohttpd::MHD
    libuv::libuv
    PostgreSQL::PostgreSQL
)
target_sources(
  reachard
  PRIVATE src/main.c
  PRIVATE src/client/client.c src/client/state.c src/client/targets.c
  PRIVATE src/database/database.c src/database/migrate.c src/database/targets.c
  PRIVATE
    src/server/handle.c
    src/server/handle/targets.c
    src/server/request.c
    src/server/server.c
  PRIVATE
    FILE_SET HEADERS
      BASE_DIRS ${CMAKE_SOURCE_DIR}/src
      FILES ${MIGRATIONS_HEADER}
)
