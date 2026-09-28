file(GLOB_RECURSE GAMESERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(gameserver ${GAMESERVER_SOURCES})
blackswan_set_output_directory(gameserver)

target_link_libraries(gameserver
    PRIVATE
        blackswan_share
        blackswan_protobuf_lite
)
