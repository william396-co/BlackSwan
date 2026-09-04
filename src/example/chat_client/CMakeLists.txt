add_executable(example_chat_client main.cpp)
blackswan_set_output_directory(example_chat_client)

target_link_libraries(example_chat_client
    PRIVATE
        blackswan_share
)
