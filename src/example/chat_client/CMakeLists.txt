add_executable(example_chat_client main.cpp)

target_link_libraries(example_chat_client
    PRIVATE
        blackswan_share
)
