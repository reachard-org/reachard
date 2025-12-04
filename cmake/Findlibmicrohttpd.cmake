# Copyright 2025 Pavel Sobolev
#
# This file is part of the Reachard project, located at
#
#     https://reachard.paveloom.dev
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

set(_pkg_name ${CMAKE_FIND_PACKAGE_NAME})

find_package(PkgConfig)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_${_pkg_name} QUIET ${_pkg_name})
endif()

find_path(
  ${_pkg_name}_INCLUDE_DIR
  NAMES microhttpd.h
  HINTS PC_${_pkg_name}_INCLUDE_DIRS
  PATH_SUFFIXES ${_pkg_name}
)

find_library(
  ${_pkg_name}_LIBRARY
  NAMES microhttpd
  HINTS PC_${_pkg_name}_LIBRARY_DIRS
)

set(${_pkg_name}_VERSION ${PC_${_pkg_name}_VERSION})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  ${_pkg_name}
  REQUIRED_VARS ${_pkg_name}_INCLUDE_DIR ${_pkg_name}_LIBRARY
  VERSION_VAR ${_pkg_name}_VERSION
)

if(${_pkg_name}_FOUND AND NOT TARGET ${_pkg_name}::MHD)
  add_library(${_pkg_name}::MHD UNKNOWN IMPORTED)
  set_target_properties(
    ${_pkg_name}::MHD
    PROPERTIES
      IMPORTED_LOCATION "${${_pkg_name}_LIBRARY}"
      INTERFACE_COMPILE_OPTIONS "${PC_${_pkg_name}_CFLAGS_OTHER}"
      INTERFACE_INCLUDE_DIRECTORIES "${${_pkg_name}_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(${_pkg_name}_INCLUDE_DIR ${_pkg_name}_LIBRARY)
