#pragma once
#include "blockchain.hpp"
#include "transaction.hpp"
#include "crypto.hpp"
#include "block.hpp"

bool validateTransaction(const Transaction& tx, const vector<Block>& chain);
bool validateBlock(const Block& block, const Block& previousBlock, const vector<Block>& chain);
bool validateBlockchain(const vector<Block>& chain);
bool validateBlockchainOLD(const vector<Block>& chain);
