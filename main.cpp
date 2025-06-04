#include <iostream>
#include "wallet.hpp"
#include "crypto.hpp"
#include "mempool.hpp"

void usage(int code) {
    std::cerr << "Usage:\n";
    std::cerr << "  ./agora <wallet.dat>    to send transaction\n";
    std::cerr << "  ./agora --list          to view mempool\n";
    exit(code);
}

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--list") {
        printMempool();
        return 0;
    }
    
    if (argc != 2) {
        usage(1);
    }
    

    std::string walletPath = argv[1];
    Wallet w;

    if (!w.loadFromFile(walletPath)) {
        std::cerr << "Failed to load wallet from " << walletPath << "\n";
        return 1;
    }

    std::cout << "Your public key: " << w.publicKeyHex << "\n";

    std::string recipient;
    int amount;

    std::cout << "Enter recipient public key:\n> ";
    std::cin >> recipient;

    std::cout << "Enter amount to send:\n> ";
    std::cin >> amount;

    std::string msg = w.publicKeyHex + "|" + recipient + "|" + std::to_string(amount);
    std::string sig = signMessage(msg, w.privateKeyHex);

    Transaction tx = { w.publicKeyHex, recipient, amount, sig };

    if (verifySignature(msg, tx.signatureHex, tx.fromPublicKeyHex)) {
        appendToMempool(tx);
        std::cout << "Transaction signed and added to mempool.\n";
    } else {
        std::cerr << "Signature failed to verify!\n";
    }

    return 0;
}



