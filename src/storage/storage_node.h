#pragma once
#include "storage_service_impl.h"

class StorageNode {
public:
    StorageNode(const std::string &naming_address,
                             const std::string &storage_address)
        : naming_address_(naming_address),
          storage_address_(storage_address),
          naming_stub_(NamingService::NewStub(
              grpc::CreateChannel(naming_address_, grpc::InsecureChannelCredentials()))) {}

    void Register() const {
        RegisterStorageRequest req;
        RegisterStorageResponse res;
        grpc::ClientContext ctx;

        req.set_storage_address(storage_address_);
        auto status = naming_stub_->RegisterStorageServer(&ctx, req, &res);

        if (!status.ok()) {
            spdlog::error("Registration failed: {}", status.error_message());
            std::exit(EXIT_FAILURE);
        }
        spdlog::info("Registered storage server at {}", storage_address_);
    }

    void SendHeartbeat() const {
        HeartbeatRequest req;
        HeartbeatResponse res;
        grpc::ClientContext ctx;

        req.set_address(storage_address_);
        auto status = naming_stub_->Heartbeat(&ctx, req, &res);

        if (status.ok()) {
            spdlog::info("Heartbeat ack: {}", res.error_message());
        } else {
            spdlog::warn("Heartbeat failed: {}", status.error_message());
        }
    }

    void Run() {
        // enable support for curl or CLion .http requests
        grpc::EnableDefaultHealthCheckService(true);
        grpc::reflection::InitProtoReflectionServerBuilderPlugin();

        grpc::ServerBuilder builder;
        builder.AddListeningPort(storage_address_, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);

        auto server = builder.BuildAndStart();
        spdlog::info("Storage server listening on {}", storage_address_);
        server->Wait();
    }

private:
    const std::string naming_address_;
    const std::string storage_address_;

    // Stub for talking to the naming server
    std::unique_ptr<NamingService::Stub> naming_stub_;

    // Your implementation of the storage‑side RPCs
    StorageServiceImpl service_;
};
