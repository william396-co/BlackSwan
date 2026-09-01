file(GLOB_RECURSE DBSERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(dbserver ${DBSERVER_SOURCES})

target_link_libraries(dbserver
    PRIVATE
        blackswan_share
)
