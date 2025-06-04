#pragma once
#include "block.hpp"
#include <vector>

std::vector<Block> loadBlockchain(const std::string& filename = "blockchain.json");
void saveBlockchain(const std::vector<Block>& chain, const std::string& filename = "blockchain.json");
const Block& getLastBlock(const std::vector<Block>& chain);
void appendBlock(std::vector<Block>& chain, const Block& newBlock, const std::string& filename = "blockchain.json");
bool validateBlockchain(const std::vector<Block>& chain);



