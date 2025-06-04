#include "blockchain.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <ctime>

using json = nlohmann::json;

namespace {
    std::string getCurrentTimestamp() {
        std::time_t now = std::time(nullptr);
        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return std::string(buf);
    }
}

std::vector<Block> loadBlockchain(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Blockchain file not found. Creating genesis block.\n";
        Block genesis(0, getCurrentTimestamp(), {}, "0");
        return {genesis};
    }

    json j;
    in >> j;
    in.close();

    std::vector<Block> chain;
    for (const auto& blockJson : j) {
        Block b(
            blockJson["index"],
            blockJson["timestamp"],
            blockJson["transactions"].get<std::vector<Transaction>>(),
            blockJson["previousHash"]
        );
        b.nonce = blockJson["nonce"];
        b.hash = blockJson["hash"];
        chain.push_back(b);
    }
    return chain;
}

void saveBlockchain(const std::vector<Block>& chain, const std::string& filename) {
    json j = chain;
    std::ofstream out(filename);
    out << j.dump(4);
    out.close();
}

const Block& getLastBlock(const std::vector<Block>& chain) {
    if (chain.empty()) throw std::runtime_error("Blockchain is empty.");
    return chain.back();
}

void appendBlock(std::vector<Block>& chain, const Block& newBlock, const std::string& filename) {
    chain.push_back(newBlock);
    saveBlockchain(chain, filename);
}

bool validateBlockchain(const std::vector<Block>& chain) {
    for (size_t i = 1; i < chain.size(); ++i) {
        const Block& prev = chain[i - 1];
        const Block& curr = chain[i];

        // Check previous hash linkage
        if (curr.previousHash != prev.hash) {
            std::cerr << "Invalid previous hash at block " << curr.index << "\n";
            return false;
        }

        // Recalculate hash and check it matches stored value
        if (curr.calculateHash() != curr.hash) {
            std::cerr << "Invalid hash at block " << curr.index << "\n";
            return false;
        }
    }
    return true;
}





