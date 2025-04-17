add_executable(storage main.cpp $<TARGET_OBJECTS:proto-objects>)

target_include_directories(storage PRIVATE ${PROJECT_SOURCE_DIR}/common ${PROTO_BINARY_DIR})

target_link_libraries(storage
        common                # Assuming you defined this in /common/CMakeLists.txt
        gRPC::grpc++
        gRPC::grpc++_reflection
        protobuf::libprotobuf
)