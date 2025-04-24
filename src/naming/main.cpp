#include "pch.h"
#include "data_manager.h"
#include "heartbeat_monitor.h"
#include "naming_service_impl.h"

int main(int argc, char **argv) {
    const std::string port = (argc > 1 ? argv[1] : "6000");
    const std::string naming_address_ = "localhost:" + port;

    // enable support for curl or CLion .http requests
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    auto manager = std::make_shared<DataManager>();
    HeartbeatMonitor monitor(manager, 5, 10);
    NamingServiceImpl service(manager);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(naming_address_, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    const auto server = builder.BuildAndStart();
    spdlog::info("Naming Server Listening on {}", naming_address_);
    monitor.Start();

    server->Wait();
    spdlog::info("Naming Server Stopped");
    monitor.Stop();
    return 0;
}
