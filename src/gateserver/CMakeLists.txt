file(GLOB_RECURSE GATESERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(gateserver ${GATESERVER_SOURCES})

target_link_libraries(gateserver
    PRIVATE
        blackswan_network_ex
)
