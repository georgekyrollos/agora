#include "transaction.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string buildTransactionMessage(const std::string& from, const std::string& to, int amount) {
    return from + "|" + to + "|" + std::to_string(amount); 
}

void to_json(json& j, const Transaction& tx) {
    j = json{{"sender", tx.fromPublicKeyHex}, {"receiver", tx.toPublicKeyHex}, {"amount", tx.amount}, {"signature", tx.signatureHex}};
}

void from_json(const json& j, Transaction& tx) {
    j.at("sender").get_to(tx.fromPublicKeyHex);
    j.at("receiver").get_to(tx.toPublicKeyHex);
    j.at("amount").get_to(tx.amount);
    j.at("signature").get_to(tx.signatureHex);
}
