#
# SDK Makefile for x86 Linux
#
#

#############################################################################
# Developer configurable items
#############################################################################

# the name of the mod binary (_i486.so is appended to the end)
NAME=server

# is this a debug build?
DODEBUG=false

# the location of the vcproj that builds the mod
MOD_PROJ=../game/server_ges-2010.vcxproj
# the name of the mod configuration (typically <proj name>_<build type><build target>)
ifeq ($(DODEBUG),true)
	MOD_CONFIG=Server_DebugWin32
else
	MOD_CONFIG=Server_ReleaseWin32
endif

# the directory the base binaries (tier0_i486.so, etc) are located
# this should point to your orange box subfolder of where you have srcds installed.
GAME_DIR=/home/jon/.steam/steam/SteamApps/common/sdk2007

# the path to your mods directory
# set this so that 'make install' or 'make installrelease' will copy your binary over automatically.
MOD_DIR=/home/jon/sourcemods/gesource_release

# compiler options (gcc 3.4.1 or above is required - 4.1.2+ recommended)
CPLUS=g++-4.7
CLINK=gcc-4.7
CC=$(CLINK)
CPP_LIB=/usr/lib/i386-linux-gnu/libstdc++.so.6

# GCC optimization flags
#OPTFLAGS=-O1 -ffast-math
OPTFLAGS=-ffast-math

# put any compiler flags you want passed here
USER_CFLAGS=

# Link Libraries
LIBFILES= \
  $(SOURCE_DIR)/lib/linux/choreoobjects_486.a \
  $(SOURCE_DIR)/lib/linux/particles_486.a \
  $(SOURCE_DIR)/lib/linux/dmxloader_486.a \
  $(SOURCE_DIR)/lib/linux/tier3_486.a \
  $(SOURCE_DIR)/lib/linux/tier2_486.a \
  $(SOURCE_DIR)/lib/linux/tier1_486.a \
  $(SOURCE_DIR)/lib/linux/mathlib_486.a \
  $(SOURCE_DIR)/lib/curl/libcurl.a \
  $(SOURCE_DIR)/lib/python/libpython2.6.a \
  $(SOURCE_DIR)/lib/python/libboost_python.a \
  steam_api_i486.so \
  tier0_i486.so \
  vstdlib_i486.so

# link flags for your mod, make sure to include any special libraries here
LDFLAGS="$(LIBFILES) -lm -ldl -lz -lssl -lldap -lrt -lutil"

#############################################################################
# Things below here shouldn't need to be altered
#############################################################################
MAKE=make

# the dir we want to put binaries we build into
BUILD_DIR=.

ifeq ($(DODEBUG),true)
	BUILD_OBJ_DIR=$(BUILD_DIR)/debug/$(CLINK)
else
	BUILD_OBJ_DIR=$(BUILD_DIR)/release/$(CLINK)
endif

# the location of the source code
SOURCE_DIR=..

# the CPU target for the build, must be i486 for now
ARCH=i486
#ARCH_CFLAGS=-mtune=i686 -march=pentium -mmmx -msse -pipe
ARCH_CFLAGS=-msse

# Flags for gcc/g++
BASE_CFLAGS=-DVPROF_LEVEL=1 -DSWDS -D_LINUX -DLINUX -fpermissive -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp

ifneq ($(DODEBUG),true)
	BASE_CFLAGS+=-DNDEBUG
endif

SHLIBEXT=so

SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared -Wl,-Map,$@_map.txt -Wl

#flags passed to the compiler
CFLAGS="$(USER_CFLAGS) $(DEFINES) $(ARCH_CFLAGS) $(OPTFLAGS) $(BASE_CFLAGS) -Usprintf=use_Q_snprintf_instead_of_sprintf -Ustrncpy=use_Q_strncpy_instead -Ufopen=dont_use_fopen -UPROTECTED_THINGS_ENABLE"

# define list passed to make for the sub makefile
BASE_DEFINES=CC=$(CC) CPLUS=$(CPLUS) CPP_LIB=$(CPP_LIB) \
	BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) \
	SOURCE_DIR=$(SOURCE_DIR) SHLIBLDFLAGS=$(SHLIBLDFLAGS) SHLIBEXT=$(SHLIBEXT) \
	CLINK=$(CLINK) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) \
	ARCH=$(ARCH) GAME_DIR=$(GAME_DIR) MOD_CONFIG=$(MOD_CONFIG) NAME=$(NAME) \
	DODEBUG=$(DODEBUG)

# Project Makefile
MAKE_MOD=Makefile.server
MAKE_VCPM=Makefile.vcpm
MAKE_PLUGIN=Makefile.plugin

ll: check vcpm server

check:
	if [ -z "$(CC)" ]; then echo "Compiler not defined."; exit; fi
	if [ ! -d $(BUILD_DIR) ];then mkdir $(BUILD_DIR);fi
	cd $(BUILD_DIR)

vcpm:
	$(MAKE) -f $(MAKE_VCPM) $(BASE_DEFINES)

server: vcpm
	if [ ! -f "tier0_i486.so" ]; then ln -s $(GAME_DIR)/bin/tier0_i486.so .; fi
	if [ ! -f "vstdlib_i486.so" ]; then ln -s $(GAME_DIR)/bin/vstdlib_i486.so .; fi
	if [ ! -f "steam_api_i486.so" ]; then ln -s $(GAME_DIR)/bin/steam_api_i486.so .; fi

	./vcpm $(MOD_PROJ)
	$(MAKE) -f $(MAKE_MOD) $(BASE_DEFINES)

plugin:
	$(MAKE) -f $(MAKE_PLUGIN) $(BASE_DEFINES)

clean:
	 $(MAKE) -f $(MAKE_VCPM) $(BASE_DEFINES) clean
	 $(MAKE) -f $(MAKE_PLUGIN) $(BASE_DEFINES) clean
	 $(MAKE) -f $(MAKE_MOD) $(BASE_DEFINES) clean

install:
	cp -f $(NAME)_$(ARCH).so $(MOD_DIR)/bin/$(NAME)_$(ARCH).so

installrelease:
	cp -f $(NAME)_$(ARCH).so $(MOD_DIR)/bin/$(NAME)_$(ARCH).so
	strip $(MOD_DIR)/bin/$(NAME)_$(ARCH).so
