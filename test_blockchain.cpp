#include "blockchain.hpp"
#include <iostream>

int main() {
    std::string path = "blockchain.json";  // or whatever your file is

    std::vector<Block> chain = loadBlockchain(path);

    for (const Block& block : chain) {
        std::cout << "Block #" << block.index << "\n";
        std::cout << "Timestamp: " << block.timestamp << "\n";
        std::cout << "Previous Hash: " << block.previousHash << "\n";
        std::cout << "Hash: " << block.hash << "\n";
        std::cout << "Nonce: " << block.nonce << "\n";
        std::cout << "Transactions:\n";
        for (const Transaction& tx : block.transactions) {
            std::cout << "  From: " << tx.fromPublicKeyHex
                      << " To: " << tx.toPublicKeyHex
                      << " Amount: " << tx.amount << "\n";
        }
        std::cout << "----------------------------\n";
    }

    return 0;
}

