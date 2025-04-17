## How to generate proto files automatically using CMake

1. Everytime you create a new .proto file, add it to the end of the `PROTOS` argument in `asio_grpc_protobuf_generate`:
    ```CMake
        asio_grpc_protobuf_generate(
            GENERATE_GRPC GENERATE_MOCK_CODE
            TARGET proto-objects
            USAGE_REQUIREMENT PUBLIC
            IMPORT_DIRS ${PROTO_IMPORT_DIRS}
            OUT_DIR "${PROTO_BINARY_DIR}"
            PROTOS
                "${CMAKE_CURRENT_LIST_DIR}/proto/hello.proto"
                "${CMAKE_CURRENT_LIST_DIR}/proto/naming.proto"
        )
    ```

2. Rebuild the `proto-objects` library in CLion