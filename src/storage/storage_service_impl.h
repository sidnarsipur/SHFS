#pragma once

class StorageServiceImpl final : public StorageService::Service {
public:
    StorageServiceImpl() = default;

    grpc::Status StorageServiceImpl::UploadFile(
        grpc::ServerContext* context,
        grpc::ServerReader<UploadRequest>* reader,
        UploadResponse* response) override {
        // Here we can process the stream of file data from the client
        UploadRequest request;
        while (reader->Read(&request)) {
            // Process file data (e.g., write to disk or memory)
            std::cout << "Received file chunk: " << request.file_name() << std::endl;
            // Append the received chunk to the storage (e.g., file on disk)
        }

        // After receiving the entire file, return success
        response->set_success(true);
        return grpc::Status::OK;
    }

    grpc::Status StorageServiceImpl::DownloadFile(
    grpc::ServerContext* context,
    const DownloadRequest* request,
    grpc::ServerWriter<DownloadResponse>* writer) override {
        // Retrieve the file data from storage
        std::string file_data = GetFileData(request->file_name());

        // Stream the file data to the client in chunks
        DownloadResponse response;
        response.set_file_data(file_data);  // You can chunk the data into smaller pieces if necessary
        writer->Write(response);

        return grpc::Status::OK;
    }

private:
    std::shared_mutex mu_; // For thread safety

    // Set of registered storage server addresses
    std::unordered_set<std::string> storage_servers_;

    // Map from filenames to the set of storage servers that store them
    std::unordered_map<std::string, std::unordered_set<std::string>> file_locations_;
};