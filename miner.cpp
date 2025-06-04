#include "block.hpp"
#include "blockchain.hpp"
#include "mempool.hpp"
#include "crypto.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>
#include <fstream>
#include <chrono>
#include <ctime>

using json = nlohmann::json;

const int DIFFICULTY = 4;           
const int MAX_TXS_PER_BLOCK = 5;    

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%FT%TZ", std::gmtime(&t));  // ISO 8601 format
    return std::string(buf);
}

int main() {
    // Load current blockchain
    Blockchain chain;
    chain.loadBlockchain("blockchain.json");

    // Load mempool
    std::vector<Transaction> mempool = readMempool();

    if (mempool.empty()) {
        std::cout << "No transactions to mine.\n";
        return 0;
    }

    
    int txCount = std::min((int)mempool.size(), MAX_TXS_PER_BLOCK);
    std::vector<Transaction> txsToInclude(mempool.begin(), mempool.begin() + txCount);

    Block newBlock(
        chain.getLastBlock().index + 1,
        getCurrentTimestamp(),
        txsToInclude,
        chain.getLastBlock().hash
    );

    std::cout << "Mining block...\n";
    mineBlock(newBlock, DIFFICULTY);
    std::cout << "Block mined: " << newBlock.hash << "\n";

    // Add block to chain and save
    chain.addBlock(newBlock);
    chain.saveToFile("blockchain.json");

    // Remove mined transactions from mempool and save
    mempool.erase(mempool.begin(), mempool.begin() + txCount);
    writeMempool(mempool);

    return 0;
}
