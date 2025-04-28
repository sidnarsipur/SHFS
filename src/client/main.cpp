#include "pch.h"

void upload_file(naming::NamingService::Stub &naming_stub, const std::string& filepath) {
    if (!std::filesystem::exists(filepath)) {
        std::cout << "File does not exist: " << filepath << std::endl;
        return;
    }

    if (!std::filesystem::is_regular_file(filepath)) {
        std::cout << "Not a regular file: " << filepath << std::endl;
        return;
    }

    grpc::ClientContext context;
    naming::FileUploadRequest request;
    naming::FileUploadResponse response;

    request.set_filepath(filepath);
    auto status = naming_stub.UploadFile(&context, request, &response);

    if (!status.ok()) {
        std::cout << "Error Uploading File to Naming Server: " << status.error_message() << std::endl;
        return;
    }

    std::vector<std::thread> threads;

    for (int i = 0; i < response.storage_addresses_size(); ++i) {
        threads.emplace_back([filepath, address = response.storage_addresses(i)]() {
            auto storageStub = storage::StorageService::NewStub(
                    grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));

            grpc::ClientContext storage_context;
            storage::UploadResponse storage_response;
            std::unique_ptr<grpc::ClientWriter<storage::UploadRequest>> writer(
                    storageStub->UploadFile(&storage_context, &storage_response));

            std::ifstream file(filepath, std::ios::binary);
            if (!file) {
                std::cerr << "Failed to open file: " << filepath << std::endl;
                return;
            }

            std::string file_content(
                    (std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());

            storage::UploadRequest upload_request;
            upload_request.set_filepath(filepath);
            upload_request.set_data(file_content);
            if (!writer->Write(upload_request)) {
                std::cerr << "Failed to write file to storage server: " << address << std::endl;
            }

            writer->WritesDone();
            grpc::Status status = writer->Finish();

            if (!status.ok()) {
                std::cerr << "Upload to " << address << " failed: " << status.error_message() << std::endl;
            } else if (!storage_response.success()) {
                std::cerr << "Server responded with error: " << storage_response.error_message() << std::endl;
            } else {
                std::cout << "Successfully uploaded to: " << address << std::endl;
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
}

void download_file(naming::NamingService::Stub &naming_stub, std::string &filepath){
    std::filesystem::create_directories("data");

    grpc::ClientContext context;
    naming::FileLookupRequest request;
    naming::FileLookupResponse response;

    request.set_filepath(filepath);

    auto status = naming_stub.FindServersWithFile(&context, request, &response);

    if(!status.ok()){
        std::cout << "Error Downloading File: " << status.error_message() << std::endl;
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution distrib(0, response.storage_addresses_size()-1);

    int random_server_idx = distrib(gen);

    std::string address = response.storage_addresses(random_server_idx);

    grpc::ClientContext context2;
    storage::DownloadRequest dow_req;
    storage::DownloadResponse dow_res;

    dow_req.set_filepath(filepath);

    auto storage_stub = storage::StorageService::NewStub(
            grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));

    std::unique_ptr<grpc::ClientReader<storage::DownloadResponse>> reader(
            storage_stub->DownloadFile(&context2, dow_req));

    std::ofstream newFile;

    bool fileRead = false;

   while(reader->Read(&dow_res)){
       if(!fileRead){
           if(!dow_res.success()){
               std::cout << "Error Downloading File: " << status.error_message() << std::endl;
               return;
           }

           newFile.open("data/" + request.filepath());
           fileRead = true;
       }

       if(newFile.is_open()){
           newFile.write(dow_res.file_data().c_str(), dow_res.file_data().size());
       }
   }

   newFile.close();

   std::cout << "Successfully Downloaded File " << filepath << std::endl;
}

void info_files(naming::NamingService::Stub &naming_stub){
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

    // Command: info
    auto info_cmd = app.add_subcommand("info", "List all files and their locations in the file system");
    info_cmd->callback([&]() {
        info_files(*stub);
    });

    // Parse CLI args
    CLI11_PARSE(app, argc, argv);

    return 0;
}
