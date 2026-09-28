find_package(Boost 1.70 QUIET CONFIG COMPONENTS system)
if(NOT Boost_FOUND)
    set(Boost_NO_WARN_NEW_VERSIONS ON)

    if(POLICY CMP0167)
        cmake_policy(SET CMP0167 OLD)
    endif()

    find_package(Boost REQUIRED COMPONENTS system)
endif()

find_package(Threads REQUIRED)

file(GLOB PROTOCOL_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/share/proto/*.pb.cc"
    "${PROJECT_SOURCE_DIR}/src/share/proto/*.cpp"
)

file(GLOB LOG_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/share/log/*.cpp")

file(GLOB NET_WORK_EX_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/share/networkEx/*.cpp")

 file(GLOB UTILS_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/share/utils/*.cpp")

add_library(blackswan_share STATIC
    ${LOG_SOURCES}
    ${NET_WORK_EX_SOURCES}
    ${UTILS_SOURCES}
    ${PROTOCOL_SOURCES}
)

target_include_directories(blackswan_share
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/share
)

target_link_libraries(blackswan_share
    PUBLIC
        blackswan_build_options
        blackswan_spdlog
        Boost::system
        Threads::Threads
)

if(TARGET Boost::headers)
    target_link_libraries(blackswan_share PUBLIC Boost::headers)
elseif(TARGET Boost::boost)
    target_link_libraries(blackswan_share PUBLIC Boost::boost)
else()
    target_include_directories(blackswan_share PUBLIC ${Boost_INCLUDE_DIRS})
endif()

if(WIN32)
    target_compile_definitions(blackswan_share PUBLIC
        _WIN32_WINNT=0x0A00
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )

    target_link_libraries(blackswan_share PUBLIC ws2_32 mswsock)
endif()
