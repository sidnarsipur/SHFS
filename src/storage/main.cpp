#include "pch.h"
#include "storage_node.h"

int main(int argc, char** argv) {
    const std::string port = (argc > 1 ? argv[1] : "7000");
    const std::string storage_addr = "localhost:" + port;
    const std::string naming_addr  = "localhost:6000";

    StorageNode node(naming_addr, storage_addr);
    node.Register();
    node.Run();

    return 0;
}
