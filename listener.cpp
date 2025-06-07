#include "p2p.hpp"
#include "handlers.hpp" // assumes handleMessage(json j) is defined here
#include "validate.hpp"
#include "mempool.hpp"
#include "transaction.hpp"
#include "block.hpp"
#include "blockchain.hpp"
#include "chainset.hpp"
#include "crypto.hpp"
#include "wallet.hpp"

#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;
using namespace std;

void processIncomingMessage(const string& message) {
    try {
        std::cout << "Received message: " << message << "\n";
        json j = json::parse(message);
        handleMessage(j);
    } catch (const std::exception& e) {
        cerr << "[Error] Failed to parse incoming message as JSON: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: ./listener <port>\n";
        return 1;
    }

    int port = stoi(argv[1]);
    listenOnPort(port, processIncomingMessage);
    return 0; // should never be reached
}
