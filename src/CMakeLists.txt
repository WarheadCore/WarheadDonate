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
# User has manually chosen to ignore the git-tests, so throw them a warning.
# This is done EACH compile so they can be alerted about the consequences.
#

# Crash logs
set(winDebugging
  ${CMAKE_SOURCE_DIR}/src/common/Debugging/WheatyExceptionReport.cpp
  ${CMAKE_SOURCE_DIR}/src/common/Debugging/WheatyExceptionReport.h)

add_subdirectory(app)
add_subdirectory(common)
add_subdirectory(database)
add_subdirectory(genrev)
