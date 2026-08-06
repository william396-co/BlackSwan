find_package(Boost 1.70 QUIET CONFIG COMPONENTS system)
if(NOT Boost_FOUND)
    set(Boost_NO_WARN_NEW_VERSIONS ON)

    if(POLICY CMP0167)
        cmake_policy(SET CMP0167 OLD)
    endif()

    find_package(Boost REQUIRED COMPONENTS system)
endif()

find_package(Threads REQUIRED)

add_executable(example_boost
    async_accept.cpp
    io_context.cpp
    main.cpp
    post_vs_dispatch.cpp
    steady_timer.cpp
    work_guard.cpp
    period_timer.cpp
)

target_link_libraries(example_boost
    PRIVATE
        blackswan_build_options
        Boost::system
        Threads::Threads
)

if(TARGET Boost::headers)
    target_link_libraries(example_boost PRIVATE Boost::headers)
elseif(TARGET Boost::boost)
    target_link_libraries(example_boost PRIVATE Boost::boost)
else()
    target_include_directories(example_boost PRIVATE ${Boost_INCLUDE_DIRS})
endif()

if(WIN32)
    target_compile_definitions(example_boost PRIVATE
        _WIN32_WINNT=0x0A00
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )

    target_link_libraries(example_boost PRIVATE ws2_32 mswsock)
endif()
