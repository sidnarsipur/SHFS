#pragma once

class NamingServiceImpl final : public naming::NamingService::Service {
public:
    explicit NamingServiceImpl(std::shared_ptr<NamingDataManager> manager)
        : dm(std::move(manager)) {}

    grpc::Status RegisterStorageServer(
        grpc::ServerContext *context,
        const naming::RegisterStorageRequest *request,
        naming::RegisterStorageResponse *response
    ) override {
        std::this_thread::sleep_for(std::chrono::seconds(dm->delay));

        dm->newServer(request->storage_address());
        spdlog::info("Registered Storage Server {}", request->storage_address());

        response->set_success(true);
        return grpc::Status::OK;
    }

    grpc::Status FindServersWithFile(
        grpc::ServerContext *context,
        const naming::FileLookupRequest *request,
        naming::FileLookupResponse *response
    ) override {
        std::this_thread::sleep_for(std::chrono::seconds(dm->delay));

        const auto &filepath = request->filepath();
        if (!dm->files().read([&](const auto &s) { return s.contains(filepath); })) {
            return {grpc::StatusCode::NOT_FOUND, "File not found."};
        }

        dm->files().read([&](const auto &m) {
            for (const auto &address: m.at(filepath)) {
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
        std::this_thread::sleep_for(std::chrono::seconds(dm->delay));

        dm->files().read([&](const auto &s) {
            for (const auto &file: s | std::views::keys) {
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
        std::this_thread::sleep_for(std::chrono::seconds(dm->delay));
        const auto active_servers = dm->servers().get();

        if (active_servers.empty()) {
            return {grpc::StatusCode::FAILED_PRECONDITION, "No active storage server."};
        }

        if (active_servers.size() < dm->replication_factor) {
            spdlog::critical("Not enough active storage servers to maintain replication factor of {}",
                             dm->replication_factor);
        }

        const auto &filename = request->filepath();
        if(dm->fileExists(filename)){
            response->set_error_message("File already exists, editing file.");
        }

        const int upload_server_count = std::min(dm->replication_factor, static_cast<int>(active_servers.size()));
        const std::vector<std::string> selected_servers = dm->getLeastLoadedServers(upload_server_count);

        for (const auto &address: selected_servers) {
            dm->addServerForFile(filename, address);
            response->add_storage_addresses(address);
        }
        return grpc::Status::OK;
    }

    grpc::Status RemoveFile(
        grpc::ServerContext *context,
        const naming::FileRemoveRequest *request,
        naming::FileRemoveResponse *response
    ) override {
        std::this_thread::sleep_for(std::chrono::seconds(dm->delay));

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

        dm->updateHeartbeat(addr);
        const std::vector<Task> tasks = dm->getReplicationTasks(addr);

        naming::TaskList *taskList = response->mutable_tasks();
        for (const auto &task: tasks) {
            naming::Task *newTask = taskList->add_tasks();
            newTask->set_source(task.source);
            newTask->set_filepath(task.filepath);
            dm->addServerForFile(task.filepath, addr);
        }

        response->set_success(true);
        return grpc::Status::OK;
    }

    grpc::Status Log(grpc::ServerContext *context, const naming::Empty *request, naming::Empty *response) override {
        dm->log();
        return grpc::Status::OK;
    }

    grpc::Status GetFileToServersMapping(
        grpc::ServerContext *context,
        const naming::Empty *request,
        naming::FileToServersMapping *response
    ) override {
        std::this_thread::sleep_for(std::chrono::seconds(dm->delay));

        dm->files().read([&](const auto &m) {
            for (const auto &[filepath, servers]: m) {
                naming::ServersWithFile *serversWithFile = response->add_serverswithfile();
                serversWithFile->set_filepath(filepath);

                for (const auto &server: servers) {
                    serversWithFile->add_servers(server);
                }
            }
        });

        return grpc::Status::OK;
    }

private:
    std::shared_ptr<NamingDataManager> dm;
};
