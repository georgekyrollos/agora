#include "validate.hpp"
#include "chainset.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>

using std::string;
using std::vector;
using std::set;
using std::map;


bool validateTransaction(const Transaction& tx, const vector<Block>& chain) {
    // Recompute and verify the stored transaction ID
    if (tx.id != computeTransactionID(tx)) return false;

    if (tx.fromPublicKeyHex == META) {
        // Coinbase: must have the sentinel signature and exact block reward
        return tx.signatureHex == REWARD_SIG && tx.amount == BLOCK_REWARD;
    }

    // Normal transaction: amount must be strictly positive
    if (tx.amount <= 0) return false;

    string msg = buildTransactionMessage(tx.fromPublicKeyHex, tx.toPublicKeyHex, tx.amount);
    if (!verifySignature(msg, tx.signatureHex, tx.fromPublicKeyHex)) return false;

    int64_t balance = getBalance(tx.fromPublicKeyHex, chain);
    return balance >= tx.amount;
}

bool validateBlock(const Block& block, const Block& previousBlock, const vector<Block>& chainSoFar) {
    // ── Genesis block ──────────────────────────────────────────────────────────
    if (block.index == 0) {
        if (block.previousHash != "0") return false;
        if (block.hash != block.calculateHash()) return false;
        if (block.hash.substr(0, DIFFICULTY) != string(DIFFICULTY, '0')) return false;

        if (block.transactions.size() != 1) return false;
        const Transaction& cb = block.transactions[0];
        if (cb.fromPublicKeyHex != META) return false;
        if (cb.signatureHex != REWARD_SIG) return false;
        if (cb.amount != BLOCK_REWARD) return false;
        if (cb.id != computeTransactionID(cb)) return false;
        return true;
    }

    // ── Non-genesis block ──────────────────────────────────────────────────────
    if (block.index != previousBlock.index + 1) return false;
    if (block.previousHash != previousBlock.hash) return false;
    if (block.hash != block.calculateHash()) return false;
    if (block.hash.substr(0, DIFFICULTY) != string(DIFFICULTY, '0')) return false;

    set<string> seenIds;
    bool seenCoinbase = false;
    // Track cumulative amount spent per sender within this block to catch
    // double-spends that individually look valid against chainSoFar balance.
    map<string, int64_t> blockSpends;

    for (size_t i = 0; i < block.transactions.size(); ++i) {
        const Transaction& tx = block.transactions[i];

        // Reject duplicate tx IDs within the block
        if (seenIds.count(tx.id)) return false;
        seenIds.insert(tx.id);

        if (tx.fromPublicKeyHex == META) {
            // Coinbase must be exactly the first transaction, appear only once,
            // carry the sentinel signature, and award exactly BLOCK_REWARD.
            if (i != 0 || seenCoinbase) return false;
            if (tx.signatureHex != REWARD_SIG) return false;
            if (tx.amount != BLOCK_REWARD) return false;
            if (tx.id != computeTransactionID(tx)) return false;
            seenCoinbase = true;
        } else {
            // Normal transaction checks
            if (tx.amount <= 0) return false;
            if (tx.id != computeTransactionID(tx)) return false;

            string msg = buildTransactionMessage(tx.fromPublicKeyHex, tx.toPublicKeyHex, tx.amount);
            if (!verifySignature(msg, tx.signatureHex, tx.fromPublicKeyHex)) return false;

            // Balance check accounting for other spends already included in this block
            int64_t chainBalance = getBalance(tx.fromPublicKeyHex, chainSoFar);
            if (chainBalance - blockSpends[tx.fromPublicKeyHex] < tx.amount) return false;
            blockSpends[tx.fromPublicKeyHex] += tx.amount;
        }
    }

    // Every mined block must contain exactly one coinbase as the first transaction
    if (!seenCoinbase) return false;

    return true;
}

bool validateBlockchainOLD(const vector<Block>& chain) {
    if (chain.empty()) return false;
    for (size_t i = 1; i < chain.size(); ++i) {
        vector<Block> upToPrev(chain.begin(), chain.begin() + i);
        if (!validateBlock(chain[i], chain[i - 1], upToPrev)) return false;
    }
    return true;
}

bool validateBlockchain(const vector<Block>& chain) {
    if (chain.empty()) return false;

    // Validate genesis block explicitly
    if (!validateBlock(chain[0], chain[0], {})) {
        std::cerr << "Genesis block failed validation.\n";
        return false;
    }

    set<string> globalSeenTxIDs;
    for (const auto& tx : chain[0].transactions)
        globalSeenTxIDs.insert(tx.id);

    for (size_t i = 1; i < chain.size(); ++i) {
        const Block& block = chain[i];

        // Check for duplicate transactions across the entire chain
        for (const auto& tx : block.transactions) {
            if (globalSeenTxIDs.count(tx.id)) {
                std::cerr << "Duplicate transaction ID detected: " << tx.id << "\n";
                return false;
            }
            globalSeenTxIDs.insert(tx.id);
        }

        vector<Block> upToPrev(chain.begin(), chain.begin() + i);
        if (!validateBlock(block, chain[i - 1], upToPrev)) {
            std::cerr << "Block " << i << " failed validation.\n";
            return false;
        }
    }

    return true;
}
