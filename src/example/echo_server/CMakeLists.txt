find_package(Boost 1.70 QUIET CONFIG COMPONENTS system)
if(NOT Boost_FOUND)
    set(Boost_NO_WARN_NEW_VERSIONS ON)

    if(POLICY CMP0167)
        cmake_policy(SET CMP0167 OLD)
    endif()

    find_package(Boost REQUIRED COMPONENTS system)
endif()

find_package(Threads REQUIRED)

add_executable(example_echo_server 
            server.cpp
            main.cpp
    )

target_link_libraries(example_echo_server
    PRIVATE
        blackswan_build_options
        Boost::system
        Threads::Threads
)

if(TARGET Boost::headers)
    target_link_libraries(example_echo_server PRIVATE Boost::headers)
elseif(TARGET Boost::boost)
    target_link_libraries(example_echo_server PRIVATE Boost::boost)
else()
    target_include_directories(example_echo_server PRIVATE ${Boost_INCLUDE_DIRS})
endif()

if(WIN32)
    target_compile_definitions(example_echo_server PRIVATE
        _WIN32_WINNT=0x0A00
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )

    target_link_libraries(example_echo_server PRIVATE ws2_32 mswsock)
endif()
