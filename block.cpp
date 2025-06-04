#include "block.hpp"
#include "crypto.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

// Utility: SHA256 hash function
static std::string sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)input.c_str(), input.size(), hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return oss.str();
}

// Block constructor
Block::Block(int idx, const std::string& ts, const std::vector<Transaction>& txs, const std::string& prevHash)
    : index(idx), timestamp(ts), previousHash(prevHash), nonce(0), hash("") {
    
    int count = std::min((int)txs.size(), MAX_TXS_PER_BLOCK);
    transactions.assign(txs.begin(), txs.begin() + count);

    hash = calculateHash();  
}

// calculates the block's hash
std::string Block::calculateHash() const {
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


