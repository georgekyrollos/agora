#include "block.hpp"
#include "blockchain.hpp"
#include "transaction.hpp"
#include "crypto.hpp"
#include "wallet.hpp"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>
#include <ctime>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

const int DIFFICULTY = 4;
const int BLOCK_REWARD = 10;
const std::string MEMPOOL_FILE = "mempool.json";
const std::string BLOCKCHAIN_FILE = "blockchain.json";

// Timestamp utility
std::string currentTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[100];
    strftime(buf, sizeof(buf), "%FT%TZ", std::gmtime(&now));
    return std::string(buf);
}

// Calculate hash using a candidate nonce
std::string calculateHash(const Block& block, uint64_t nonce) {
    std::ostringstream oss;
    oss << block.index << block.timestamp;
    for (const auto& tx : block.transactions) {
        oss << tx.fromPublicKeyHex << tx.toPublicKeyHex << tx.amount;
    }
    oss << block.previousHash << nonce;
    return sha256(oss.str());
}

// Mine the block: updates nonce and hash
void mineBlock(Block& block, int difficulty) {
    std::string prefix(difficulty, '0');
    uint64_t nonce = 0;
    std::string hash;

    do {
        hash = calculateHash(block, nonce);
        nonce++;
    } while (hash.substr(0, difficulty) != prefix);

    block.nonce = nonce - 1;
    block.hash = hash;
}

// Load mempool
std::vector<Transaction> loadMempool() {
    std::ifstream in(MEMPOOL_FILE);
    if (!in.is_open()) return {};
    json j;
    in >> j;
    return j.get<std::vector<Transaction>>();
}

// Save mempool
void saveMempool(const std::vector<Transaction>& mempool) {
    std::ofstream out(MEMPOOL_FILE);
    json j = mempool;
    out << j.dump(4);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./miner <wallet.dat>\n";
        return 1;
    }

    Wallet miner;
    if (!miner.loadFromFile(argv[1])) {
        std::cerr << "Failed to load miner wallet from " << argv[1] << "\n";
        return 1;
    }

    std::vector<Block> chain = loadBlockchain(BLOCKCHAIN_FILE);

    if (chain.empty()) {
        std::cout << "Creating genesis block...\n";
    
        // Create a valid coinbase transaction
        Transaction genesisReward;
        genesisReward.fromPublicKeyHex = "COINBASE";
        genesisReward.toPublicKeyHex = miner.publicKeyHex;
        genesisReward.amount = BLOCK_REWARD;
        genesisReward.signatureHex = "reward";
    
        std::vector<Transaction> genesisTxs;
        genesisTxs.push_back(genesisReward);  // Avoid brace-init just in case of C++17 edge issues
    
        Block genesisBlock(
            0,
            currentTimestamp(),
            genesisTxs,
            "0"  // no previous hash
        );
    
        mineBlock(genesisBlock, DIFFICULTY);
        std::cout << "Genesis block mined: " << genesisBlock.hash << "\n";
    
        chain.push_back(genesisBlock);
        saveBlockchain(chain, BLOCKCHAIN_FILE);
    }

    std::vector<Transaction> mempool = loadMempool();

    if (mempool.empty()) {
        std::cout << "Mempool is empty.\n";
        return 0;
    }

    const Block& lastBlock = chain.back();
    std::vector<Transaction> validTxs;

    for (const auto& tx : mempool) {
        if (tx.fromPublicKeyHex == "COINBASE") continue;  // ignore malformed coinbases
        std::string msg = buildTransactionMessage(tx.fromPublicKeyHex, tx.toPublicKeyHex, tx.amount);
        double senderBalance = getEffectiveBalanceDuringMining(tx.fromPublicKeyHex, chain, validTxs);   // here 
        if (verifySignature(msg, tx.signatureHex, tx.fromPublicKeyHex)) {
            if (senderBalance < tx.amount) {
                std::cout << "Skipping transaction from " << tx.fromPublicKeyHex << " due to insufficient balance\n";
                continue;
            }
            validTxs.push_back(tx);
            if (validTxs.size() >= MAX_TXS_PER_BLOCK-1) break;
        } else {
            std::cout << "Skipping invalid transaction from " << tx.fromPublicKeyHex << " due to signature verification failure\n";
        }
    }

    if (validTxs.empty()) {
        std::cout << "No valid transactions to include in the block.\n";
        return 0;
    }

    // Add block reward as the first transaction
    Transaction rewardTx = {
        "COINBASE",
        miner.publicKeyHex,
        BLOCK_REWARD,
        "reward"  // placeholder signature
    };

    std::vector<Transaction> blockTxs = { rewardTx };
    blockTxs.insert(blockTxs.end(), validTxs.begin(), validTxs.end());

    Block newBlock(
        lastBlock.index + 1,
        currentTimestamp(),
        blockTxs,
        lastBlock.hash
    );

    std::cout << "Mining block #" << newBlock.index << "...\n";
    mineBlock(newBlock, DIFFICULTY);
    std::cout << "Block mined: " << newBlock.hash << "\n";

    chain.push_back(newBlock);
    saveBlockchain(chain, BLOCKCHAIN_FILE);

    // Remove included transactions from mempool
    for (const auto& tx : validTxs) {
        auto it = std::find_if(mempool.begin(), mempool.end(), [&](const Transaction& mtx) {
            return tx.signatureHex == mtx.signatureHex;
        });
        if (it != mempool.end()) mempool.erase(it);
    }

    saveMempool(mempool);
    return 0;
}

