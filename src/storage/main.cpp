#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include "naming.grpc.pb.h"
#include "storage_service_impl.h"

grpc::Status register_storage(const std::string &naming_address, const std::string &storage_address) {
    const auto stub = NamingService::NewStub(grpc::CreateChannel(naming_address, grpc::InsecureChannelCredentials()));
    RegisterStorageRequest req;
    RegisterStorageResponse res;
    grpc::ClientContext context;
    req.set_storage_address(storage_address);
    return stub->RegisterStorageServer(&context, req, &res);
}

int main(int argc, char **argv) {
    // enable support for curl or CLion .http requests
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    // Default port number
    std::string port = "7000";
    if (argc > 1) port = argv[1];
    const std::string storage_address = "localhost:" + port;

    // register storage server on naming
    grpc::Status status = register_storage("localhost:6000", storage_address);
    if (!status.ok()) {
        std::cerr << "Registration failed: " << status.error_message() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "Registered successfully!" << std::endl;

    StorageServiceImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(storage_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    const std::unique_ptr server(builder.BuildAndStart());
    std::cout << "Server listening on " << storage_address << std::endl;
    server->Wait();
}
