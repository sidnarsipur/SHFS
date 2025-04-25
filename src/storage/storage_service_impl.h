#pragma once

class StorageServiceImpl final : public storage::StorageService::Service {
    public:
        explicit StorageServiceImpl(std::string storage_address, std::shared_ptr<StorageDataManager> manager)
        : storage_address_(std::move(storage_address)), sdm(std::move(manager))
        {
            std::filesystem::create_directories("data");
        }

        grpc::Status UploadFile(
                grpc::ServerContext* context,
                grpc::ServerReader<storage::UploadRequest>* reader,
                storage::UploadResponse* response)
                override {

            std::unique_lock lock(mu_);
            storage::UploadRequest request;

            while (reader->Read(&request)) {
                spdlog::info("Received Upload Request for file {}", request.filepath());
                std::ofstream newFile("data/" + request.filepath());

                if(newFile.is_open()){
                    newFile.write(request.data().c_str(),request.data().size());
                    sdm->addFile(request.filepath());
                    spdlog::info("Finish Upload Request for file {}", request.filepath());

                    newFile.close();
                }
                else {
                    spdlog::error("File Storage failed");
                    return grpc::Status::CANCELLED;
                }
            }

            response->set_success(true);
            return grpc::Status::OK;
        }

        grpc::Status DownloadFile(
                grpc::ServerContext* context,
                const storage::DownloadRequest* request,
                grpc::ServerWriter<storage::DownloadResponse>* writer)
                override {

            std::unique_lock lock(mu_);

            std::ifstream file("data/" + request->filepath(), std::ios::binary);
            storage::DownloadResponse res;

            spdlog::info("Received Download Request for file {}", request->filepath());

            if(!file.is_open()){
                spdlog::error("Failed to Download File: {}", request->filepath());
                res.set_success(false);
                res.set_error_message(("Failed to open file requested"));

                writer->Write(res);
                return grpc::Status::CANCELLED;
            }

            std::ostringstream buffer;
            buffer << file.rdbuf();
            file.close();

            res.set_file_data(buffer.str());

            writer->Write(res);

            spdlog::info("Finish Download Request for file {}", request->filepath());

            return grpc::Status::OK;
        }

        grpc::Status RemoveFile(
                grpc::ServerContext* context,
                const ::storage::RemoveRequest* request,
                storage::RemoveResponse* response)
                override {

            std::unique_lock lock(mu_);

            if(!sdm->fileExists(request->filepath())){
                response->set_success(false);
                response->set_error_message("File does not exist");

                return grpc::Status::CANCELLED;
            }

            try{
                std::filesystem::remove("data/" + request->filepath());
            } catch (const std::filesystem::filesystem_error& e){
                response->set_success(false);
                response->set_error_message("Error removing file");

                return grpc::Status::CANCELLED;
            }

            sdm->removeFile(request->filepath());

            response->set_success(true);
            spdlog::info("Removed file {}", request->filepath());

            return grpc::Status::OK;

        }

    grpc::Status ShareFiles(
            ::grpc::ServerContext* context,
            const ::storage::EmptyMessage* request,
            ::grpc::ServerWriter< ::storage::ShareResponse>* writer) override {

            std::unique_lock lock(mu_);

            spdlog::info("Received ShareFiles Request");

            storage::ShareResponse res;

            sdm->files().read([&res](const std::unordered_set<std::string>& files) {
                for(const auto& file: files){
                    std::ifstream inFile("data/" + file, std::ios::binary);

                    if(!inFile.is_open()){
                        spdlog::warn("Failed to open file: {}", file);
                    }

                    std::ostringstream buffer;
                    buffer << inFile.rdbuf();
                    inFile.close();

                    auto* outFile = res.add_files();

                    outFile->set_file_path(file);
                    outFile->set_file_data(buffer.str());
                }
            });

            writer->Write(res);
            spdlog::info("Finished ShareFiles Request");

            return grpc::Status::OK;
        }

    private:
        std::string storage_address_;
        std::shared_mutex mu_;
        std::shared_ptr<StorageDataManager> sdm;
};