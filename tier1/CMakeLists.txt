project(tier1)

set(TIER1_SOURCES
  bitbuf.cpp
  byteswap.cpp
  characterset.cpp
  checksum_crc.cpp
  checksum_md5.cpp
  commandbuffer.cpp
  convar.cpp
  datamanager.cpp
  diff.cpp
  generichash.cpp
  interface.cpp
  KeyValues.cpp
  mempool.cpp
  memstack.cpp
  NetAdr.cpp
  newbitbuf.cpp
  processor_detect.cpp
  rangecheckedvar.cpp
  stringpool.cpp
  strtools.cpp
  tier1.cpp
  tokenreader.cpp
  uniqueid.cpp
  utlbuffer.cpp
  utlbufferutil.cpp
  utlstring.cpp
  utlsymbol.cpp
)

add_library(tier1 STATIC ${TIER1_SOURCES})

target_compile_definitions(tier1 PRIVATE _LIB TIER1_STATIC_LIB)

target_include_directories(tier1 PRIVATE
  "${CMAKE_SOURCE_DIR}/common"
  "${CMAKE_SOURCE_DIR}/public"
  "${CMAKE_SOURCE_DIR}/public/tier0"
  "${CMAKE_SOURCE_DIR}/public/tier1"
)
