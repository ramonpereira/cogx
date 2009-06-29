# Install script for directory: /home/plison/svn.cogx/binding/development/tools/cast/src/c++/cast/core

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Debug")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" MATCHES "^(Unspecified)$")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    IF(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cast/libCASTCore.so")
      FILE(RPATH_CHECK
           FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cast/libCASTCore.so"
           RPATH "")
    ENDIF(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cast/libCASTCore.so")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cast" TYPE SHARED_LIBRARY FILES "/home/plison/svn.cogx/binding/development/BUILD/tools/cast/src/c++/cast/core/libCASTCore.so")
    IF(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cast/libCASTCore.so")
      FILE(RPATH_REMOVE
           FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cast/libCASTCore.so")
      IF(CMAKE_INSTALL_DO_STRIP)
        EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cast/libCASTCore.so")
      ENDIF(CMAKE_INSTALL_DO_STRIP)
    ENDIF(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cast/libCASTCore.so")
  ENDIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" MATCHES "^(Unspecified)$")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" MATCHES "^(Unspecified)$")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/cast/core" TYPE FILE FILES
    "/home/plison/svn.cogx/binding/development/tools/cast/src/c++/cast/core/CASTUtils.hpp"
    "/home/plison/svn.cogx/binding/development/tools/cast/src/c++/cast/core/CASTComponent.hpp"
    "/home/plison/svn.cogx/binding/development/tools/cast/src/c++/cast/core/SubarchitectureComponent.hpp"
    "/home/plison/svn.cogx/binding/development/tools/cast/src/c++/cast/core/CASTComponentPermissionsMap.hpp"
    "/home/plison/svn.cogx/binding/development/tools/cast/src/c++/cast/core/CASTWorkingMemory.hpp"
    "/home/plison/svn.cogx/binding/development/tools/cast/src/c++/cast/core/CASTWMPermissionsMap.hpp"
    "/home/plison/svn.cogx/binding/development/tools/cast/src/c++/cast/core/CASTData.hpp"
    "/home/plison/svn.cogx/binding/development/tools/cast/src/c++/cast/core/CASTWorkingMemoryInterface.hpp"
    "/home/plison/svn.cogx/binding/development/tools/cast/src/c++/cast/core/StringMap.hpp"
    "/home/plison/svn.cogx/binding/development/tools/cast/src/c++/cast/core/CASTTimer.hpp"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" MATCHES "^(Unspecified)$")

