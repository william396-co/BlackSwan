file(GLOB_RECURSE GAMESERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(gameserver ${GAMESERVER_SOURCES})

target_link_libraries(gameserver
    PRIVATE
        blackswan_network_ex
)
