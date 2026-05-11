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

function(version_function)
  file(
    READ ${${arg_PACKAGE_NAME}_INCLUDE_DIR}/${arg_HEADER_NAME}
    _header_content
    LIMIT 2000
  )
  string(
    REGEX MATCHALL "#define UTHASH_VERSION ([0-9]+.[0-9]+.[0-9]+)"
    ${_header_version}
    ${_header_content}
  )
  set(${arg_PACKAGE_NAME}_VERSION ${CMAKE_MATCH_1} PARENT_SCOPE)
endfunction()

find_package_via_files(HEADER_NAME uthash.h VERSION_FUNCTION version_function)
