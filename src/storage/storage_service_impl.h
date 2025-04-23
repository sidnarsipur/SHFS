#pragma once

class StorageServiceImpl final : public storage::StorageService::Service {
    public:
        explicit StorageServiceImpl(std::string storage_address): storage_address_(std::move(storage_address)) {
            std::filesystem::create_directories("data");
        }

        grpc::Status UploadFile(grpc::ServerContext* context, grpc::ServerReader<storage::UploadRequest>* reader, storage::UploadResponse* response) override {
            std::unique_lock lock(mu_);

            // Here we can process the stream of file data from the client
            storage::UploadRequest request;
            while (reader->Read(&request)) {
                // Process file data (e.g., write to disk or memory)
                spdlog::info("Received Upload Request for file {}", request.filepath());
                // Append the received chunk to the storage (e.g., file on disk)
                std::ofstream newFile("data/" + request.filepath());

                if(newFile.is_open()){
                    newFile.write(request.data().c_str(),request.data().size());
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

        grpc::Status DownloadFile(grpc::ServerContext* context, const storage::DownloadRequest* request, grpc::ServerWriter<storage::DownloadResponse>* writer) override {
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

    private:
        std::string storage_address_;

        // For thread safety
        std::shared_mutex mu_;

        // Set of registered storage server addresses
        std::unordered_set<std::string> storage_servers_;

        // Map from filenames to the set of storage servers that store them
        std::unordered_map<std::string, std::unordered_set<std::string>> file_locations_;
};