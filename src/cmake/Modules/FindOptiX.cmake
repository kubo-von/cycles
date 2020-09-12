# - Find OptiX library
# Find the native OptiX includes
# This module defines
#  OPTIX_INCLUDE_DIR, where to find version.h, Set when
#                            OPTIX_INCLUDE_DIR is found.
#  OPTIX_ROOT_DIR, The base directory to search for OptiX.
#                        This can also be an environment variable.
#  OPTIX_FOUND, If false, do not try to use OptiX.
#

#=============================================================================
# Copyright 2011 Blender Foundation.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

# If OPTIX_ROOT_DIR was defined in the environment, use it.
IF(NOT OPTIX_ROOT_DIR AND NOT $ENV{OPTIX_ROOT_DIR} STREQUAL "")
  SET(OPTIX_ROOT_DIR $ENV{OPTIX_ROOT_DIR})
ENDIF()

SET(_optix_SEARCH_DIRS
  ${OPTIX_ROOT_DIR}
  /usr/local
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt/lib/optix
)

FIND_PATH(OPTIX_INCLUDE_DIR
  NAMES
    optix.h
  HINTS
    ${_optix_SEARCH_DIRS}
  PATH_SUFFIXES
    include
)

# handle the QUIETLY and REQUIRED arguments and set OPTIX_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OptiX DEFAULT_MSG
    OPTIX_INCLUDE_DIR)

MARK_AS_ADVANCED(
  OPTIX_INCLUDE_DIR
)
