// Validation unit tests — run with: make test
#include "validate.hpp"
#include "wallet.hpp"
#include "crypto.hpp"
#include "block.hpp"
#include "transaction.hpp"
#include "chainset.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <ctime>

static int passed = 0;
static int failed = 0;

#define CHECK(name, expr) \
    do { \
        if (expr) { std::cout << "  PASS: " << name << "\n"; ++passed; } \
        else      { std::cout << "  FAIL: " << name << "\n"; ++failed; } \
    } while (0)

static std::string ts() {
    time_t now = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return buf;
}

static void mineBlock(Block& block) {
    std::string prefix(DIFFICULTY, '0');
    for (int nonce = 0; ; ++nonce) {
        block.nonce = nonce;
        block.hash = block.calculateHash();
        if (block.hash.substr(0, DIFFICULTY) == prefix) break;
    }
}

static Block makeGenesisBlock(const std::string& minerPub) {
    Transaction cb;
    cb.fromPublicKeyHex = META;
    cb.toPublicKeyHex = minerPub;
    cb.amount = BLOCK_REWARD;
    cb.signatureHex = REWARD_SIG;
    cb.ts = ts();
    cb.id = computeTransactionID(cb);

    Block b(0, ts(), {cb}, "0");
    mineBlock(b);
    return b;
}

static Transaction makeCoinbase(const std::string& minerPub) {
    Transaction cb;
    cb.fromPublicKeyHex = META;
    cb.toPublicKeyHex = minerPub;
    cb.amount = BLOCK_REWARD;
    cb.signatureHex = REWARD_SIG;
    cb.ts = ts();
    cb.id = computeTransactionID(cb);
    return cb;
}

static Transaction makeSignedTx(const Wallet& from, const std::string& toPub, int64_t amount) {
    Transaction tx;
    tx.fromPublicKeyHex = from.publicKeyHex;
    tx.toPublicKeyHex = toPub;
    tx.amount = amount;
    tx.ts = ts();
    std::string msg = buildTransactionMessage(from.publicKeyHex, toPub, amount);
    tx.signatureHex = signMessage(msg, from.privateKeyHex);
    tx.id = computeTransactionID(tx);
    return tx;
}

// ── Test cases ────────────────────────────────────────────────────────────────

static void testGenesisValidation() {
    std::cout << "\n[Genesis validation]\n";
    Wallet w; w.generateKeys();

    Block good = makeGenesisBlock(w.publicKeyHex);
    CHECK("valid genesis accepted", validateBlock(good, good, {}));

    // Wrong previousHash
    Block bad1 = good;
    bad1.previousHash = "abc";
    bad1.hash = bad1.calculateHash();
    CHECK("genesis with non-zero prevHash rejected", !validateBlock(bad1, bad1, {}));

    // Coinbase with wrong reward
    Transaction badCb = makeCoinbase(w.publicKeyHex);
    badCb.amount = BLOCK_REWARD + 1;
    badCb.id = computeTransactionID(badCb);
    Block bad2(0, ts(), {badCb}, "0");
    mineBlock(bad2);
    CHECK("genesis coinbase wrong reward rejected", !validateBlock(bad2, bad2, {}));

    // No transactions
    Block bad3(0, ts(), {}, "0");
    mineBlock(bad3);
    CHECK("genesis with no transactions rejected", !validateBlock(bad3, bad3, {}));

    // validateBlockchain must validate genesis
    std::vector<Block> chain1 = {good};
    CHECK("single-genesis chain passes validateBlockchain", validateBlockchain(chain1));

    Block badGenesis(0, ts(), {}, "0");
    mineBlock(badGenesis);
    std::vector<Block> chain2 = {badGenesis};
    CHECK("chain with invalid genesis rejected by validateBlockchain",
          !validateBlockchain(chain2));
}

