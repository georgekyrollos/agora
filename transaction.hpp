#pragma once
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;



struct Transaction {
    std::string fromPublicKeyHex;
    std::string toPublicKeyHex;
    double amount;
    std::string signatureHex;

    std::string serialize() const {
        return fromPublicKeyHex + "|" + toPublicKeyHex + "|" + std::to_string(amount);
    }
};

std::string buildTransactionMessage(const std::string& from, const std::string& to, int amount);

void to_json(json& j, const Transaction& tx);
void from_json(const json& j, Transaction& tx);

