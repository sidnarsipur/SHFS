#include <grpcpp/grpcpp.h>
#include "naming.grpc.pb.h"
#include <iostream>
#include <memory>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

class NamingServiceImpl final : public NamingService::Service {
    std::unordered_set<std::string> storage_addresses;
public:
    grpc::Status RegisterStorage(grpc::ServerContext* context, const RegisterRequest* request, RegisterResponse* response) override {
        // Implement the logic for RegisterStorage hereew1
        const std::string& storage_address = request->storage_address();

        // Store it in the hashmap (keyed by storage address for simplicity)
        storage_addresses.insert(storage_address);
        std::cout << "Registering storage with address: " << storage_address << std::endl;

        // Set the response (assuming success)
        response->set_success(true);

        // Return the response status
        return grpc::Status::OK;
    }

    grpc::Status LookupFile(grpc::ServerContext* context, const LookupRequest* request, LookupResponse* response) override {
        // std::string filename = request->filename();
        for (const auto& address : storage_addresses) {
            response->add_storage_address(address); // add each unique address from the set
        }
        return grpc::Status::OK;
    }
};

int main() {
    // enable support for curl or CLion .http requests
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    NamingServiceImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on 0.0.0.0:50051\n";
    server->Wait();
}
