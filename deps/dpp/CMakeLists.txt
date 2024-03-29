#
# This file is part of the WarheadCore Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

# Group sources
GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_library(libdpp STATIC ${PRIVATE_SOURCES})

target_compile_definitions(libdpp
  PUBLIC
    -DDPP_BUILD)

if (WIN32 AND NOT MINGW)
  target_compile_definitions(libdpp
    PUBLIC
#     -DWIN32_LEAN_AND_MEAN
      -D_WINSOCK_DEPRECATED_NO_WARNINGS
      -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

if (MSVC)
  target_compile_options(libdpp
    PRIVATE
      /DFD_SETSIZE=1024
      /sdl
      $<$<NOT:$<CONFIG:Debug>>:/O2>)
endif()

target_include_directories(libdpp
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libdpp
  PRIVATE
    warhead-dependency-interface
    zlib
    openssl
    threads)

# Generate precompiled header
target_precompile_headers(libdpp PRIVATE PrecompiledHeaders/libdppPCH.h)

set_target_properties(libdpp
  PROPERTIES
    FOLDER
      "deps")
