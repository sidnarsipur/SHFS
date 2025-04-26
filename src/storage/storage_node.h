#pragma once

#include "storage_data_manager.h"
#include "heartbeat_signaler.h"
#include "storage_service_impl.h"

class StorageNode {
    public:
        StorageNode(std::string naming_address, std::string storage_address):
            naming_address_(std::move(naming_address)),
            storage_address_(std::move(storage_address)),
            naming_stub_(naming::NamingService::NewStub(grpc::CreateChannel(naming_address_, grpc::InsecureChannelCredentials()))) {}

        void Register() const {
            naming::RegisterStorageRequest req;
            naming:: RegisterStorageResponse res;
            grpc::ClientContext ctx;

            req.set_storage_address(storage_address_);
            const auto status = naming_stub_->RegisterStorageServer(&ctx, req, &res);

            if (!status.ok()) {
                spdlog::error("Registration failed. Is Naming server running?\n{}", status.error_message());
                std::exit(EXIT_FAILURE);
            }

            spdlog::info("Registered Storage Server",storage_address_ );
        }

        void Run() {
            // enable support for curl or CLion .http requests
            grpc::EnableDefaultHealthCheckService(true);
            grpc::reflection::InitProtoReflectionServerBuilderPlugin();

            auto manager = std::make_shared<StorageDataManager>();
            HeartbeatSignaler signaler(storage_address_, manager,  *naming_stub_, 5);
            StorageServiceImpl service(storage_address_, manager);

            grpc::ServerBuilder builder;
            builder.AddListeningPort(storage_address_, grpc::InsecureServerCredentials());
            builder.RegisterService(&service);

            const auto server = builder.BuildAndStart();
            signaler.Start();
            spdlog::info("Storage Server Listening on {}", storage_address_);

            server->Wait();
            spdlog::info("Storage Server Stopped");
            signaler.Stop();
        }

    private:
        const std::string naming_address_;
        const std::string storage_address_;

        // Stub for talking to the naming server
        std::unique_ptr<naming::NamingService::Stub> naming_stub_;
};
