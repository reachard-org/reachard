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

function(find_package_via_find_package)
  include(FindPackageHandleStandardArgs)

  set(options)
  set(one_value_keywords PACKAGE_NAME ALIAS_NAME TARGET_NAME)
  set(multi_value_keywords)
  cmake_parse_arguments(
    PARSE_ARGV 0
    arg
    "${options}"
    "${one_value_keywords}"
    "${multi_value_keywords}"
  )

  if(NOT DEFINED arg_PACKAGE_NAME)
    set(arg_PACKAGE_NAME ${CMAKE_FIND_PACKAGE_NAME})
  endif()

  if(NOT DEFINED arg_ALIAS_NAME)
    set(arg_ALIAS_NAME ${arg_PACKAGE_NAME})
  endif()

  if(NOT DEFINED arg_TARGET_NAME)
    message(SEND_ERROR "TARGET_NAME must be defined.")
  endif()

  find_package(${arg_PACKAGE_NAME} QUIET CONFIG)

  include(FindPackageHandleStandardArgs)

  find_package_handle_standard_args(${arg_PACKAGE_NAME} CONFIG_MODE)

  if(NOT TARGET ${arg_PACKAGE_NAME}::${arg_ALIAS_NAME})
    add_library(${arg_PACKAGE_NAME}::${arg_ALIAS_NAME} ALIAS ${arg_TARGET_NAME})
  endif()
endfunction()

function(find_package_via_pkg_config)
  include(FindPackageHandleStandardArgs)

  find_package(PkgConfig REQUIRED)

  set(options)
  set(one_value_keywords PACKAGE_NAME TARGET_NAME HEADER_NAME LIBRARY_NAME)
  set(multi_value_keywords)
  cmake_parse_arguments(
    PARSE_ARGV 0
    arg
    "${options}"
    "${one_value_keywords}"
    "${multi_value_keywords}"
  )

  if(NOT DEFINED arg_PACKAGE_NAME)
    set(arg_PACKAGE_NAME ${CMAKE_FIND_PACKAGE_NAME})
  endif()

  if(NOT DEFINED arg_TARGET_NAME)
    set(arg_TARGET_NAME ${arg_PACKAGE_NAME})
  endif()

  if(NOT DEFINED arg_HEADER_NAME)
    message(SEND_ERROR "HEADER_NAME must be defined.")
  endif()

  if(NOT DEFINED arg_LIBRARY_NAME)
    message(SEND_ERROR "LIBRARY_NAME must be defined.")
  endif()

  pkg_check_modules(PC_${arg_PACKAGE_NAME} QUIET ${arg_PACKAGE_NAME})

  find_path(
    ${arg_PACKAGE_NAME}_INCLUDE_DIR
    NAMES ${arg_HEADER_NAME}
    HINTS PC_${arg_PACKAGE_NAME}_INCLUDE_DIRS
    PATH_SUFFIXES ${arg_PACKAGE_NAME}
  )

  find_library(
    ${arg_PACKAGE_NAME}_LIBRARY
    NAMES ${arg_LIBRARY_NAME}
    HINTS PC_${arg_PACKAGE_NAME}_LIBRARY_DIRS
  )

  set(${arg_PACKAGE_NAME}_VERSION ${PC_${arg_PACKAGE_NAME}_VERSION})

  find_package_handle_standard_args(
    ${arg_PACKAGE_NAME}
    REQUIRED_VARS
      ${arg_PACKAGE_NAME}_INCLUDE_DIR
      ${arg_PACKAGE_NAME}_LIBRARY
      ${arg_PACKAGE_NAME}_VERSION
    VERSION_VAR ${arg_PACKAGE_NAME}_VERSION
  )

  add_library(${arg_PACKAGE_NAME}::${arg_TARGET_NAME} UNKNOWN IMPORTED)
  set_target_properties(
    ${arg_PACKAGE_NAME}::${arg_TARGET_NAME}
    PROPERTIES
      IMPORTED_LOCATION "${${arg_PACKAGE_NAME}_LIBRARY}"
      INTERFACE_COMPILE_OPTIONS "${PC_${arg_PACKAGE_NAME}_CFLAGS_OTHER}"
      INTERFACE_INCLUDE_DIRECTORIES "${${arg_PACKAGE_NAME}_INCLUDE_DIR}"
  )
endfunction()
