#pragma once
#include <utility>

#include "storage_service_impl.h"

class StorageNode {
    public:
        StorageNode(std::string naming_address, std::string storage_address):
            naming_address_(std::move(naming_address)),
            storage_address_(std::move(storage_address)),
            naming_stub_(naming::NamingService::NewStub(grpc::CreateChannel(naming_address_, grpc::InsecureChannelCredentials()))),
            service_(std::move(storage_address))
            {}

        void Register() const {
            naming::RegisterStorageRequest req;
            naming:: RegisterStorageResponse res;
            grpc::ClientContext ctx;

            req.set_storage_address(storage_address_);
            auto status = naming_stub_->RegisterStorageServer(&ctx, req, &res);

            if (!status.ok()) {
                spdlog::error("Registration failed: {}", status.error_message());
                std::exit(EXIT_FAILURE);
            }

            spdlog::info("Registered Storage Server with Naming Server at {}", storage_address_);
        }

        bool SendHeartbeat() const {
            naming::HeartbeatRequest req;
            naming::HeartbeatResponse res;
            grpc::ClientContext ctx;

            req.set_address(storage_address_);
            auto status = naming_stub_->Heartbeat(&ctx, req, &res);

            if (status.ok()) {
                spdlog::info("Heartbeat Ack: {}", res.error_message());
                return true;
            }
            else {
                spdlog::warn("Heartbeat Failed: {}", status.error_message());
                return false;
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
            spdlog::info("Storage Server Listening on {}", storage_address_);
            server->Wait();
        }

    private:
        const std::string naming_address_;
        const std::string storage_address_;

        // Stub for talking to the naming server
        std::unique_ptr<naming::NamingService::Stub> naming_stub_;

        // Your implementation of the storage‑side RPCs
        StorageServiceImpl service_;
};
