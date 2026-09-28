file(GLOB_RECURSE GATESERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(gateserver ${GATESERVER_SOURCES})
blackswan_set_output_directory(gateserver)

target_link_libraries(gateserver
    PRIVATE
        blackswan_share
        blackswan_protobuf_lite
)
