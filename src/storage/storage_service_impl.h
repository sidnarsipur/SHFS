#pragma once

class StorageServiceImpl final : public storage::StorageService::Service {
    public:
        explicit StorageServiceImpl(std::string storage_address): storage_address_(std::move(storage_address)) {}

        grpc::Status UploadFile(grpc::ServerContext* context, grpc::ServerReader<storage::UploadRequest>* reader, storage::UploadResponse* response) override {
            std::unique_lock lock(mu_);

            // Here we can process the stream of file data from the client
            storage::UploadRequest request;
            while (reader->Read(&request)) {
                // Process file data (e.g., write to disk or memory)
                spdlog::info("Received File Chunk: {}", request.filepath());
                // Append the received chunk to the storage (e.g., file on disk)
                std::ofstream newFile(request.filepath());

                if(newFile.is_open()){
                    newFile.write(request.data().c_str(),request.data().size());
                    spdlog::info("File {} Written to Storage Server", request.filepath());

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
            // Retrieve the file data from storage
    //        std::string file_data = GetFileData(request->filepath());

            // Stream the file data to the client in chunks
            storage::DownloadResponse response;
//            response.set_file_data(file_data);  // You can chunk the data into smaller pieces if necessary
            writer->Write(response);

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