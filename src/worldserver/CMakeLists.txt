file(GLOB_RECURSE WORLDSERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(worldserver ${WORLDSERVER_SOURCES})
blackswan_set_output_directory(worldserver)

target_link_libraries(worldserver
    PRIVATE
        blackswan_share
)
