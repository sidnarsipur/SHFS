cmake_minimum_required(VERSION 3.28)
project(DFS)

set(CMAKE_CXX_STANDARD 20)

# Find necessary packages
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(asio-grpc CONFIG REQUIRED)

# === Generate Protobuf and gRPC files ===
set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(PROTO_IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}/proto")

# Proto objects are created by this library
add_library(proto-objects OBJECT)

# Link necessary libraries for proto-objects
target_link_libraries(proto-objects PUBLIC protobuf::libprotobuf gRPC::grpc++_unsecure)

# Generate the Protobuf and gRPC files
asio_grpc_protobuf_generate(
        GENERATE_GRPC GENERATE_MOCK_CODE
        TARGET proto-objects
        USAGE_REQUIREMENT PUBLIC
        IMPORT_DIRS ${PROTO_IMPORT_DIRS}
        OUT_DIR "${PROTO_BINARY_DIR}"
        PROTOS "${CMAKE_CURRENT_LIST_DIR}/proto/hello.proto"
)

# === Client & Server executables ===
add_executable(server src/server.cpp $<TARGET_OBJECTS:proto-objects>)
add_executable(client src/client.cpp $<TARGET_OBJECTS:proto-objects>)

# Include directories for generated files
target_include_directories(server PRIVATE ${PROTO_BINARY_DIR})
target_include_directories(client PRIVATE ${PROTO_BINARY_DIR})

# Link necessary libraries for the executables
target_link_libraries(server PRIVATE gRPC::grpc++ protobuf::libprotobuf)
target_link_libraries(client PRIVATE gRPC::grpc++ protobuf::libprotobuf)

