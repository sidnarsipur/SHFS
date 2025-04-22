#include <grpcpp/grpcpp.h>
#include <iostream>
#include <fstream>
#include <string>
#include "CLI11.hpp"
#include "naming.grpc.pb.h"
#include "storage.grpc.pb.h"
#include "upload.h"
#include "download.h"


void upload_file (const std::string& filepath) {
}

void list_files() {
}

int main(int argc, char** argv) {
    auto stub = NamingService::NewStub(grpc::CreateChannel("localhost:6000", grpc::InsecureChannelCredentials()));

    CLI::App app{"File System CLI"};
    app.require_subcommand(1);
    argv = app.ensure_utf8(argv);

    // Command: upload
    auto upload_cmd = app.add_subcommand("upload", "Upload a file to the file system");
    std::string upload_filepath;
    upload_cmd->add_option("filepath", upload_filepath, "The file to upload")->required();
    upload_cmd->callback([&]() {
        upload_file(upload_filepath);
    });

    // Command: download
    auto download_cmd = app.add_subcommand("download", "Download a file from the file system");
    std::string download_filename;
    download_cmd->add_option("filename", download_filename, "The file to read")->required();
    download_cmd->callback([&]() {
        download_file(*stub, download_filename);
    });

    // Command: list
    auto list_cmd = app.add_subcommand("list", "List all files in the file system");
    list_cmd->callback([&]() {
        list_files();
    });

    // Parse CLI args
    CLI11_PARSE(app, argc, argv);

    return 0;
}
