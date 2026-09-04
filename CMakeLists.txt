cmake_minimum_required(VERSION 3.20)

project(BlackSwan VERSION 0.1.0 LANGUAGES CXX)

option(BLACKSWAN_BUILD_EXAMPLES "Build BlackSwan example targets" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(BLACKSWAN_PROTOBUF_ROOT "${PROJECT_SOURCE_DIR}/thirdparty/protobuf-36.1/win-x64")
set(BLACKSWAN_PROTOBUF_RELEASE_LIB_DIR "${BLACKSWAN_PROTOBUF_ROOT}/lib/release")
set(BLACKSWAN_PROTOBUF_DEBUG_LIB_DIR "${BLACKSWAN_PROTOBUF_ROOT}/lib/debug")
if(NOT EXISTS "${BLACKSWAN_PROTOBUF_ROOT}/include/google/protobuf/stubs/common.h")
    message(FATAL_ERROR "Protobuf headers not found: ${BLACKSWAN_PROTOBUF_ROOT}/include")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
function(blackswan_set_output_directory target_name)
    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY
            "${PROJECT_SOURCE_DIR}/bin/${target_name}"
    )

    if(CMAKE_CONFIGURATION_TYPES)
        foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
            string(TOUPPER "${config}" config_upper)

            set_target_properties(${target_name} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY_${config_upper}
                    "${PROJECT_SOURCE_DIR}/bin/${target_name}"
            )
        endforeach()
    endif()
endfunction()

add_library(blackswan_build_options INTERFACE)
target_compile_features(blackswan_build_options INTERFACE cxx_std_20)
target_include_directories(blackswan_build_options
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src
        ${BLACKSWAN_PROTOBUF_ROOT}/include
)

if(MSVC)
    target_compile_options(blackswan_build_options INTERFACE /W4 /permissive-)
    target_compile_definitions(blackswan_build_options INTERFACE
        _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
    )
else()
    target_compile_options(blackswan_build_options INTERFACE -Wall -Wextra -Wpedantic)
endif()

add_library(blackswan_protobuf_lite STATIC IMPORTED GLOBAL)
file(GLOB BLACKSWAN_PROTOBUF_RELEASE_DEPS
    "${BLACKSWAN_PROTOBUF_RELEASE_LIB_DIR}/absl_*.lib"
    "${BLACKSWAN_PROTOBUF_RELEASE_LIB_DIR}/libutf8_validity.lib"
)
file(GLOB BLACKSWAN_PROTOBUF_DEBUG_DEPS
    "${BLACKSWAN_PROTOBUF_DEBUG_LIB_DIR}/absl_*.lib"
    "${BLACKSWAN_PROTOBUF_DEBUG_LIB_DIR}/libutf8_validity.lib"
)
set_target_properties(blackswan_protobuf_lite PROPERTIES
    IMPORTED_LOCATION_DEBUG "${BLACKSWAN_PROTOBUF_DEBUG_LIB_DIR}/libprotobuf-lited.lib"
    IMPORTED_LOCATION_RELEASE "${BLACKSWAN_PROTOBUF_RELEASE_LIB_DIR}/libprotobuf-lite.lib"
    IMPORTED_LOCATION_RELWITHDEBINFO "${BLACKSWAN_PROTOBUF_RELEASE_LIB_DIR}/libprotobuf-lite.lib"
    IMPORTED_LOCATION_MINSIZEREL "${BLACKSWAN_PROTOBUF_RELEASE_LIB_DIR}/libprotobuf-lite.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${BLACKSWAN_PROTOBUF_ROOT}/include"
)
target_link_libraries(blackswan_protobuf_lite INTERFACE
    "$<$<CONFIG:Debug>:${BLACKSWAN_PROTOBUF_DEBUG_DEPS}>"
    "$<$<NOT:$<CONFIG:Debug>>:${BLACKSWAN_PROTOBUF_RELEASE_DEPS}>"
)
target_compile_definitions(blackswan_protobuf_lite INTERFACE
    _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
)

set(BLACKSWAN_SPDLOG_ROOT "${PROJECT_SOURCE_DIR}/thirdparty/spdlog")
if(NOT EXISTS "${BLACKSWAN_SPDLOG_ROOT}/include/spdlog/spdlog.h")
    message(FATAL_ERROR "spdlog headers not found: ${BLACKSWAN_SPDLOG_ROOT}/include")
endif()

add_library(blackswan_spdlog INTERFACE)
set_target_properties(blackswan_spdlog PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${BLACKSWAN_SPDLOG_ROOT}/include"
)

set(BLACKSWAN_TOML_ROOT "${PROJECT_SOURCE_DIR}/thirdparty/toml")
if(NOT EXISTS "${BLACKSWAN_TOML_ROOT}/toml.hpp")
    message(FATAL_ERROR "toml headers not found: ${BLACKSWAN_TOML_ROOT}")
endif()

add_library(blackswan_toml INTERFACE)
set_target_properties(blackswan_toml PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${BLACKSWAN_TOML_ROOT}"
)
target_link_libraries(blackswan_build_options INTERFACE blackswan_toml)

add_subdirectory(src/share)

add_subdirectory(src/dbserver)
add_subdirectory(src/loginserver)
add_subdirectory(src/worldserver)
add_subdirectory(src/gateserver)
add_subdirectory(src/gameserver)
add_subdirectory(src/client)

if(BLACKSWAN_BUILD_EXAMPLES)
    #add_subdirectory(src/example/boost)
    #add_subdirectory(src/example/echo_server)
    #add_subdirectory(src/example/echo_client)
    add_subdirectory(src/example/chat_client)
    add_subdirectory(src/example/chat_server)
    #add_subdirectory(src/example/asio_server)
    #add_subdirectory(src/example/net_client)
    #add_subdirectory(src/example/net_server)
endif()
