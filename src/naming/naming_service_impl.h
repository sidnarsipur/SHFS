#pragma once

class NamingServiceImpl final : public naming::NamingService::Service {
public:
    explicit NamingServiceImpl(std::shared_ptr<DataManager> manager, const int delay = 1)
        : dm(std::move(manager)), delay_(delay) {}

    grpc::Status RegisterStorageServer(
        grpc::ServerContext *context,
        const naming::RegisterStorageRequest *request,
        naming::RegisterStorageResponse *response
    ) override {
        std::this_thread::sleep_for(std::chrono::seconds(delay_));

        dm->updateHeartbeat(request->storage_address());
        dm->active_servers().write([&](auto &s) { s.insert(request->storage_address()); });
        spdlog::info("Registered Storage Server with Naming Server");

        response->set_success(true);
        return grpc::Status::OK;
    }

    grpc::Status FindServersWithFile(
        grpc::ServerContext *context,
        const naming::FileLookupRequest *request,
        naming::FileLookupResponse *response
    ) override {
        std::this_thread::sleep_for(std::chrono::seconds(delay_));

        const auto &filepath = request->filepath();
        if (!dm->files().read([&](const auto &s) { return s.contains(filepath); })) {
            return {grpc::StatusCode::NOT_FOUND, "File not found."};
        }

        dm->active_servers().read([&](const auto &s) {
            for (const auto &address: s) {
                response->add_storage_addresses(address);
            }
        });
        return grpc::Status::OK;
    }

    grpc::Status ListFiles(
        grpc::ServerContext *context,
        const naming::Empty *request,
        naming::FileListResponse *response
    ) override {
        std::this_thread::sleep_for(std::chrono::seconds(delay_));

        dm->files().read([&](const auto &s) {
            for (const auto &file: s) {
                response->add_filepaths(file);
            }
        });
        return grpc::Status::OK;
    }

    grpc::Status UploadFile(
        grpc::ServerContext *context,
        const naming::FileUploadRequest *request,
        naming::FileUploadResponse *response
    ) override {
        std::this_thread::sleep_for(std::chrono::seconds(delay_));

        if (dm->active_servers().read([&](const auto &s) { return s.empty(); })) {
            return {grpc::StatusCode::FAILED_PRECONDITION, "No active storage server."};
        }

        dm->active_servers().read([&](const auto &s) {
            for (const auto &address: s) {
                response->add_storage_addresses(address);
            }
        });

        // TODO: handle updating a file, not just uploading new files
        dm->files().write([&](auto &s) { s.insert(request->filepath()); });
        return grpc::Status::OK;
    }

    grpc::Status RemoveFile(
        grpc::ServerContext *context,
        const naming::FileRemoveRequest *request,
        naming::FileRemoveResponse *response
    ) override {
        std::this_thread::sleep_for(std::chrono::seconds(delay_));

        const auto &filename = request->filepath();
        if (filename.empty()) {
            return {grpc::StatusCode::INVALID_ARGUMENT, "File name cannot be empty."};
        }

        if (!dm->files().read([&](const auto &s) { return s.contains(filename); })) {
            return {grpc::StatusCode::NOT_FOUND, "File not found in storage servers."};
        }

        // TODO: handle removing a file from storage servers. What happens if some servers fail to remove the file?
        dm->files().write([&](auto &s) { s.erase(filename); });
        return grpc::Status::OK;
    }

    grpc::Status Heartbeat(
        grpc::ServerContext *context,
        const naming::HeartbeatRequest *request,
        naming::HeartbeatResponse *response
    ) override {
        std::string addr = request->address();
        spdlog::info("Received heartbeat from: {}", addr);

        dm->updateHeartbeat(request->address());

        response->set_success(true);
        return grpc::Status::OK;
    }

private:
    std::shared_ptr<DataManager> dm;
    int delay_;
};
