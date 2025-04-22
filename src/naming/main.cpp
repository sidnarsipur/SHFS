#include "pch.h"
#include "naming_node.h"

int main(int argc, char **argv) {
    std::string port = (argc > 1 ? argv[1] : "6000");
    std::string naming_addr = "localhost:" + port;

    NamingNode node(naming_addr);
    node.Run();
    return 0;
}
