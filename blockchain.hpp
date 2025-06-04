#pragma once
#include "block.hpp"
#include <vector>

std::vector<Block> loadBlockchain(const std::string& filename = "blockchain.json");
void saveBlockchain(const std::vector<Block>& chain, const std::string& filename = "blockchain.json");
const Block& getLastBlock() const;

