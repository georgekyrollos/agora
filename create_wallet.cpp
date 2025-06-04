// create_wallet.cpp
#include <iostream>
#include "wallet.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./create_wallet <output_wallet.dat>\n";
        return 1;
    }

    std::string filename = argv[1];
    Wallet w;
    w.generateKeys();

    w.saveToFile(filename);
    
    std::cout << "Wallet created and saved to " << filename << "\n";
    std::cout << "Public Key:\n" << w.publicKeyHex << "\n";
    return 0;
}