static void testRejectNonPositiveAmounts() {
    std::cout << "\n[Non-positive amounts]\n";
    Wallet w; w.generateKeys();
    Block genesis = makeGenesisBlock(w.publicKeyHex);
    std::vector<Block> chain = {genesis};

    Transaction zeroTx;
    zeroTx.fromPublicKeyHex = w.publicKeyHex;
    zeroTx.toPublicKeyHex = w.publicKeyHex;
    zeroTx.amount = 0;
    zeroTx.ts = ts();
    std::string msg0 = buildTransactionMessage(w.publicKeyHex, w.publicKeyHex, 0);
    zeroTx.signatureHex = signMessage(msg0, w.privateKeyHex);
    zeroTx.id = computeTransactionID(zeroTx);
    CHECK("zero-amount transaction rejected", !validateTransaction(zeroTx, chain));

    Transaction negTx;
    negTx.fromPublicKeyHex = w.publicKeyHex;
    negTx.toPublicKeyHex = w.publicKeyHex;
    negTx.amount = -5;
    negTx.ts = ts();
    std::string msgN = buildTransactionMessage(w.publicKeyHex, w.publicKeyHex, -5);
    negTx.signatureHex = signMessage(msgN, w.privateKeyHex);
    negTx.id = computeTransactionID(negTx);
    CHECK("negative-amount transaction rejected", !validateTransaction(negTx, chain));
}

static void testCoinbaseRewardEnforcement() {
    std::cout << "\n[Coinbase reward enforcement]\n";
    Wallet miner; miner.generateKeys();
    Wallet sender; sender.generateKeys();

    Block genesis = makeGenesisBlock(miner.publicKeyHex);
    std::vector<Block> chain = {genesis};

    // Block with no coinbase
    Transaction normalTx = makeSignedTx(miner, sender.publicKeyHex, 1);
    Block noCoinbase(1, ts(), {normalTx}, genesis.hash);
    mineBlock(noCoinbase);
    CHECK("block with no coinbase rejected", !validateBlock(noCoinbase, genesis, chain));

    // Block with coinbase wrong reward
    Transaction badCb;
    badCb.fromPublicKeyHex = META;
    badCb.toPublicKeyHex = miner.publicKeyHex;
    badCb.amount = BLOCK_REWARD * 2;
    badCb.signatureHex = REWARD_SIG;
    badCb.ts = ts();
    badCb.id = computeTransactionID(badCb);
    Block wrongReward(1, ts(), {badCb}, genesis.hash);
    mineBlock(wrongReward);
    CHECK("block with inflated coinbase reward rejected",
          !validateBlock(wrongReward, genesis, chain));

    // Block with coinbase not first
    Transaction goodCb = makeCoinbase(miner.publicKeyHex);
    Block cbNotFirst(1, ts(), {normalTx, goodCb}, genesis.hash);
    mineBlock(cbNotFirst);
    CHECK("block with coinbase not first rejected",
          !validateBlock(cbNotFirst, genesis, chain));

    // Valid block
    Block validBlock(1, ts(), {goodCb}, genesis.hash);
    mineBlock(validBlock);
    CHECK("block with correct coinbase accepted", validateBlock(validBlock, genesis, chain));
}

static void testBlockIndexProgression() {
    std::cout << "\n[Block index progression]\n";
    Wallet miner; miner.generateKeys();

    Block genesis = makeGenesisBlock(miner.publicKeyHex);
    std::vector<Block> chain = {genesis};

    Transaction cb = makeCoinbase(miner.publicKeyHex);

    // Skip index (0 → 2)
    Block skipIdx(2, ts(), {cb}, genesis.hash);
    mineBlock(skipIdx);
    CHECK("block skipping index rejected", !validateBlock(skipIdx, genesis, chain));

    // Same index
    Block sameIdx(0, ts(), {cb}, genesis.hash);
    mineBlock(sameIdx);
    CHECK("block with same index as predecessor rejected",
          !validateBlock(sameIdx, genesis, chain));

    // Correct index
    Block correct(1, ts(), {cb}, genesis.hash);
    mineBlock(correct);
    CHECK("block with correct index accepted", validateBlock(correct, genesis, chain));
}

