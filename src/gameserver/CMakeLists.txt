add_executable(gameserver main.cpp)

target_link_libraries(gameserver
    PRIVATE
        blackswan_build_options
)
