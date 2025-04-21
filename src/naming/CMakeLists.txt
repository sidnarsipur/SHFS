add_executable(naming main.cpp)
target_include_directories(naming PRIVATE ${PROJECT_SOURCE_DIR}/common ${PROTO_BINARY_DIR})

target_link_libraries(naming
    PRIVATE
        common                # Assuming you defined this in /common/CMakeLists.txt
        proto-objects
        gRPC::grpc++_reflection
)