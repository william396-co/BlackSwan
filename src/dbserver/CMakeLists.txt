file(GLOB_RECURSE DBSERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(dbserver ${DBSERVER_SOURCES})
blackswan_set_output_directory(dbserver)

target_link_libraries(dbserver
    PRIVATE
        blackswan_share
        blackswan_protobuf_lite
)
