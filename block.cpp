#include "block.hpp"
#include "crypto.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <openssl/sha.h>

Block::Block(int idx, const string& ts, const vector<Transaction>& txs, const string& prevHash)
    : index(idx), timestamp(ts), previousHash(prevHash), nonce(0), hash("") {
    
    transactions = txs;
    if (transactions.size() > MAX_TXS_PER_BLOCK)
        transactions.resize(MAX_TXS_PER_BLOCK);

    hash = calculateHash();  
}

string Block::calculateHash() const {
    std::ostringstream data;
    data << index << timestamp << previousHash << nonce;

    for (const auto& tx : transactions) {
        data << tx.fromPublicKeyHex << tx.toPublicKeyHex << tx.amount << tx.signatureHex;
    }

    return sha256(data.str());
}

void to_json(nlohmann::json& j, const Block& b) {
    j = nlohmann::json{
        {"index", b.index},
        {"timestamp", b.timestamp},
        {"transactions", b.transactions},
        {"previousHash", b.previousHash},
        {"nonce", b.nonce},
        {"hash", b.hash}
    };
}

void from_json(const nlohmann::json& j, Block& b) {
    j.at("index").get_to(b.index);
    j.at("timestamp").get_to(b.timestamp);
    j.at("transactions").get_to(b.transactions);
    j.at("previousHash").get_to(b.previousHash);
    j.at("nonce").get_to(b.nonce);
    j.at("hash").get_to(b.hash);
}
