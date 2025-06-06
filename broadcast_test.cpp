#include "p2p.hpp"
#include <iostream>

int main() {
    listenOnPort(8080, [](const string& msg) {
        std::cout << "Received: " << msg << std::endl;
    });
}
