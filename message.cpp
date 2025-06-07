#include "message.hpp"

string formatTransactionMsg(Transaction tx) {
    json j;
    j["type"] = "TX";
    j["transaction"] = {
        {"amount", tx.amount},
        {"id", tx.id},
        {"receiver", tx.toPublicKeyHex},
        {"sender", tx.fromPublicKeyHex},
        {"signature", tx.signatureHex},
        {"timestamp", tx.ts}
    };
    return j.dump(4);
}

string formatBlockMsg(Block bk) {
    json j;
    j["type"] = "BK";
    j["block"] = {
        {"hash", bk.hash},
        {"index", bk.index},
        {"nonce", bk.nonce},
        {"previousHash", bk.previousHash},
        {"timestamp", bk.timestamp},
        {"transactions", bk.transactions}
    };
    return j.dump(4);
}
