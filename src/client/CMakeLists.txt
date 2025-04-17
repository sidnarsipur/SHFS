add_executable(client main.cpp $<TARGET_OBJECTS:proto-objects>)

target_include_directories(client PRIVATE ${PROJECT_SOURCE_DIR}/common ${PROTO_BINARY_DIR})

target_link_libraries(client
        common                # Assuming you defined this in /common/CMakeLists.txt
        gRPC::grpc++
        protobuf::libprotobuf
)