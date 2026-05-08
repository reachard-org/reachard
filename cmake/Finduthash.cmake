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

set(_pkg_name ${CMAKE_FIND_PACKAGE_NAME})

set(_target_name uthash)
set(_header_name uthash.h)

find_path(
  ${_pkg_name}_INCLUDE_DIR
  NAMES ${_header_name}
  PATH_SUFFIXES ${_pkg_name}
)

file(
  READ ${${_pkg_name}_INCLUDE_DIR}/${_header_name}
  _header_content
  LIMIT 2000
)
string(
  REGEX MATCHALL "#define UTHASH_VERSION ([0-9]+.[0-9]+.[0-9]+)"
  ${_header_version}
  ${_header_content}
)
set(${_pkg_name}_VERSION ${CMAKE_MATCH_1})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  ${_pkg_name}
  REQUIRED_VARS ${_pkg_name}_INCLUDE_DIR ${_pkg_name}_VERSION
  VERSION_VAR ${_pkg_name}_VERSION
)

if(${_pkg_name}_FOUND AND NOT TARGET ${_pkg_name}::${_target_name})
  add_library(${_pkg_name}::${_target_name} INTERFACE IMPORTED)
  set_target_properties(
    ${_pkg_name}::${_target_name}
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${${_pkg_name}_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(${_pkg_name}_INCLUDE_DIR)
