cmake_minimum_required(VERSION 3.22)

project(Lab2QRCodeWASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Override default CMake optimization levels for size
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-Oz -flto -g0 -DNDEBUG -fvisibility=hidden -fvisibility-inlines-hidden")
    set(CMAKE_C_FLAGS_RELEASE "-Oz -flto -g0 -DNDEBUG -fvisibility=hidden")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Oz -flto -g0")
endif()

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "This project must be built with Emscripten. Use: emcmake cmake")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

find_package(Threads REQUIRED)
find_package(ZXing REQUIRED)

set(WASM_SOURCES
    src/barcode_api_simple.cpp
)

add_executable(${PROJECT_NAME} ${WASM_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    ZXing::ZXing
    Threads::Threads
)

set(EMSCRIPTEN_LINK_FLAGS
    -lembind
    -sEXPORT_NAME=createLab2QRCodeModule
    -sMODULARIZE=1
    -sALLOW_MEMORY_GROWTH=1
    -sMAXIMUM_MEMORY=2GB
    -sENVIRONMENT=web
    -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString']
    -sASSERTIONS=0
    -sEXCEPTION_CATCHING_ALLOWED=[]
    -sINVOKE_RUN=0
    -sEMBIND_STD_STRING_IS_UTF8=1
    --bind
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND EMSCRIPTEN_LINK_FLAGS
        --closure 1
        -sMALLOC=emmalloc
        -sFILESYSTEM=0
        -sWASM_BIGINT=0
        -sINITIAL_MEMORY=16777216
        -sERROR_ON_UNDEFINED_SYMBOLS=0
        -sEXPORT_ALL=0
    )
else()
    list(APPEND EMSCRIPTEN_LINK_FLAGS
        -O0
        -g
        -sASSERTIONS=2
    )
endif()

target_link_options(${PROJECT_NAME} PRIVATE ${EMSCRIPTEN_LINK_FLAGS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "lab2qrcode"
    SUFFIX ".js"
)

message(STATUS "===========================================")
message(STATUS "Lab2QRCode WASM Module")
message(STATUS "===========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output: lab2qrcode.js + lab2qrcode.wasm")
message(STATUS "===========================================")
