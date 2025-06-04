#include "wallet.hpp"
#include <openssl/ec.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

std::string toHex(const unsigned char* data, size_t len) {
    std::stringstream ss;
    for (size_t i = 0; i < len; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    return ss.str();
}

Wallet::Wallet() {}

void Wallet::generateKeys() {
    EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
    EC_KEY_generate_key(key);

    const BIGNUM* priv = EC_KEY_get0_private_key(key);
    const EC_POINT* pub = EC_KEY_get0_public_key(key);
    const EC_GROUP* group = EC_KEY_get0_group(key);

    unsigned char privBytes[32];
    BN_bn2binpad(priv, privBytes, 32);
    privateKeyHex = toHex(privBytes, 32);

    unsigned char pubBytes[65];
    EC_POINT_point2oct(group, pub, POINT_CONVERSION_UNCOMPRESSED, pubBytes, sizeof(pubBytes), nullptr);
    publicKeyHex = toHex(pubBytes, 65);

    EC_KEY_free(key);
}

void Wallet::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    file << privateKeyHex << "\n" << publicKeyHex << "\n";
    file.close();
}

bool Wallet::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) return false;
    std::getline(file, privateKeyHex);
    std::getline(file, publicKeyHex);
    file.close();
    return true;
}
