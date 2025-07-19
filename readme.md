# DFS

A fault tolerant distributed file system with heartbeat detection, automatic failure recovery, and dynamic file rebalancing base on server load.

## Prerequisites

Before building the project, make sure the following tools and libraries are installed:

* **CMake** (version 3.26 or above)
* **vcpkg** (for managing dependencies)
* **A C++20-compatible compiler**

### Installing Dependencies with vcpkg

1. **Install vcpkg** (if not already installed)

   Clone the vcpkg repository and run the installation script:

   ```bash
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.sh
   ```

2. **Install Dependencies Using vcpkg**

   Once vcpkg is installed, use it to install the required libraries:

   ```bash
   ./vcpkg install grpc protobuf asio-grpc spdlog
   ```

   This will install the following dependencies:

   * `grpc` (gRPC library)
   * `protobuf` (Protocol Buffers)
   * `asio-grpc` (Asynchronous gRPC)
   * `spdlog` (Logging library)

   Ensure that the installed libraries are compatible with your environment.

## Building the Project

### Step 1: Create a Build Directory

```bash
mkdir build
```

### Step 2: Configure the Project

Use `cmake` to save a configuration for the project, specifying the vcpkg toolchain:

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Step 3: Build the Project

After configuring the project, build it using:

```bash
cmake --build build
```

This will compile the source code and generate the necessary executables in the appropriate directories.

### Step 5: Running the executables

Run like so:

```bash
./build/src/client/client.exe
```

## Additional Information

### Precompiled headers

All library includes should be placed in `pch.h` to get the benefit of precompiled header speed up.

### Testing

To run tests, you can use an external Python script or manually invoke specific tests once you’ve built the necessary components. Ensure your testing environment is set up properly according to your configuration.

### How to generate proto files automatically using CMake

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
