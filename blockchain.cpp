#include "blockchain.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<Block> loadBlockchain(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) return {};

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



