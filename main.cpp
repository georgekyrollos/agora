#include <iostream>
#include "wallet.hpp"
#include "crypto.hpp"
#include "mempool.hpp"
#include "blockchain.hpp"
#include <vector>
#include <string>

void usage(int code) {
    std::cerr << "Usage:\n";
    std::cerr << "  ./agora <wallet.dat>    to use your wallet (view balance or send)\n";
    std::cerr << "  ./agora --list          to view mempool\n";
    exit(code);
}

double getBalance(const std::string& pubkey, const std::vector<Block>& chain) {
    double balance = 0;
    for (const auto& block : chain) {
        for (const auto& tx : block.transactions) {
            if (tx.toPublicKeyHex == pubkey) balance += tx.amount;
            if (tx.fromPublicKeyHex == pubkey) balance -= tx.amount;
        }
    }
    return balance;
}

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--list") {
        printMempool();
        return 0;
    }

    if (argc != 2) usage(1);

    std::string walletPath = argv[1];
    Wallet w;

    if (!w.loadFromFile(walletPath)) {
        std::cerr << "Failed to load wallet from " << walletPath << "\n";
        return 1;
    }

    std::cout << "Welcome to Agora!\n";
    std::cout << "Your public key: " << w.publicKeyHex << "\n";

    while (true) {
        std::cout << "\nChoose an option:\n";
        std::cout << "1. View balance\n";
        std::cout << "2. Send transaction\n";
        std::cout << "3. Exit\n> ";

        int choice;
        std::cin >> choice;

        if (choice == 1) {
            std::vector<Block> chain = loadBlockchain("blockchain.json");
            double balance = getBalance(w.publicKeyHex, chain);
            std::cout << "Your current balance is: " << balance << "\n";
        } else if (choice == 2) {
            std::string recipient;
            int amount;

            std::cout << "Enter recipient public key:\n> ";
            std::cin >> recipient;

            std::cout << "Enter amount to send:\n> ";
            std::cin >> amount;

            std::string msg = buildTransactionMessage(w.publicKeyHex, recipient, amount);
            std::string sig = signMessage(msg, w.privateKeyHex);

            Transaction tx = { w.publicKeyHex, recipient, amount, sig };

            if (verifySignature(msg, tx.signatureHex, tx.fromPublicKeyHex)) {
                appendToMempool(tx);
                std::cout << "Transaction signed and added to mempool.\n";
            } else {
                std::cerr << "Signature failed to verify!\n";
            }
        } else if (choice == 3) {
            std::cout << "Goodbye!\n";
            break;
        } else {
            std::cout << "Invalid option. Try again.\n";
        }
    }

    return 0;
}





