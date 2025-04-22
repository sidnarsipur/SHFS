#include <grpcpp/grpcpp.h>
#include "naming.grpc.pb.h"
#include <iostream>
#include <memory>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
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
    const std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << naming_address << std::endl;
    server->Wait();
}