static void testCumulativeSpendInBlock() {
    std::cout << "\n[Cumulative spend within block]\n";
    Wallet miner; miner.generateKeys();
    Wallet alice; alice.generateKeys();
    Wallet bob;   bob.generateKeys();
    Wallet carol; carol.generateKeys();

    Block genesis = makeGenesisBlock(miner.publicKeyHex);
    std::vector<Block> chain = {genesis};

    // Miner has exactly BLOCK_REWARD coins. Try to spend 6 + 6 = 12 in one block.
    Transaction tx1 = makeSignedTx(miner, alice.publicKeyHex, 6);
    Transaction tx2 = makeSignedTx(miner, bob.publicKeyHex, 6);
    Transaction cb = makeCoinbase(carol.publicKeyHex);

    Block doubleSpend(1, ts(), {cb, tx1, tx2}, genesis.hash);
    mineBlock(doubleSpend);
    CHECK("cumulative double-spend within block rejected",
          !validateBlock(doubleSpend, genesis, chain));

    // Spending exactly BLOCK_REWARD should be fine
    Transaction txExact = makeSignedTx(miner, alice.publicKeyHex, BLOCK_REWARD);
    Block exactSpend(1, ts(), {cb, txExact}, genesis.hash);
    mineBlock(exactSpend);
    CHECK("spending full balance in one tx accepted", validateBlock(exactSpend, genesis, chain));
}

static void testDuplicateTxInBlock() {
    std::cout << "\n[Duplicate tx IDs in block]\n";
    Wallet miner; miner.generateKeys();
    Wallet alice; alice.generateKeys();

    Block genesis = makeGenesisBlock(miner.publicKeyHex);
    std::vector<Block> chain = {genesis};

    Transaction cb = makeCoinbase(miner.publicKeyHex);
    Transaction tx = makeSignedTx(miner, alice.publicKeyHex, 1);

    Block dupBlock(1, ts(), {cb, tx, tx}, genesis.hash);
    mineBlock(dupBlock);
    CHECK("block with duplicate tx IDs rejected", !validateBlock(dupBlock, genesis, chain));
}

static void testTxIDRecomputation() {
    std::cout << "\n[Transaction ID recomputation]\n";
    Wallet w; w.generateKeys();
    Block genesis = makeGenesisBlock(w.publicKeyHex);
    std::vector<Block> chain = {genesis};

    Transaction tx = makeSignedTx(w, w.publicKeyHex, 1);
    std::string savedId = tx.id;
    CHECK("tx with correct ID accepted", validateTransaction(tx, chain));

    tx.id = "tampered_id";
    CHECK("tx with tampered ID rejected", !validateTransaction(tx, chain));
    tx.id = savedId;

    // Tamper amount and keep old ID → ID mismatch
    tx.amount = 2;
    CHECK("tx with tampered amount (ID mismatch) rejected", !validateTransaction(tx, chain));
}

static void testMalformedInputs() {
    std::cout << "\n[Malformed hex inputs]\n";
    // Odd-length hex should not crash verifySignature
    CHECK("odd-length pubkey hex rejected", !verifySignature("msg", "aabb", "0x04abc"));
    // Non-hex characters
    CHECK("non-hex pubkey rejected", !verifySignature("msg", "aabb", std::string(130, 'Z')));
    // Wrong length (too short pubkey)
    CHECK("short pubkey rejected",
          !verifySignature("msg", std::string(128, 'a'), "04" + std::string(128, 'a')));
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "=== Agora validation tests ===\n";

    testGenesisValidation();
    testRejectNonPositiveAmounts();
    testCoinbaseRewardEnforcement();
    testBlockIndexProgression();
    testCumulativeSpendInBlock();
    testDuplicateTxInBlock();
    testTxIDRecomputation();
    testMalformedInputs();

    std::cout << "\n=== Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
