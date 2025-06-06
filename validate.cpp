#include "validate.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <set>

using std::string;
using std::vector;
using std::set;


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./validate <blockchain.json>\n";
        return 1;
    }

    string filename = argv[1];
    vector<Block> chain = loadBlockchain(filename);

    if (chain.empty()) {
        std::cerr << "Blockchain file is empty or malformed.\n";
        return 1;
    }

    if (validateBlockchain(chain)) {
        std::cout << "Blockchain is VALID\n";
    } else {
        std::cout << "Blockchain is INVALID\n";
    }

    return 0;
}


bool validateTransaction(const Transaction& tx, const vector<Block>& chain) {
    if (tx.fromPublicKeyHex == META) {
        return tx.signatureHex == REWARD_SIG;
    }

    string msg = buildTransactionMessage(tx.fromPublicKeyHex, tx.toPublicKeyHex, tx.amount);
    if (!verifySignature(msg, tx.signatureHex, tx.fromPublicKeyHex)) return false;

    double balance = getBalance(tx.fromPublicKeyHex, chain);
    return balance >= tx.amount;
}

bool validateBlock(const Block& block, const Block& previousBlock, const vector<Block>& chainSoFar) {
    // if (block.previousHash != previousBlock.hash) return false;
    // if (block.hash != block.calculateHash()) return false;
    // if (block.hash.substr(0, DIFFICULTY) != string(DIFFICULTY, '0')) return false;
    if (block.previousHash != previousBlock.hash) {
        std::cout << "FAILED: previousHash mismatch\n";
        std::cout << "Expected: " << previousBlock.hash << "\n";
        std::cout << "Got:      " << block.previousHash << "\n";
        return false;
    }

    string recomputed = block.calculateHash();
    if (block.hash != recomputed) {
        std::cout << "FAILED: hash mismatch\n";
        std::cout << "Expected: " << recomputed << "\n";
        std::cout << "Got:      " << block.hash << "\n";
        return false;
    }

    if (block.hash.substr(0, DIFFICULTY) != string(DIFFICULTY, '0')) {
        std::cout << "FAILED: hash does not meet difficulty requirement\n";
        std::cout << "Hash:      " << block.hash << "\n";
        std::cout << "Expected prefix: " << string(DIFFICULTY, '0') << "\n";
        return false;
    }
   
    set<string> seenIds;
    bool seenCoinbase = false;

    for (size_t i = 0; i < block.transactions.size(); ++i) {
        const Transaction& tx = block.transactions[i];
        if (seenIds.count(tx.id)) return false;
        seenIds.insert(tx.id);
        if (tx.fromPublicKeyHex == META) {
            if (i != 0 || seenCoinbase || tx.signatureHex != REWARD_SIG) return false;
            seenCoinbase = true;
        }

        if (!validateTransaction(tx, chainSoFar)) return false;
    }
    
    return true;
}

bool validateBlockchain(const vector<Block>& chain) {
    if (chain.empty()) return false;
    for (size_t i = 1; i < chain.size(); ++i) {
        vector<Block> upToPrev(chain.begin(), chain.begin() + i);
        if (!validateBlock(chain[i], chain[i - 1], upToPrev)) return false;
    }
    return true;
}


