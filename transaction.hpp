#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using std::string;
using std::vector;

struct Transaction {
    string fromPublicKeyHex;
    string toPublicKeyHex;
    double amount;
    string signatureHex;
    string ts;
    string id;

    string serialize() const {
        return fromPublicKeyHex + "|" + toPublicKeyHex + "|" + std::to_string(amount);
    }
};

string buildTransactionMessage(const string& from, const string& to, int amount);
string computeTransactionID(const Transaction& tx);

void to_json(json& j, const Transaction& tx);
void from_json(const json& j, Transaction& tx);

