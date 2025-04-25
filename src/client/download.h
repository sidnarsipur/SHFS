#pragma once

inline std::vector<std::string> getStorageServersForFile(naming::NamingService::Stub &stub, const std::string &filename) {
    naming::FileLookupRequest request;
//    request.set_filename(filename);
    naming::FileLookupResponse reply;
    grpc::ClientContext context;

    auto status = stub.FindServersWithFile(&context, request, &reply);
    if (!status.ok()) {
        spdlog::error("RPC failed");
        std::exit(EXIT_FAILURE);
    }

    return {reply.storage_addresses().begin(), reply.storage_addresses().end()};
}

inline void download_file(naming::NamingService::Stub &naming_stub, const std::string &filename) {
    const auto storage_addresses = getStorageServersForFile(naming_stub, filename);
    if (storage_addresses.empty()) {
        spdlog::error("No storage servers found for file: {}", filename);
        return;
    }

    for (const auto &address: storage_addresses) {
        auto storage_stub = storage::StorageService::NewStub(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
        grpc::ClientContext context;
        storage::DownloadRequest request;
//        request.set_file_name(filename);
        storage::DownloadResponse response;

        std::unique_ptr<grpc::ClientReader<storage::DownloadResponse>> reader = storage_stub->DownloadFile(&context, request);
        while (reader->Read(&response)) {
            // Process the downloaded file data
            spdlog::info("Received file data: {}", response.file_data());
        }
    }
}
