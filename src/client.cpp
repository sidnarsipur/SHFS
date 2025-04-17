#include <grpcpp/grpcpp.h>
#include "hello.grpc.pb.h"

int main() {
  auto stub = Greeter::NewStub(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

  HelloRequest request;
  request.set_name("Henry");
  HelloReply reply;
  grpc::ClientContext context;

  auto status = stub->SayHello(&context, request, &reply);
  if (status.ok()) {
    std::cout << "Server replied: " << reply.message() << "\n";
  } else {
    std::cerr << "RPC failed\n";
  }
}


