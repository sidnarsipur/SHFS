add_executable(client main.cpp)
target_include_directories(client PRIVATE ${PROJECT_SOURCE_DIR}/common ${PROTO_BINARY_DIR})

target_link_libraries(client PRIVATE
        common                # Assuming you defined this in /common/CMakeLists.txt
        proto-objects
)