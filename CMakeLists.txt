cmake_minimum_required(VERSION 3.20)

project(BlackSwan VERSION 0.1.0 LANGUAGES CXX)

option(BLACKSWAN_BUILD_EXAMPLES "Build BlackSwan example targets" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
if(CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${config} config_upper)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} ${PROJECT_SOURCE_DIR}/bin)
    endforeach()
endif()

add_library(blackswan_build_options INTERFACE)
target_compile_features(blackswan_build_options INTERFACE cxx_std_20)
target_include_directories(blackswan_build_options
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src
)

if(MSVC)
    target_compile_options(blackswan_build_options INTERFACE /W4 /permissive-)
else()
    target_compile_options(blackswan_build_options INTERFACE -Wall -Wextra -Wpedantic)
endif()

add_subdirectory(src/worldserver)
add_subdirectory(src/gateserver)
add_subdirectory(src/gameserver)

if(BLACKSWAN_BUILD_EXAMPLES)
    add_subdirectory(src/example/boost)
    add_subdirectory(src/example/echo_server)
    add_subdirectory(src/example/echo_client)
    add_subdirectory(src/example/chat_server)
endif()
