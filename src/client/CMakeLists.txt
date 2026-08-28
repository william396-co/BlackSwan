file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(client ${CLIENT_SOURCES})

target_link_libraries(client
    PRIVATE
        blackswan_network_ex
)
