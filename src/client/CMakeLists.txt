# Separate browser build. Do not point the native mu97k CMake at this file.
# The native client stays on MSVC Win32. Configure this tree only with emcmake.
cmake_minimum_required(VERSION 3.25)
project(mu97k_browser LANGUAGES C CXX)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR
        "This build is Emscripten-only. From a shell where emsdk_env.bat has run: "
        "emcmake cmake -S emscripten_port -B emscripten_port/build -G Ninja")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src")
    set(MU97K_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../reference/Mu-97k-Client-Source")
    set(MU97K_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../reference/Mu-97k-Client-Source")
else()
    set(MU97K_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
endif()


set(JPEG_SOURCES
    jpeg-6b/jcapimin.c
    jpeg-6b/jcapistd.c
    jpeg-6b/jccoefct.c
    jpeg-6b/jccolor.c
    jpeg-6b/jcdctmgr.c
    jpeg-6b/jchuff.c
    jpeg-6b/jcinit.c
    jpeg-6b/jcmainct.c
    jpeg-6b/jcmarker.c
    jpeg-6b/jcmaster.c
    jpeg-6b/jcomapi.c
    jpeg-6b/jcparam.c
    jpeg-6b/jcphuff.c
    jpeg-6b/jcprepct.c
    jpeg-6b/jcsample.c
    jpeg-6b/jctrans.c
    jpeg-6b/jdapimin.c
    jpeg-6b/jdapistd.c
    jpeg-6b/jdatadst.c
    jpeg-6b/jdatasrc.c
    jpeg-6b/jdcoefct.c
    jpeg-6b/jdcolor.c
    jpeg-6b/jddctmgr.c
    jpeg-6b/jdhuff.c
    jpeg-6b/jdinput.c
    jpeg-6b/jdmainct.c
    jpeg-6b/jdmarker.c
    jpeg-6b/jdmaster.c
    jpeg-6b/jdmerge.c
    jpeg-6b/jdphuff.c
    jpeg-6b/jdpostct.c
    jpeg-6b/jdsample.c
    jpeg-6b/jdtrans.c
    jpeg-6b/jerror.c
    jpeg-6b/jfdctflt.c
    jpeg-6b/jfdctfst.c
    jpeg-6b/jfdctint.c
    jpeg-6b/jidctflt.c
    jpeg-6b/jidctfst.c
    jpeg-6b/jidctint.c
    jpeg-6b/jidctred.c
    jpeg-6b/jquant1.c
    jpeg-6b/jquant2.c
    jpeg-6b/jutils.c
    jpeg-6b/jmemmgr.c
    jpeg-6b/jmemansi.c
)
list(TRANSFORM JPEG_SOURCES PREPEND "${MU97K_ROOT}/lib/libjpeg/")

add_library(jpeg STATIC ${JPEG_SOURCES})
target_include_directories(jpeg PUBLIC "${MU97K_ROOT}/lib/libjpeg/jpeg-6b")
target_compile_definitions(jpeg PRIVATE HAVE_PROTOTYPES)

add_executable(jpeg_roundtrip browser/jpeg_roundtrip.cpp)
target_link_libraries(jpeg_roundtrip PRIVATE jpeg)
target_link_options(jpeg_roundtrip PRIVATE "-sENVIRONMENT=node")

add_executable(protocol_check browser/protocol_check.cpp)
target_include_directories(protocol_check PRIVATE "${MU97K_ROOT}/src")
target_link_options(protocol_check PRIVATE "-sENVIRONMENT=node")

add_executable(abi_probe browser/abi_probe.cpp)
target_link_options(abi_probe PRIVATE "-sENVIRONMENT=node")

add_executable(webgl2_smoke browser/webgl2_smoke.cpp)
target_link_options(webgl2_smoke PRIVATE
    "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/browser/shell.html"
    "-sMIN_WEBGL_VERSION=2"
    "-sMAX_WEBGL_VERSION=2"
    "-sENVIRONMENT=web"
)
set_target_properties(webgl2_smoke PROPERTIES SUFFIX ".html")
