add_executable(naming main.cpp $<TARGET_OBJECTS:proto-objects>)

target_include_directories(naming PRIVATE ${PROJECT_SOURCE_DIR}/common ${PROTO_BINARY_DIR})

target_link_libraries(naming
        common                # Assuming you defined this in /common/CMakeLists.txt
        gRPC::grpc++
        protobuf::libprotobuf
)