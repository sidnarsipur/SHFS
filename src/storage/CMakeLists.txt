add_executable(storage main.cpp)
target_include_directories(storage PRIVATE ${PROJECT_SOURCE_DIR}/common ${PROTO_BINARY_DIR})
target_precompile_headers(storage PRIVATE pch.h)

target_link_libraries(storage
    PRIVATE
        common                # Assuming you defined this in /common/CMakeLists.txt
        proto-objects
        gRPC::grpc++_reflection
        spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>
)