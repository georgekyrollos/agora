#include "handlers.hpp"
#include "validate.hpp"
#include "mempool.hpp"
#include "transaction.hpp"
#include "block.hpp"
#include "blockchain.hpp"
#include "chainset.hpp"
#include "p2p.hpp"
#include "sync.hpp"
#include "message.hpp"
#include <iostream>

void handleMessage(const json& j)
{
    string type;
    j.at("type").get_to(type);

    if(type == TX_FLAG)
    {
        Transaction tx;
        from_json(j.at("transaction"), tx);
        handleTransaction(tx);
    }
    else if(type == BK_FLAG)
    {
        Block bk(j.at("block").at("index"), j.at("block").at("timestamp"), j.at("block").at("transactions").get<vector<Transaction>>(), j.at("block").at("previousHash"));
        from_json(j.at("block"), bk);
        handleBlock(bk);
    }
    // add more types as needed
}

void handleTransaction(Transaction tx) 
{
    ChainSet chainSet = loadChainSet(BLOCKCHAINS_FILE);
    // transactions need only be valid in any fork to be added to mempool (overly generous will filter out later in pipeline)
    for (const auto& chain : chainSet.chains) {   
        if (validateTransaction(tx, chain)) {
            appendToMempool(tx);
            std::cout << "Transaction added to mempool\n";
            return;
        }
    }
    std::cout << "Invalid transaction\n";
    
}

// void handleBlock(Block bk) {
//     std::lock_guard<std::mutex> lock(chainsetMutex);  // Automatically unlocks when scope ends

//     ChainSet chainSet = loadChainSet(BLOCKCHAINS_FILE);
//     bool added = false;

//     // Try appending to existing chains
//     for (auto& chain : chainSet.chains) {
//         if (validateBlock(bk, chain.back(), chain)) {
//             chain.push_back(bk);
//             added = true;
//             break;
//         }
//     }

//     // If not appended, create a fork
//     if (!added) {
//         for (const auto& chain : chainSet.chains) {
//             if (!chain.empty() && chain.back().hash == bk.previousHash) {
//                 vector<Block> newFork = chain;
//                 newFork.push_back(bk);
//                 chainSet.chains.push_back(newFork);
//                 added = true;
//                 break;
//             }
//         }
//     }

//     if (added) {
//         chainSet.tryReplaceMainChain(chainSet.chains.back());
//         saveAllChains(chainSet.chains, chainSet.mainIndex, BLOCKCHAINS_FILE);
//         std::cout << "Block added. Chain count: " << chainSet.chains.size() << "\n";
//     } else {
//         std::cout << "Invalid block. Not added.\n";
//     }
// }

void handleBlock(Block bk) {
    std::lock_guard<std::mutex> lock(chainsetMutex);
    ChainSet chainSet = loadChainSet(BLOCKCHAINS_FILE);
    bool addedToExisting = false;

    if (chainSet.chains.empty() && bk.index == 0 && bk.previousHash == "0") {
        if (validateBlock(bk, bk, {})) {  
            chainSet.chains.push_back({bk});
            chainSet.mainIndex = 0;
            saveAllChains(chainSet.chains, chainSet.mainIndex, BLOCKCHAINS_FILE);
            std::cout << "Genesis block accepted.\n";
            return;
        } else {
            std::cout << "Invalid genesis block.\n";
            return;
        }
    }


    for (auto& chain : chainSet.chains) {
        if (chain.back().hash == bk.previousHash) {
            if (validateBlock(bk, chain.back(), chain)) {
                chain.push_back(bk);
                addedToExisting = true;
                chainSet.tryReplaceMainChain(chain);
                saveAllChains(chainSet.chains, chainSet.mainIndex, BLOCKCHAINS_FILE);
                std::cout << "Block added to chain (extended)\n";
                return;
            }
        }
    }

    // Try to create a new fork if it attaches to a past block
    for (const auto& chain : chainSet.chains) {
        for (size_t i = 0; i < chain.size(); ++i) {
            if (chain[i].hash == bk.previousHash) {
                vector<Block> newFork(chain.begin(), chain.begin() + i + 1);
                if (validateBlock(bk, newFork.back(), newFork)) {
                    newFork.push_back(bk);
                    chainSet.chains.push_back(newFork);
                    chainSet.tryReplaceMainChain(newFork);
                    saveAllChains(chainSet.chains, chainSet.mainIndex, BLOCKCHAINS_FILE);
                    std::cout << "Block added. Chain count: " << chainSet.chains.size() << "\n";
                    return;
                }
            }
        }
    }

    std::cout << "Invalid block. Not added.\n";
}



