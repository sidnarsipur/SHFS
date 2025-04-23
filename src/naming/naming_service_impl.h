#pragma once

class NamingServiceImpl final : public naming::NamingService::Service {
public:
    NamingServiceImpl() = default;

    grpc::Status RegisterStorageServer(grpc::ServerContext *context, const naming::RegisterStorageRequest *request,
                                       naming::RegisterStorageResponse *response) override {
        std::unique_lock lock(mu_);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms_));

        storage_servers_.insert(request->storage_address());
        spdlog::info("Registered Storage Server with Naming Server");

        response->set_success(true);
        return grpc::Status::OK;
    }

    grpc::Status FindServersWithFile(grpc::ServerContext *context, const naming::FileLookupRequest *request,
                                     naming::FileLookupResponse *response) override {
        std::shared_lock lock(mu_);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms_));

        const auto &filepath = request->filepath();
        if (files_.contains(filepath)) {
            for (const auto &address: storage_servers_) {
                response->add_storage_addresses(address);
            }
        }
        return grpc::Status::OK;
    }

    grpc::Status ListFiles(grpc::ServerContext *context, const naming::Empty *request,
                           naming::FileListResponse *response) override {
        spdlog::info("Listing files");
        std::shared_lock lock(mu_);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms_));

        for (const auto &file: files_) {
            response->add_filepaths(file);
        }
        spdlog::info("Listed files");
        return grpc::Status::OK;
    }

    grpc::Status UploadFile(grpc::ServerContext *context, const naming::FileUploadRequest *request,
                            naming::FileUploadResponse *response) override {
        std::unique_lock lock(mu_);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms_));

        if (storage_servers_.empty()) {
            return {grpc::StatusCode::FAILED_PRECONDITION, "No storage servers registered."};
        }

        for (const auto &address: storage_servers_) {
            response->add_storage_addresses(address);
        }

        // TODO: handle updating a file, not just uploading new files
        files_.insert(request->filepath());
        return grpc::Status::OK;
    }

    grpc::Status RemoveFile(grpc::ServerContext *context, const naming::FileRemoveRequest *request,
                            naming::FileRemoveResponse *response) override {
        std::unique_lock lock(mu_);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms_));

        const auto &filename = request->filepath();
        if (filename.empty()) {
            return {grpc::StatusCode::INVALID_ARGUMENT, "File name cannot be empty."};
        }

        if (!files_.contains(filename)) {
            return {grpc::StatusCode::NOT_FOUND, "File not found."};
        }

        // TODO: handle removing a file from storage servers. What happens if some servers fail to remove the file?
        files_.erase(filename);
        return grpc::Status::OK;
    }

    grpc::Status Heartbeat(grpc::ServerContext *context, const naming::HeartbeatRequest *request,
                           naming::HeartbeatResponse *response) override {
        std::string addr = request->address();
        spdlog::info("Received heartbeat from: {}", addr);

        // You could update an internal map of address -> last seen timestamp here
        // serverState[addr] = std::chrono::steady_clock::now();

        response->set_success(true);
        // response->set_error_message("Heartbeat received.");
        return grpc::Status::OK;
    }

private:
    int delay_ms_ = 1000;
    std::shared_mutex mu_;
    std::unordered_set<std::string> storage_servers_;
    std::unordered_set<std::string> files_;
};
