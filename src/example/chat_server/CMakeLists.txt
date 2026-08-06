add_executable(example_chat_server main.cpp)

target_link_libraries(example_chat_server
    PRIVATE
        blackswan_network_ex
)
