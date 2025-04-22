#include "pch.h"
#include "naming_service_impl.h"

int main(int argc, char **argv) {
    // enable support for curl or CLion .http requests
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    // Default port number
    std::string port = "6000";
    if (argc > 1) port = argv[1];
    const std::string naming_address = "localhost:" + port;

    NamingServiceImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(naming_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    const std::unique_ptr server(builder.BuildAndStart());
    spdlog::info("Server lffistening ofn {}", naming_address);
    server->Wait();
}
