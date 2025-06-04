#include "wallet.hpp"
#include "transaction.hpp"
#include "crypto.hpp"
#include <iostream>

int main() {
    Wallet sender;
    sender.loadFromFile("wallet.dat");

    Wallet receiver;
    receiver.generateKeys(); // one-time throwaway receiver

    Transaction tx;
    tx.fromPublicKeyHex = sender.publicKeyHex;
    tx.toPublicKeyHex = receiver.publicKeyHex;
    tx.amount = 42.0;
    tx.signatureHex = signMessage(tx.serialize(), sender.privateKeyHex);

    std::cout << "Signed Transaction:\n";
    std::cout << "From: " << tx.fromPublicKeyHex << "\n";
    std::cout << "To: " << tx.toPublicKeyHex << "\n";
    std::cout << "Amount: " << tx.amount << "\n";
    std::cout << "Signature: " << tx.signatureHex << "\n";

    std::cout << "Verifying... " << (verifySignature(tx.serialize(), tx.signatureHex, tx.fromPublicKeyHex) ? "OK" : "FAIL") << "\n";
}

