#pragma once
#include "naming_service_impl.h"

class NamingNode {
public:
    explicit NamingNode(std::string naming_address)
        : naming_address_(std::move(naming_address)) {}

    void Run() {
        // enable support for curl or CLion .http requests
        grpc::EnableDefaultHealthCheckService(true);
        grpc::reflection::InitProtoReflectionServerBuilderPlugin();

        grpc::ServerBuilder builder;
        builder.AddListeningPort(naming_address_, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);

        const auto server = builder.BuildAndStart();
        spdlog::info("Naming server listening on {}", naming_address_);
        server->Wait();
    }

private:
    const std::string naming_address_;
    NamingServiceImpl service_;
};