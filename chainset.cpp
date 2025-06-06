#include "chainset.hpp"
#include "validate.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;
using std::ifstream;
using std::ofstream;


bool ChainSet::hasMainChain() const {
    return !chains.empty() && mainIndex >= 0 && mainIndex < chains.size();
}

const vector<Block>& ChainSet::getMainChain() const {
    if (!hasMainChain()) {
        static const vector<Block> emptyChain;
        return emptyChain;
    }
    return chains[mainIndex];
}

vector<Block>& ChainSet::getMainChain() {
    if (!hasMainChain()) {
        static vector<Block> emptyChain;
        return emptyChain;
    }
    return chains[mainIndex];
}

void ChainSet::addNewChain(const vector<Block>& newChain) {
    chains.push_back(newChain);
}

void ChainSet::tryReplaceMainChain(const vector<Block>& newChain) {
    if (validateBlockchain(newChain) && newChain.size() > getMainChain().size()) {
        chains.push_back(newChain);
        mainIndex = chains.size() - 1;
    }
}

ChainSet loadChainSet(const string& filename) {
    ChainSet chainSet;

    ifstream in(filename);
    if (!in.is_open()) {
        return chainSet;  // Default empty set with mainIndex = 0
    }

    json j;
    try {
        in >> j;
    } catch (...) {
        in.close();
        return chainSet;  // Malformed JSON
    }
    in.close();

    if (j.contains("mainIndex") && j["mainIndex"].is_number_integer()) {
        chainSet.mainIndex = j["mainIndex"];
    }

    if (j.contains("chains") && j["chains"].is_array()) {
        for (const auto& chainJson : j["chains"]) {
            if (!chainJson.is_array()) continue;

            vector<Block> chain;
            for (const auto& blockJson : chainJson) {
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
            chainSet.chains.push_back(chain);
        }
    }

    return chainSet;
}




void saveAllChains(const vector<vector<Block>>& allChains, int mainIndex, const string& filename) {
    json j;
    j["mainIndex"] = mainIndex;
    j["chains"] = allChains;

    ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Failed to open blockchain file for writing.\n";
        return;
    }
    out << j.dump(4);
    out.close();
}

