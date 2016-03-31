project(gesource)

cmake_minimum_required(VERSION 2.8.12)

set(GESOURCE_VERSION "4.3.0")

add_definitions(-DVPROF_LEVEL=1 -DSWDS -D_LINUX -DLINUX -Dstricmp=strcasecmp -D_stricmp=strcasecmp
  -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca
  -Dstrcmpi=strcasecmp)

add_compile_options(--std=c++0x -w -ffast-math -fpermissive -msse -Usprintf=use_Q_snprintf_instead_of_sprintf
  -Ustrncpy=use_Q_strncpy_instead -Ufopen=dont_use_fopen -UPROTECTED_THINGS_ENABLE)

add_subdirectory(game)
