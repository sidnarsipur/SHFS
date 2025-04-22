#pragma once
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <string>
#include <vector>
#include "naming.grpc.pb.h"

inline std::vector<std::string> getStorageServersForFile(NamingService::Stub &stub, const std::string &filename) {
    FileLookupRequest request;
    request.set_filename(filename);
    FileLookupResponse reply;
    grpc::ClientContext context;

    auto status = stub.GetStorageServersForFile(&context, request, &reply);
    if (!status.ok()) {
        std::cerr << "RPC failed" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return {reply.storage_addresses().begin(), reply.storage_addresses().end()};
}

inline void download_file(NamingService::Stub &stub, const std::string &filename) {
    auto storage_addresses = getStorageServersForFile(stub, filename);
    if (storage_addresses.empty()) {
        std::cerr << "No storage servers found for file: " << filename << std::endl;
        return;
    }

    for (const auto &address: storage_addresses) {
        auto stub = StorageService::NewStub(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
        grpc::ClientContext context;
        DownloadRequest request;
        request.set_file_name(filename);
        DownloadResponse response;

        std::unique_ptr<grpc::ClientReader<DownloadResponse>> reader = stub->DownloadFile(&context, request);
        while (reader->Read(&response)) {
            // Process the downloaded file data
            std::cout << "Received file data: " << response.file_data() << std::endl;
        }
    }
}
