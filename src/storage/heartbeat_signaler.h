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

    void SendHeartbeat() {
        naming::HeartbeatRequest req;
        naming::HeartbeatResponse res;
        grpc::ClientContext ctx;

        req.set_address(storage_address_);
        const auto status = naming_stub_.Heartbeat(&ctx, req, &res);

        if (!status.ok()) {
            spdlog::warn("Heartbeat Failed: {}", status.error_message());
            return;
        }

        handleTasksConcurrently(res.tasks());
    }

    void handleTasksConcurrently(const naming::TaskList &res) const {
        std::vector<std::thread> threads;

        for (const auto &protoTask: res.tasks()) {
            threads.emplace_back([&]() {
                processReplicationTask(protoTask);
            });
        }

        for (auto &t: threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    // Placeholder function
    void processReplicationTask(const naming::Task &task) const {
        auto stub = storage::StorageService::NewStub(
            CreateChannel(task.source(), grpc::InsecureChannelCredentials()));

        grpc::ClientContext context;
        storage::DownloadResponse response;
        storage::DownloadRequest request;
        request.set_filepath(task.filepath());

        std::unique_ptr reader(stub->DownloadFile(&context, request));
        std::ofstream outFile("data/" + task.filepath(), std::ios::binary);
        bool success = false;

        while (reader->Read(&response)) {
            if (!response.success()) {
                spdlog::error("Download failed: {}", response.error_message());
                break;
            }
            outFile << response.file_data();
            success = true;
        }

        outFile.close();
        grpc::Status status = reader->Finish();
        if (!status.ok()) {
            spdlog::error("gRPC failed: {}", status.error_message());
        } else if (success) {
            spdlog::info("Downloaded: {} from {}", task.filepath(), task.source());
            dm->addFile(task.filepath());
        }
    }
};
