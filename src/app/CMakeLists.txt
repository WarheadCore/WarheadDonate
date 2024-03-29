#
# This file is part of the WarheadApp Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

macro(GenerateApp dir appName)
  CollectSourceFiles(
    ${dir}
    PRIVATE_SOURCES)

  if (WIN32)
    list(APPEND PRIVATE_SOURCES ${winDebugging})
  endif()

  if (MSVC)
    list(APPEND PRIVATE_SOURCES ${dir}/${appName}.rc)
  endif()

  GroupSources(${dir})

  add_executable(${appName}
    ${PRIVATE_SOURCES})

  target_link_libraries(${appName}
    PRIVATE
      warhead-core-interface
    PUBLIC
      database
      libdpp)

  CollectIncludeDirectories(
    ${dir}
    PUBLIC_INCLUDES)

  target_include_directories(${appName}
    PUBLIC
      ${PUBLIC_INCLUDES}
    PRIVATE
      ${CMAKE_CURRENT_BINARY_DIR})

  set_target_properties(${appName}
    PROPERTIES
      FOLDER
        "app")

  # Install config
  CopyDefaultConfig(${appName})

  if (UNIX)
    install(TARGETS ${appName} DESTINATION bin)
  elseif (WIN32)
    install(TARGETS ${appName} DESTINATION "${CMAKE_INSTALL_PREFIX}")
  endif()
endmacro()

GenerateApp(${CMAKE_CURRENT_SOURCE_DIR} "WarheadDonate")
