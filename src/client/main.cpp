#include "pch.h"

void upload_file(naming::NamingService::Stub &naming_stub, const std::string& filepath) {
    if (!std::filesystem::exists(filepath)) {
           std::cout << "File does not exist: " << filepath << std::endl;
        return;
    }
    if (!std::filesystem::is_regular_file(filepath)) {
        std::cout << "Not a regular file:" << filepath << std::endl;
        return;
    }

    grpc::ClientContext context;
    naming::FileUploadRequest request;
    naming::FileUploadResponse response;

    request.set_filepath(filepath);

    auto status = naming_stub.UploadFile(&context, request, &response);

    if(!status.ok()){
        std::cout << "Error Uploading File" << std::endl;
        std::cout << response.error_message() << std::endl;
        return;
    }

    std::cout << "Successfully uploaded file" << filepath << std::endl;
}

void download_file(naming::NamingService::Stub &naming_stub, std::string &filepath){
}

void list_files(naming::NamingService::Stub &naming_stub) {

    grpc::ClientContext context;
    naming::Empty empty;
    naming::FileListResponse res;

    auto status = naming_stub.ListFiles(&context, empty, &res);

    if(!status.ok()) {
        spdlog::error("List Files RPC failed");
    }

    if(res.filepaths().size() == 0){
        std::cout << "No files in filesystem" << std::endl;
        return;
    }

    for(int i = 0; i < res.filepaths_size(); ++i) {
        std::cout << res.filepaths(i) << std::endl;
    }
}

int main(int argc, char** argv) {
    auto stub = naming::NamingService::NewStub(grpc::CreateChannel("localhost:6000", grpc::InsecureChannelCredentials()));

    CLI::App app{"File System CLI"};
    app.require_subcommand(1);
    argv = app.ensure_utf8(argv);

    // Command: upload
    auto upload_cmd = app.add_subcommand("upload", "Upload a file to the file system");
    std::string upload_filepath;
    upload_cmd->add_option("filepath", upload_filepath, "The file to upload")->required();
    upload_cmd->callback([&]() {
        upload_file(*stub, upload_filepath);
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
        list_files(*stub);
    });

    // Parse CLI args
    CLI11_PARSE(app, argc, argv);

    return 0;
}
