#pragma once
#include <string>
#include <vector>
#include "transaction.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using std::string;
using std::vector;



const int MAX_TXS_PER_BLOCK = 5; 
const int DIFFICULTY = 6;
const string REWARD_SIG = "reward";
const string META = "COINBASE";

struct Block {
    int index;
    string timestamp;
    vector<Transaction> transactions;
    string previousHash;
    int nonce;
    string hash;

    Block(int idx, const string& ts, const vector<Transaction>& txs, const string& prevHash);

    string calculateHash() const;
};


void to_json(json& j, const Block& b);
void from_json(const json& j, Block& b);
