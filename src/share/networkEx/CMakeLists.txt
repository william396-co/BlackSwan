find_package(Boost 1.70 QUIET CONFIG COMPONENTS system)
if(NOT Boost_FOUND)
    set(Boost_NO_WARN_NEW_VERSIONS ON)

    if(POLICY CMP0167)
        cmake_policy(SET CMP0167 OLD)
    endif()

    find_package(Boost REQUIRED COMPONENTS system)
endif()

find_package(Threads REQUIRED)

add_library(blackswan_network_ex STATIC)
target_sources(blackswan_network_ex
    PRIVATE
        packet.cpp
)

target_link_libraries(blackswan_network_ex
    PUBLIC
        blackswan_build_options
        Boost::system
        Threads::Threads
)

target_include_directories(blackswan_network_ex
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/share
)

if(TARGET Boost::headers)
    target_link_libraries(blackswan_network_ex PUBLIC Boost::headers)
elseif(TARGET Boost::boost)
    target_link_libraries(blackswan_network_ex PUBLIC Boost::boost)
else()
    target_include_directories(blackswan_network_ex PUBLIC ${Boost_INCLUDE_DIRS})
endif()

if(WIN32)
    target_compile_definitions(blackswan_network_ex PUBLIC
        _WIN32_WINNT=0x0A00
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )

    target_link_libraries(blackswan_network_ex PUBLIC ws2_32 mswsock)
endif()
