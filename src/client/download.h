#pragma once

inline std::vector<std::string> getStorageServersForFile(NamingService::Stub &stub, const std::string &filename) {
    FileLookupRequest request;
//    request.set_filename(filename);
    FileLookupResponse reply;
    grpc::ClientContext context;

    auto status = stub.FindServersWithFile(&context, request, &reply);
    if (!status.ok()) {
        spdlog::error("RPC failed");
        std::exit(EXIT_FAILURE);
    }

    return {reply.storage_addresses().begin(), reply.storage_addresses().end()};
}

inline void download_file(NamingService::Stub &naming_stub, const std::string &filename) {
    const auto storage_addresses = getStorageServersForFile(naming_stub, filename);
    if (storage_addresses.empty()) {
        spdlog::error("No storage servers found for file: {}", filename);
        return;
    }

    for (const auto &address: storage_addresses) {
        auto storage_stub = StorageService::NewStub(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
        grpc::ClientContext context;
        DownloadRequest request;
//        request.set_file_name(filename);
        DownloadResponse response;

        std::unique_ptr<grpc::ClientReader<DownloadResponse>> reader = storage_stub->DownloadFile(&context, request);
        while (reader->Read(&response)) {
            // Process the downloaded file data
            spdlog::info("Received file data: {}", response.file_data());
        }
    }
}
