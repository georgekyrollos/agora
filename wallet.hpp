#pragma once
#include <string>

class Wallet {
public:
    std::string privateKeyHex;
    std::string publicKeyHex;

    Wallet();
    void generateKeys();
    void saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
};
