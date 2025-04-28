#include "pch.h"
#include "naming_data_manager.h"
#include "heartbeat_monitor.h"
#include "naming_service_impl.h"

int main(int argc, char **argv) {
    const std::string port = (argc > 1 ? argv[1] : "6000");
    const std::string naming_address_ = "localhost:" + port;
    const std::string timeout_str = (argc > 2 ? argv[2] : "10");
    const int timeout = std::stoi(timeout_str);
    const std::string replication_factor_str = (argc > 3 ? argv[3] : "2");
    const int replication_factor = std::stoi(replication_factor_str);
    const int delay = 0; // Default delay

    // enable support for curl or CLion .http requests
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    auto manager = std::make_shared<NamingDataManager>(delay, replication_factor);
    HeartbeatMonitor monitor(manager, 5, timeout);
    // monitor.test_basic_single_source();
    // monitor.test_multiple_sources();
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
