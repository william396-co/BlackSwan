add_executable(worldserver main.cpp)

target_link_libraries(worldserver
    PRIVATE
        blackswan_build_options
)
