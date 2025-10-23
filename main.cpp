#include <iostream>
#include <vector>
#include <string>
#include <ctime>

#include "wallet.hpp"
#include "crypto.hpp"
#include "mempool.hpp"
#include "blockchain.hpp"
#include "chainset.hpp"
#include "message.hpp"
#include "p2p.hpp"
#include "pseudonym_resolver.hpp"

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::cin;

namespace {
    string getCurrentTimestamp() {
        time_t now = time(nullptr);
        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return string(buf);
    }

    // Helper: detect compressed 33-byte pubkey hex (for strict-mode warning)
    bool isCompressedPubkeyHex(string h) {
        if (h.rfind("0x", 0) == 0 || h.rfind("0X", 0) == 0) h = h.substr(2);
        if (h.size() != 66) return false;
        if (!(h[0] == '0' && (h[1] == '2' || h[1] == '3'))) return false;
        for (char c : h) {
            bool hex = (c >= '0' && c <= '9') ||
                       (c >= 'a' && c <= 'f') ||
                       (c >= 'A' && c <= 'F');
            if (!hex) return false;
        }
        return true;
    }
}

static const char* PSEUDONYMS_PATH = "pseudonyms.json";

void usage(int code) {
    cerr << "Usage:\n";
    cerr << "  ./agora <wallet.dat>    # use your wallet (view balance or send)\n";
    cerr << "  ./agora --list          # view mempool\n";
    cerr << "\nNote: recipients must be pseudonyms from " << PSEUDONYMS_PATH << "\n";
    exit(code);
}

int main(int argc, char* argv[]) {
    if (argc == 2 && string(argv[1]) == "--list") {
        printMempool();
        return 0;
    }
    if (argc != 2) usage(1);

    string walletPath = argv[1];
    Wallet w;
    if (!w.loadFromFile(walletPath)) {
        cerr << "Failed to load wallet from " << walletPath << "\n";
        return 1;
    }

    // Initialize pseudonym resolver (loads on startup)
    PseudonymResolver resolver(PSEUDONYMS_PATH);

    cout << "Welcome to Agora!\n";
    cout << "Your public key: " << w.publicKeyHex << "\n";

    for (;;) {
        cout << "\nChoose an option:\n";
        cout << "1. View balance\n";
        cout << "2. Send transaction (by pseudonym)\n";
        cout << "3. Exit\n> ";

        int choice;
        if (!(cin >> choice)) break;

        if (choice == 1) {
            const string BLOCKCHAINS_FILE = "blockchains.json";
            ChainSet chainSet = loadChainSet(BLOCKCHAINS_FILE);
            vector<Block> chain = chainSet.getMainChain();
            double balance = getBalance(w.publicKeyHex, chain);
            cout << "Your current balance is: " << balance << "\n";

        } else if (choice == 2) {
            string recipientPseudo;
            long long amount;

            cout << "Enter recipient pseudonym:\n> ";
            cin >> recipientPseudo;

            // Strict mode: do NOT accept raw pubkeys at the prompt.
            if (isCompressedPubkeyHex(recipientPseudo)) {
                cerr << "Direct public keys are not accepted at the prompt.\n"
                     << "Add a pseudonym mapping in " << PSEUDONYMS_PATH
                     << " (or via a contacts CLI) and try again.\n";
                continue;
            }

            cout << "Enter amount to send:\n> ";
            cin >> amount;

            // Resolve pseudonym -> hex pubkey (throws on error)
            string recipientPubHex;
            try {
                recipientPubHex = resolver.resolveToPubkey(recipientPseudo);
            } catch (const std::exception& e) {
                cerr << "Pseudonym error: " << e.what() << "\n";
                continue;
            }

            // Build & sign
            string msg = buildTransactionMessage(w.publicKeyHex, recipientPubHex, amount);
            string sig = signMessage(msg, w.privateKeyHex);

            Transaction tx;
            tx.fromPublicKeyHex = w.publicKeyHex;
            tx.toPublicKeyHex   = recipientPubHex;  // on-chain uses real pubkey
            tx.amount           = amount;
            tx.signatureHex     = sig;
            tx.ts               = getCurrentTimestamp();
            tx.id               = computeTransactionID(tx);

            string txMsg = formatTransactionMsg(tx);

            if (verifySignature(msg, tx.signatureHex, tx.fromPublicKeyHex)) {
                appendToMempool(tx);
                broadcastMessage(txMsg, DEFAULT_PORT);
                cout << "Transaction signed and broadcasted.\n";
                cout << "(to pseudonym \"" << recipientPseudo << "\" → " << recipientPubHex << ")\n";
            } else {
                cerr << "Signature failed to verify!\n";
            }

        } else if (choice == 3) {
            cout << "Goodbye!\n";
            break;

        } else {
            cout << "Invalid option. Try again.\n";
        }
    }

    return 0;
}
