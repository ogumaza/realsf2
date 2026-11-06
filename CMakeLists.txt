# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)
# realsf2 --version reports this version. The file properties of the Windows program and the Info.plist of the macOS
# app show it too.
project(realsf2 VERSION 1.0.1 LANGUAGES CXX)

# The submodules: seqlib writes the MIDI, SoundFont and SFZ files. ntdlib holds the conversion of the Nintendo 64's
# sound data (ntdlib::sequence_export_n64) and the converters' command-line frame (ntdlib::sequence_export). xsflib
# holds the common code that ntdlib builds on, and the CMake helpers. xsflib's XsflibProject.cmake sets the C++
# standard, a release build by default, the static C runtime and the compiler options that keep the output the same on
# every platform.
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/externals/xsflib/CMakeLists.txt")
  message(FATAL_ERROR "externals/xsflib is missing. Run: git submodule update --init")
endif()
include(externals/xsflib/cmake/XsflibProject.cmake)
xsflib_project(WARNINGS_ON_EVERY_TARGET)
set(NTDLIB_SEQUENCE_EXPORT ON)
xsflib_add_libraries(seqlib xsflib ntdlib)

add_executable(realsf2 src/main.cpp)
target_link_libraries(realsf2 PRIVATE ntdlib::sequence_export_n64)
xsflib_add_version(realsf2 VERSION ${PROJECT_VERSION} DEFINE REALSF2_VERSION
                   DESCRIPTION "Nintendo 64 music converter" PRODUCT realsf2
                   COPYRIGHT "Copyright (c) 2026 the realsf2 authors. MIT licence.")

# Finder cannot drop files onto a command-line executable. Package it in a macOS droplet app, with the version in its
# Info.plist.
xsflib_add_droplet_tool(realsf2 APP_NAME Realsf2 VERSION ${PROJECT_VERSION}
                        DESCRIPTION "Drop Nintendo 64 images (.z64, .n64, .v64) here to convert their music to MIDI, SoundFont and SFZ. Output goes in a folder beside each image."
                        PROMPT "Choose Nintendo 64 images (.z64, .n64, .v64) to convert:")

include(CTest)
if(BUILD_TESTING)
  # The program reports the version it was built as.
  string(REPLACE "." "\\." version_pattern "${PROJECT_VERSION}")
  add_test(NAME realsf2_version COMMAND realsf2 --version)
  set_tests_properties(realsf2_version PROPERTIES PASS_REGULAR_EXPRESSION "^realsf2 ${version_pattern}[\r\n]*$")

  # Load local_tests.cmake from each immediate subdirectory to register tests that use locally supplied files.
  xsflib_include_local_tests()
endif()
