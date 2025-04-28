#pragma once

class NamingServiceImpl final : public naming::NamingService::Service {
public:
    explicit NamingServiceImpl(std::shared_ptr<NamingDataManager> manager, const int delay = 0)
        : dm(std::move(manager)), delay_(delay) {}

    grpc::Status RegisterStorageServer(
        grpc::ServerContext *context,
        const naming::RegisterStorageRequest *request,
        naming::RegisterStorageResponse *response
    ) override {
        std::this_thread::sleep_for(std::chrono::seconds(delay_));

        dm->updateHeartbeat(request->storage_address());
        dm->active_servers().write([&](auto &s) { s.insert(request->storage_address()); });
        spdlog::info("Registered Storage Server {}", request->storage_address());

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
        std::this_thread::sleep_for(std::chrono::seconds(delay_));

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
        std::this_thread::sleep_for(std::chrono::seconds(delay_));
        auto active_servers = dm->active_servers().get();

        if (active_servers.empty()) {
            return {grpc::StatusCode::FAILED_PRECONDITION, "No active storage server."};
        }

        for (const auto &address: active_servers) {
            response->add_storage_addresses(address);
        }

        // TODO: handle updating a file, not just uploading new files
        const auto &filename = request->filepath();
        for (const auto &address: active_servers) {
            dm->addServerForFile(filename, address);
        }
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

        dm->updateHeartbeat(addr);
        const std::vector<Task> tasks = dm->getReplicationTasks(addr);

        // TODO: remove for production
        // spdlog::info("Assigning {} replication task(s) to {}: [{}]",
        //              tasks.size(), addr,
        //              fmt::join(tasks | std::views::transform([](const Task& t) {
        //                  return fmt::format("{{source: {}, filepath: {}}}", t.source, t.filepath);
        //              }), ", "));

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
        std::this_thread::sleep_for(std::chrono::seconds(delay_));

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
    int delay_;
};
