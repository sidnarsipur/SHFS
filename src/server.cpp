#include <grpcpp/grpcpp.h>
#include "hello.grpc.pb.h"
#include <iostream>
#include <memory>

class GreeterServiceImpl final : public Greeter::Service {
    grpc::Status SayHello(grpc::ServerContext* context, const HelloRequest* request,
                          HelloReply* reply) override {
        reply->set_message("Hello, " + request->name());
        return grpc::Status::OK;
    }
};

int main() {
    GreeterServiceImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on 0.0.0.0:50051\n";
    server->Wait();
}
