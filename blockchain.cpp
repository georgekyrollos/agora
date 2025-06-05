#include "blockchain.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <fstream>        
#include <stdexcept>      


using json = nlohmann::json;
using std::ifstream;
using std::ofstream;
using std::runtime_error;


namespace {
    string getCurrentTimestamp() {
        time_t now = time(nullptr);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buf);
    }
}

vector<Block> loadBlockchain(const string& filename) {
    ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Blockchain file not found. Creating genesis block.\n";
        Block genesis(0, getCurrentTimestamp(), {}, "0");
        return {genesis};
    }

    json j;
    in >> j;
    in.close();

    vector<Block> chain;
    for (const auto& blockJson : j) {
        Block b(
            blockJson["index"],
            blockJson["timestamp"],
            blockJson["transactions"].get<vector<Transaction>>(),
            blockJson["previousHash"]
        );
        b.nonce = blockJson["nonce"];
        b.hash = blockJson["hash"];
        chain.push_back(b);
    }
    return chain;
}

void saveBlockchain(const vector<Block>& chain, const string& filename) {
    json j = chain;
    ofstream out(filename);
    out << j.dump(4);
    out.close();
}

const Block& getLastBlock(const vector<Block>& chain) {
    if (chain.empty()) throw runtime_error("Blockchain is empty.");
    return chain.back();
}

void appendBlock(vector<Block>& chain, const Block& newBlock, const string& filename) {
    chain.push_back(newBlock);
    saveBlockchain(chain, filename);
}

bool validateBlockchain(const vector<Block>& chain) {
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





