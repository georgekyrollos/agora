#pragma once
#include "block.hpp"
#include <vector>
#include <string>

using std::vector;
using std::string;

const int BLOCK_REWARD = 10;
const string MEMPOOL_FILE = "mempool.json";
const string BLOCKCHAINS_FILE = "blockchains.json";

struct ChainSet {
    vector<vector<Block>> chains;
    int mainIndex = 0;

    bool hasMainChain() const;
    const vector<Block>& getMainChain() const;
    vector<Block>& getMainChain();
    void addNewChain(const vector<Block>& newChain);
    void tryReplaceMainChain(const vector<Block>& newChain);
};

ChainSet loadChainSet(const string& filename);
void saveAllChains(const vector<vector<Block>>& chains, int mainIndex, const string& filename);
bool isTxInChainSet(const string& txID, const ChainSet& chainSet);
bool isTxInChainSetOrValidTxs(const string& txID, const ChainSet& chainSet, const vector<Transaction>& validTxs);
