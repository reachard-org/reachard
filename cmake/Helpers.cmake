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

include(FindPackageHandleStandardArgs)

function(find_package_via_find_package)
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
    message(FATAL_ERROR "TARGET_NAME must be defined.")
  endif()

  find_package(${arg_PACKAGE_NAME} QUIET CONFIG)

  include(FindPackageHandleStandardArgs)

  find_package_handle_standard_args(${arg_PACKAGE_NAME} CONFIG_MODE)

  if(NOT TARGET ${arg_PACKAGE_NAME}::${arg_ALIAS_NAME})
    add_library(${arg_PACKAGE_NAME}::${arg_ALIAS_NAME} ALIAS ${arg_TARGET_NAME})
  endif()
endfunction()

function(find_package_via_files)
  set(options)
  set(
    one_value_keywords
    PACKAGE_NAME
    TARGET_NAME
    HEADER_NAME
    LIBRARY_NAME
    VERSION_FUNCTION
  )
  set(multi_value_keywords HEADER_HINTS LIBRARY_HINTS)
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

  if(NOT DEFINED arg_HEADER_NAME OR arg_HEADER_NAME STREQUAL "")
    message(FATAL_ERROR "HEADER_NAME must be defined and not empty.")
  endif()

  if(DEFINED arg_LIBRARY_NAME AND arg_LIBRARY_NAME STREQUAL "")
    message(FATAL_ERROR "LIBRARY_NAME must not be empty")
  endif()

  if(NOT DEFINED arg_VERSION_FUNCTION)
    message(FATAL_ERROR "VERSION_FUNCTION must be defined.")
  endif()

  find_path(
    ${arg_PACKAGE_NAME}_INCLUDE_DIR
    NAMES ${arg_HEADER_NAME}
    HINTS ${arg_HEADER_HINTS}
    PATH_SUFFIXES ${arg_PACKAGE_NAME}
  )

  if(DEFINED arg_LIBRARY_NAME)
    find_library(
      ${arg_PACKAGE_NAME}_LIBRARY
      NAMES ${arg_LIBRARY_NAME}
      HINTS ${arg_LIBRARY_HINTS}
    )
  endif()

  cmake_language(CALL ${arg_VERSION_FUNCTION})

  set(
    _required_vars
    ${arg_PACKAGE_NAME}_INCLUDE_DIR
    ${arg_PACKAGE_NAME}_VERSION
  )
  if(DEFINED arg_LIBRARY_NAME)
    list(APPEND _required_vars ${arg_PACKAGE_NAME}_LIBRARY)
  endif()

  find_package_handle_standard_args(
    ${arg_PACKAGE_NAME}
    REQUIRED_VARS ${_required_vars}
    VERSION_VAR ${arg_PACKAGE_NAME}_VERSION
  )

  if(DEFINED arg_LIBRARY_NAME)
    set(_library_type UNKNOWN)
  else()
    set(_library_type INTERFACE)
  endif()

  set(
    _properties
    INTERFACE_INCLUDE_DIRECTORIES
    "${${arg_PACKAGE_NAME}_INCLUDE_DIR}"
  )
  if(DEFINED arg_LIBRARY_NAME)
    list(APPEND _properties IMPORTED_LOCATION "${${arg_PACKAGE_NAME}_LIBRARY}")
  endif()

  add_library(${arg_PACKAGE_NAME}::${arg_TARGET_NAME} ${_library_type} IMPORTED)
  set_target_properties(
    ${arg_PACKAGE_NAME}::${arg_TARGET_NAME}
    PROPERTIES ${_properties}
  )
endfunction()

function(find_package_via_pkg_config)
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
    message(FATAL_ERROR "HEADER_NAME must be defined.")
  endif()

  if(NOT DEFINED arg_LIBRARY_NAME)
    message(FATAL_ERROR "LIBRARY_NAME must be defined.")
  endif()

  pkg_check_modules(PC_${arg_PACKAGE_NAME} QUIET ${arg_PACKAGE_NAME})

  function(version_function)
    set(
      ${arg_PACKAGE_NAME}_VERSION
      ${PC_${arg_PACKAGE_NAME}_VERSION}
      PARENT_SCOPE
    )
  endfunction()

  find_package_via_files(
    PACKAGE_NAME ${arg_PACKAGE_NAME}
    TARGET_NAME ${arg_TARGET_NAME}
    HEADER_NAME ${arg_HEADER_NAME}
    LIBRARY_NAME ${arg_LIBRARY_NAME}
    HEADER_HINTS PC_${arg_PACKAGE_NAME}_INCLUDE_DIRS
    LIBRARY_HINTS PC_${arg_PACKAGE_NAME}_LIBRARY_DIRS
    VERSION_FUNCTION version_function
  )
endfunction()
