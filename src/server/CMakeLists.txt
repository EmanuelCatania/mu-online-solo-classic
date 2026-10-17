cmake_minimum_required(VERSION 3.16)
project(MuSoloServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Signed char compatibility for MU 0.97k
add_compile_options(-fsigned-char)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Common
    ${CMAKE_CURRENT_SOURCE_DIR}/GameServer
    ${CMAKE_CURRENT_SOURCE_DIR}/Solo
)

file(GLOB_RECURSE GAMESERVER_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/GameServer/*.cpp")
file(GLOB_RECURSE SOLO_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/Solo/*.cpp")

set(ALL_SRCS ${GAMESERVER_SRCS} ${SOLO_SRCS})

add_executable(muserver ${ALL_SRCS})

if (EMSCRIPTEN)
    set_target_properties(muserver PROPERTIES
        OUTPUT_NAME "muserver"
        SUFFIX ".js"
    )

    set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Data")

    target_link_options(muserver PRIVATE
        "-O2"
        "-sWASM=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sINITIAL_MEMORY=64MB"
        "-sMAXIMUM_MEMORY=512MB"
        "-sNO_EXIT_RUNTIME=1"
        "-sMODULARIZE=1"
        "-sEXPORT_NAME=createMuServer"
        "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','FS','addFunction','removeFunction','setValue','getValue','UTF8ToString','stringToUTF8','HEAPU8','HEAP8','HEAP16','HEAP32','HEAPU32']"
        "-sEXPORTED_FUNCTIONS=['_malloc','_free','_SoloServer_Init','_SoloServer_Tick','_SoloServer_ConnectPlayer','_SoloServer_DisconnectPlayer','_SoloServer_ReceiveClientPacket','_SoloServer_SetSendCallback','_SoloServer_SetSaveCallback','_SoloServer_GetPlayerCount','_SoloServer_GetMonsterCount']"
        "-sALLOW_TABLE_GROWTH=1"
        "--preload-file" "${DATA_DIR}@/Data"
    )
endif()
