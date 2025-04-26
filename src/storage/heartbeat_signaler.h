#pragma once

class HeartbeatSignaler {
public:
    explicit HeartbeatSignaler(
        std::string storage_address_,
        std::shared_ptr<StorageDataManager> manager,
        naming::NamingService::Stub &naming_stub,
        const int interval
    )
        : storage_address_(std::move(storage_address_)), dm(std::move(manager)), naming_stub_(naming_stub),
          interval_(interval), stop_flag_(false) {}

    void Start() {
        thread_ = std::thread([this] {
            while (!stop_flag_.load()) {
                SendHeartbeat();
                std::this_thread::sleep_for(std::chrono::seconds(interval_));
            }
        });
    }

    void Stop() {
        stop_flag_.store(true);
        if (thread_.joinable()) {
            thread_.join();
        }
    }

private:
    std::string storage_address_;
    std::shared_ptr<StorageDataManager> dm;
    naming::NamingService::Stub &naming_stub_;
    int interval_;
    std::atomic<bool> stop_flag_;
    std::thread thread_;

    void SendHeartbeat() const {
        naming::HeartbeatRequest req;
        naming::HeartbeatResponse res;
        grpc::ClientContext ctx;

        req.set_address(storage_address_);
        const auto status = naming_stub_.Heartbeat(&ctx, req, &res);

        if (!status.ok()) {
            spdlog::warn("Heartbeat Failed: {}", status.error_message());
        }
    }
};
