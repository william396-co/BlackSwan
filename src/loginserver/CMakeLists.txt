file(GLOB_RECURSE LOGINSERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(loginserver ${LOGINSERVER_SOURCES})

blackswan_set_output_directory(loginserver)

target_link_libraries(loginserver
    PRIVATE
        blackswan_share
        blackswan_protobuf_lite
)
