project(gesource)

cmake_minimum_required(VERSION 2.8.12)

set(GESOURCE_VERSION "4.3.0")

set(INSTALL_DIR "${CMAKE_BINARY_DIR}/bin" CACHE FILEPATH "Path to install binaries too.")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

#set(CMAKE_VERBOSE_MAKEFILE TRUE)

add_definitions(-DVPROF_LEVEL=1 -DSWDS -D_snprintf=use_Q_snprintf_instead -Dstricmp=strcasecmp -D_stricmp=strcasecmp
  -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_vsnprintf=vsnprintf -Dstrcmpi=strcasecmp -D_USRDLL -DVECTOR)

if (CMAKE_HOST_UNIX)
  add_definitions(-D_LINUX -DLINUX -D_alloca=alloca -DARCH=i486)
  add_compile_options(--std=c++14 -w -ffast-math -fpermissive -msse4)
else()
  add_definitions(-DWIN32 -D_WIN32 -D_WINDOWS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

add_subdirectory(thirdparty/boost/libs/python)
add_subdirectory(mathlib)
add_subdirectory(tier1)
add_subdirectory(game)
