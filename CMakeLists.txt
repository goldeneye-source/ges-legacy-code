project(gesource)

cmake_minimum_required(VERSION 2.8.12)

set(GESOURCE_VERSION "4.3.0")

add_definitions(-DVPROF_LEVEL=1 -DSWDS -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp 
  -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -Dstrcmpi=strcasecmp)

if (UNIX)
add_definitions(-D_LINUX -DLINUX -D_alloca=alloca)
add_compile_options(--std=c++0x -w -ffast-math -fpermissive -msse)
endif()

add_compile_options(-Usprintf=use_Q_snprintf_instead_of_sprintf -Ustrncpy=use_Q_strncpy_instead 
  -Ufopen=dont_use_fopen -UPROTECTED_THINGS_ENABLE)

add_subdirectory(game)
