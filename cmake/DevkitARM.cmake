
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

if(NOT DEVKITARM)
  set(DEVKITARM $ENV{DEVKITARM} CACHE PATH "DEVKITARM path")
endif()
if(NOT DEVKITPRO)
  set(DEVKITPRO $ENV{DEVKITPRO} CACHE PATH "DEVKITPRO path")
endif()

if(NOT DEVKITARM)
  message(FATAL_ERROR
    "Please set DEVKITARM in your environment.")
endif()

if(NOT DEVKITPRO)
  message(FATAL_ERROR
    "Please set DEVKITPRO in your environment.")
endif()
set(CMAKE_SYSTEM_PROCESSOR arm-none-eabi)
if(NOT EXISTS ${DEVKITARM}/bin/${CMAKE_SYSTEM_PROCESSOR}-gcc)
  set(CMAKE_SYSTEM_PROCESSOR arm-eabi)
  if(NOT EXISTS ${DEVKITARM}/bin/${CMAKE_SYSTEM_PROCESSOR}-gcc)
    message(FATAL_ERROR "unable to find suitable arm-gcc")
  endif()
endif()

set(CMAKE_C_COMPILER ${DEVKITARM}/bin/${CMAKE_SYSTEM_PROCESSOR}-gcc)
set(CMAKE_CXX_COMPILER ${DEVKITARM}/bin/${CMAKE_SYSTEM_PROCESSOR}-g++)
set(CMAKE_FIND_ROOT_PATH ${DEVKITARM})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_LIBRARY_PREFIXES lib)
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()
