project(gesource)

cmake_minimum_required(VERSION 2.8.12)

set(GESOURCE_VERSION "5.0.0")

# Default to a release build
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE
        STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# Global definitions
add_definitions(-DVPROF_LEVEL=1 -DSWDS -D_snprintf=use_Q_snprintf_instead -Dstricmp=strcasecmp -D_stricmp=strcasecmp
  -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_vsnprintf=vsnprintf -Dstrcmpi=strcasecmp -D_USRDLL -DVECTOR)

# Platform specific definitions
if (CMAKE_HOST_UNIX)
  add_definitions(-D_LINUX -DLINUX -D_alloca=alloca -DARCH=i486)
  add_compile_options(--std=c++11 -w -ffast-math -fpermissive -msse4)
else()
  add_definitions(-DWIN32 -D_WIN32 -D_WINDOWS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

# Add in the projects
add_subdirectory("thirdparty/boost/libs/python" "boost_python")
add_subdirectory("mathlib")
add_subdirectory("tier1")
add_subdirectory("game")
