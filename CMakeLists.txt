cmake_minimum_required(VERSION 3.26)
project(DFS CXX)
set(CMAKE_CXX_STANDARD 20)

# Find necessary packages
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(asio-grpc CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# === Generate Protobuf and gRPC files ===
set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(PROTO_IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}/proto")

# Proto objects are created by this library
add_library(proto-objects STATIC)

# Link necessary libraries for proto-objects
target_link_libraries(proto-objects PUBLIC protobuf::libprotobuf gRPC::grpc++_unsecure)

# Generate the Protobuf and gRPC files
asio_grpc_protobuf_generate(
        GENERATE_GRPC GENERATE_MOCK_CODE
        TARGET proto-objects
        USAGE_REQUIREMENT PUBLIC
        IMPORT_DIRS ${PROTO_IMPORT_DIRS}
        OUT_DIR "${PROTO_BINARY_DIR}"
        PROTOS
            "${CMAKE_CURRENT_LIST_DIR}/proto/naming.proto"
            "${CMAKE_CURRENT_LIST_DIR}/proto/storage.proto"
)

add_subdirectory(src/common)
add_subdirectory(src/naming)
add_subdirectory(src/storage)
add_subdirectory(src/client)
