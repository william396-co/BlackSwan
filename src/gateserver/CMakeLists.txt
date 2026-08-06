add_executable(gateserver main.cpp)

target_link_libraries(gateserver
    PRIVATE
        blackswan_build_options
)
