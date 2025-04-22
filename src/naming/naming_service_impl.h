#pragma once

class NamingServiceImpl final : public NamingService::Service {
public:
    NamingServiceImpl() = default;

    grpc::Status RegisterStorageServer(grpc::ServerContext *context,
                                       const RegisterStorageRequest *request,
                                       RegisterStorageResponse *response) override {
        std::unique_lock lock(mu_);
        storage_servers_.insert(request->storage_address());
        response->set_success(true);
        return grpc::Status::OK;
    }

    grpc::Status GetStorageServersForFile(grpc::ServerContext *context,
                                          const FileLookupRequest *request,
                                          FileLookupResponse *response) override {
        std::shared_lock lock(mu_);
        const auto &filename = request->filename();
        if (file_locations_.contains(filename)) {
            for (const auto &addr: file_locations_[filename]) {
                response->add_storage_addresses(addr);
            }
        }
        return grpc::Status::OK;
    }

    grpc::Status ListAllFiles(grpc::ServerContext *context,
                              const Empty *request,
                              FileListResponse *response) override {
        std::shared_lock lock(mu_);
        for (const auto &key: file_locations_ | std::views::keys) {
            response->add_filenames(key);
        }
        return grpc::Status::OK;
    }

    grpc::Status GetStorageServerForUpload(grpc::ServerContext *context,
                                           const FileUploadRequest *request,
                                           FileUploadResponse *response) override {
        std::unique_lock lock(mu_);
        if (storage_servers_.empty()) {
            return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "No storage servers registered.");
        }

        // TODO: Add smarter selection strategy (e.g., round-robin, load-based)
        auto it = storage_servers_.begin();
        response->set_storage_address(*it);

        // Update internal state
        // TODO: handle updating a file, not just uploading new files
        file_locations_[request->filename()].insert(*it);

        return grpc::Status::OK;
    }

    grpc::Status RemoveFile(grpc::ServerContext *context,
                            const FileRemoveRequest *request,
                            FileRemoveResponse *response) override {
        std::unique_lock lock(mu_);
        const auto &filename = request->filename();
        const auto it = file_locations_.find(filename);
        if (it != file_locations_.end()) {
            file_locations_.erase(it);
            // TODO: Inform storage servers to delete file (distributed implementation)
            response->set_success(true);
        } else {
            response->set_success(false);
        }
        return grpc::Status::OK;
    }

    grpc::Status Heartbeat(grpc::ServerContext* context,
                               const HeartbeatRequest* request,
                               HeartbeatResponse* response) override {
        std::string addr = request->address();
        spdlog::info("Received heartbeat from: {}", addr);

        // You could update an internal map of address -> last seen timestamp here
        // serverState[addr] = std::chrono::steady_clock::now();

        response->set_success(true);
        response->set_message("Heartbeat received.");
        return grpc::Status::OK;
    }

private:
    std::shared_mutex mu_; // For thread safety

    // Set of registered storage server addresses
    std::unordered_set<std::string> storage_servers_;

    // Map from filenames to the set of storage servers that store them
    std::unordered_map<std::string, std::unordered_set<std::string>> file_locations_;
};