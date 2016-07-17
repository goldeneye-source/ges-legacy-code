project(gesource)

cmake_minimum_required(VERSION 2.8.12)

set(GESOURCE_VERSION "4.3.0")

set(INSTALL_DIR "${CMAKE_BINARY_DIR}/bin" CACHE FILEPATH "Path to install binaries too.")

set(SDK_DIR "" CACHE FILEPATH "Path to the Source 2013 SDK")
if(NOT EXISTS ${SDK_DIR}/bin/steam_api_i486.so)
  message(SEND_ERROR "Cannot find Source 2013 SDK, did you set SDK_DIR variable??")
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)

add_definitions(-DVPROF_LEVEL=1 -DSWDS -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp
  -Dstrnicmp=strncasecmp -D_vsnprintf=vsnprintf -Dstrcmpi=strcasecmp -D_USRDLL
  -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -DVECTOR -DPROTECTED_THINGS_ENABLE
  -Dsprintf=use_Q_snprintf_instead_of_sprintf -Dstrncpy=use_Q_strncpy_instead -D_snprintf=use_Q_snprintf_instead)

if (CMAKE_HOST_UNIX)
  add_definitions(-D_LINUX -DLINUX -D_alloca=alloca)
  add_compile_options(--std=c++0x -w -ffast-math -fpermissive -msse)
else()
  add_definitions(-DWIN32 -D_WIN32 -D_WINDOWS)
endif()

add_compile_options(-Usprintf=use_Q_snprintf_instead_of_sprintf -Ustrncpy=use_Q_strncpy_instead
  -Ufopen=dont_use_fopen -UPROTECTED_THINGS_ENABLE)

add_subdirectory(thirdparty/boost/libs/python)
add_subdirectory(mathlib)
add_subdirectory(tier1)
add_subdirectory(game)
