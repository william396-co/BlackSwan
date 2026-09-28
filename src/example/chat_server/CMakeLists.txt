add_executable(example_chat_server main.cpp)
blackswan_set_output_directory(example_chat_server)

target_link_libraries(example_chat_server
    PRIVATE
        blackswan_share
)
