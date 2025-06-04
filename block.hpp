#pragma once
#include <string>
#include <vector>
#include "transaction.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;


const int MAX_TXS_PER_BLOCK = 5; 

struct Block {
    int index;
    std::string timestamp;
    std::vector<Transaction> transactions;
    std::string previousHash;
    int nonce;
    std::string hash;

    Block(int idx, const std::string& ts, const std::vector<Transaction>& txs, const std::string& prevHash);

    std::string calculateHash() const;
};


void to_json(json& j, const Block& b);
void from_json(const json& j, Block& b);
