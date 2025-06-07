#include "block.hpp"
#include "blockchain.hpp"
#include "chainset.hpp"
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
#include <unordered_set>

using json = nlohmann::json;
using std::string;
using std::vector;

// const int BLOCK_REWARD = 10;
// const string MEMPOOL_FILE = "mempool.json";
// const string BLOCKCHAINS_FILE = "blockchains.json";

// Timestamp utility
namespace {
    string getCurrentTimestamp() {
        time_t now = time(nullptr);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buf);
    }
}

// Calculate hash using a candidate nonce
string calculateHash(const Block& block, uint64_t nonce) {
    std::ostringstream oss;
    oss << block.index << block.timestamp << block.previousHash << nonce;
    for (const auto& tx : block.transactions) {
        oss << tx.fromPublicKeyHex << tx.toPublicKeyHex << tx.amount << tx.signatureHex;
    }
    return sha256(oss.str());
}

// Mine the block: updates nonce and hash
void mineBlock(Block& block, int difficulty) {
    string prefix(difficulty, '0');
    uint64_t nonce = 0;
    string hash;

    do {
        hash = calculateHash(block, nonce);
        nonce++;
    } while (hash.substr(0, difficulty) != prefix);

    block.nonce = nonce - 1;
    block.hash = hash;
}

// Load mempool
vector<Transaction> loadMempool() {
    std::ifstream in(MEMPOOL_FILE);
    if (!in.is_open()) return {};
    json j;
    in >> j;
    return j.get<vector<Transaction>>();
}

// Save mempool
void saveMempool(const vector<Transaction>& mempool) {
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

    ChainSet chainSet;
    chainSet = loadChainSet(BLOCKCHAINS_FILE);
    
    if (chainSet.getMainChain().empty()) {
        std::cout << "Creating genesis block...\n";
    
        // Create a valid coinbase transaction
        Transaction genesisReward;
        genesisReward.fromPublicKeyHex = META;
        genesisReward.toPublicKeyHex = miner.publicKeyHex;
        genesisReward.amount = BLOCK_REWARD;
        genesisReward.signatureHex = REWARD_SIG;
        genesisReward.ts = getCurrentTimestamp();
        genesisReward.id = computeTransactionID(genesisReward);
    
        vector<Transaction> genesisTxs;
        genesisTxs.push_back(genesisReward);  // Avoid brace-init just in case of C++17 edge issues
    
        Block genesisBlock(
            0,
            getCurrentTimestamp(),
            genesisTxs,
            "0"  // no previous hash
        );
    
        mineBlock(genesisBlock, DIFFICULTY);
        std::cout << "Genesis block mined: " << genesisBlock.hash << "\n";
    
        chainSet.addNewChain({genesisBlock});
        saveAllChains(chainSet.chains, 0, BLOCKCHAINS_FILE);
    }

    vector<Transaction> mempool = loadMempool();
    // vector<Block> chain = chainSet.getMainChain();

    if (mempool.empty()) {
        std::cout << "Mempool is empty.\n";
        return 0;
    }

    const Block& lastBlock = chainSet.chains[chainSet.mainIndex].back();
    vector<Transaction> validTxs;

    // for (const auto& tx : mempool) {
    //     if (tx.fromPublicKeyHex == "COINBASE") continue;  // ignore malformed coinbases
    //     string msg = buildTransactionMessage(tx.fromPublicKeyHex, tx.toPublicKeyHex, tx.amount);
    //     double senderBalance = getEffectiveBalanceDuringMining(tx.fromPublicKeyHex, chainSet.chains[chainSet.mainIndex], validTxs);   // here 
    //     if (verifySignature(msg, tx.signatureHex, tx.fromPublicKeyHex)) {
    //         if (senderBalance < tx.amount) {
    //             std::cout << "Skipping transaction from " << tx.fromPublicKeyHex << " due to insufficient balance\n";
    //             continue;
    //         }
    //         if (isTxInChainSetOrValidTxs(tx.id, chainSet, validTxs)) {
    //             std::cout << "Skipping transaction from " << tx.fromPublicKeyHex << " due to duplicate\n";
    //             continue;
    //         }
    //         validTxs.push_back(tx);
    //         if (validTxs.size() >= MAX_TXS_PER_BLOCK-1) break;
    //     } else {
    //         std::cout << "Skipping invalid transaction from " << tx.fromPublicKeyHex << " due to signature verification failure\n";
    //     }
    // }

    std::unordered_set<string> seenTxIDs;
    for (const auto& tx : mempool) {
        if (tx.fromPublicKeyHex == "COINBASE") continue;

        if (seenTxIDs.count(tx.id) || isTxInChainSet(tx.id, chainSet)) {
            std::cout << "Skipping duplicate transaction " << tx.id << "\n";
            continue;
        }

        string msg = buildTransactionMessage(tx.fromPublicKeyHex, tx.toPublicKeyHex, tx.amount);
        double senderBalance = getEffectiveBalanceDuringMining(tx.fromPublicKeyHex, chainSet.chains[chainSet.mainIndex], validTxs);

        if (!verifySignature(msg, tx.signatureHex, tx.fromPublicKeyHex)) {
            std::cout << "Skipping invalid transaction from " << tx.fromPublicKeyHex << "\n";
            continue;
        }

        if (senderBalance < tx.amount) {
            std::cout << "Skipping transaction from " << tx.fromPublicKeyHex << " due to insufficient balance\n";
            continue;
        }

        validTxs.push_back(tx);
        seenTxIDs.insert(tx.id);

        if (validTxs.size() >= MAX_TXS_PER_BLOCK - 1) break;
    }

        if (validTxs.empty()) {
            std::cout << "No valid transactions to include in the block.\n";
            return 0;
        }

    // Add block reward as the first transaction
    Transaction rewardTx;
    rewardTx.fromPublicKeyHex = META;
    rewardTx.toPublicKeyHex = miner.publicKeyHex;
    rewardTx.amount = BLOCK_REWARD;
    rewardTx.signatureHex = REWARD_SIG;
    rewardTx.ts = getCurrentTimestamp();
    rewardTx.id = computeTransactionID(rewardTx);

    vector<Transaction> blockTxs = { rewardTx };
    blockTxs.insert(blockTxs.end(), validTxs.begin(), validTxs.end());

    Block newBlock(
        lastBlock.index + 1,
        getCurrentTimestamp(),
        blockTxs,
        lastBlock.hash
    );

    std::cout << "Mining block #" << newBlock.index << "...\n";
    mineBlock(newBlock, DIFFICULTY);
    std::cout << "Block mined: " << newBlock.hash << "\n";

    chainSet.chains[chainSet.mainIndex].push_back(newBlock);
    chainSet.tryReplaceMainChain(chainSet.chains[chainSet.mainIndex]);
    saveAllChains(chainSet.chains, chainSet.mainIndex, BLOCKCHAINS_FILE);

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

