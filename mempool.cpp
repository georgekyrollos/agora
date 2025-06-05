// mempool.cpp
#include "mempool.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void appendToMempool(const Transaction& tx) {
    std::ifstream inFile("mempool.json");
    json mempool;

    if (inFile) {
        inFile >> mempool;
        inFile.close();
    } else {
        mempool = json::array();
    }

    json txJson = {
        {"sender", tx.fromPublicKeyHex},
        {"receiver", tx.toPublicKeyHex},
        {"amount", tx.amount},
        {"signature", tx.signatureHex},
    };

    mempool.push_back(txJson);

    std::ofstream outFile("mempool.json");
    outFile << mempool.dump(4);
}

std::vector<Transaction> readMempool() {
    std::ifstream file("mempool.json");
    std::vector<Transaction> mempool;

    if (file) {
        json j;
        file >> j;
        mempool = j.get<std::vector<Transaction>>();
    }

    return mempool;
}

void printMempool() {
    std::ifstream file("mempool.json");
    if (!file) {
        std::cout << "No mempool found.\n";
        return;
    }

    json mempool;
    file >> mempool;

    std::cout << "Current mempool:\n";
    for (const auto& tx : mempool) {
        std::cout << "Sender: " << tx["sender"] << "\n";
        std::cout << "Receiver: " << tx["receiver"] << "\n";
        std::cout << "Amount: " << tx["amount"] << "\n";
        std::cout << "Signature: " << tx["signature"] << "\n";
        std::cout << "--------------------------\n";
    }
}

