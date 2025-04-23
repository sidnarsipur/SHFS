#include "pch.h"
#include "naming_service_impl.h"

int main(int argc, char **argv) {
    std::string port = (argc > 1 ? argv[1] : "6000");
    std::string naming_address_ = "localhost:" + port;

    // enable support for curl or CLion .http requests
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    grpc::ServerBuilder builder;
    builder.AddListeningPort(naming_address_, grpc::InsecureServerCredentials());
    NamingServiceImpl service;
    builder.RegisterService(&service);

    const auto server = builder.BuildAndStart();
    spdlog::info("Naming Server Listening on {}", naming_address_);
    server->Wait();
    return 0;
}
