add_executable(client main.cpp)

target_link_libraries(client
    PRIVATE
        blackswan_network_ex
)
