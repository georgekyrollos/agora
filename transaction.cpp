#include "transaction.hpp"
#include "crypto.hpp"
#include <nlohmann/json.hpp>

#include <iostream>

using json = nlohmann::json;

string computeTransactionID(const Transaction& tx) {
    std::ostringstream oss;
    oss << tx.fromPublicKeyHex
        << tx.toPublicKeyHex
        << tx.amount
        << tx.ts;  
    return sha256(oss.str());
}


string buildTransactionMessage(const string& from, const string& to, int amount) {
    return from + "|" + to + "|" + std::to_string(amount); 
}

void to_json(json& j, const Transaction& tx) {
    j = json{{"id", tx.id}, {"sender", tx.fromPublicKeyHex}, {"receiver", tx.toPublicKeyHex}, {"amount", tx.amount}, {"signature", tx.signatureHex}, {"timestamp", tx.ts}};
}

void from_json(const json& j, Transaction& tx) {
    j.at("id").get_to(tx.id);
    j.at("sender").get_to(tx.fromPublicKeyHex);
    j.at("receiver").get_to(tx.toPublicKeyHex);
    j.at("amount").get_to(tx.amount);
    j.at("signature").get_to(tx.signatureHex);
    j.at("timestamp").get_to(tx.ts);
}
