#include "crypto.hpp"
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <string>




static string toHex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    return oss.str();
}

static vector<unsigned char> fromHex(const string& hex) {
    vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.length(); i += 2)
        bytes.push_back((unsigned char) strtol(hex.substr(i, 2).c_str(), nullptr, 16));
    return bytes;
}

string signMessage(const string& message, const string& privHex) {
    vector<unsigned char> privBytes = fromHex(privHex);

    EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
    BIGNUM* priv = BN_bin2bn(privBytes.data(), 32, nullptr);
    EC_KEY_set_private_key(key, priv);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)message.c_str(), message.length(), hash);

    ECDSA_SIG* sig = ECDSA_do_sign(hash, sizeof(hash), key);
    const BIGNUM *r, *s;
    ECDSA_SIG_get0(sig, &r, &s);

    unsigned char rbin[32], sbin[32];
    BN_bn2binpad(r, rbin, 32);
    BN_bn2binpad(s, sbin, 32);

    string hexSig = toHex(rbin, 32) + toHex(sbin, 32);

    ECDSA_SIG_free(sig);
    BN_free(priv);
    EC_KEY_free(key);
    return hexSig;
}

bool verifySignature(const string& message, const string& sigHex, const string& pubHex) {
    vector<unsigned char> pubBytes = fromHex(pubHex);
    vector<unsigned char> sigBytes = fromHex(sigHex);

    // if (pubBytes.size() != 65 || pubBytes[0] != 0x04) {
    //     std::cerr << "Invalid public key format\n";
    //     return false;
    // }

    EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
    EC_POINT* pub = EC_POINT_new(EC_KEY_get0_group(key));
    EC_POINT_oct2point(EC_KEY_get0_group(key), pub, pubBytes.data(), pubBytes.size(), nullptr);
    EC_KEY_set_public_key(key, pub);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)message.c_str(), message.length(), hash);

    BIGNUM* r = BN_bin2bn(sigBytes.data(), 32, nullptr);
    BIGNUM* s = BN_bin2bn(sigBytes.data() + 32, 32, nullptr);
    ECDSA_SIG* sig = ECDSA_SIG_new();
    ECDSA_SIG_set0(sig, r, s);

    int valid = ECDSA_do_verify(hash, sizeof(hash), sig, key);

    ECDSA_SIG_free(sig);
    EC_POINT_free(pub);
    EC_KEY_free(key);

    return valid == 1;
}

string sha256(const string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)data.c_str(), data.size(), hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return oss.str();
}

double getBalance(const string& pubkey, const vector<Block>& chain) {
    double balance = 0;
    for (const auto& block : chain) {
        for (const auto& tx : block.transactions) {
            if (tx.toPublicKeyHex == pubkey) balance += tx.amount;
            if (tx.fromPublicKeyHex == pubkey) balance -= tx.amount;
        }
    }
    return balance;
}



double getEffectiveBalanceDuringMining(const string& pubKeyHex,const vector<Block>& chain,const vector<Transaction>& includedTxs) {
    double confirmed = getBalance(pubKeyHex, chain);
    double pendingSpent = 0.0;

    for (const auto& tx : includedTxs) {
        if (tx.fromPublicKeyHex == pubKeyHex) {
            pendingSpent += tx.amount;
        }
    }

    return confirmed - pendingSpent;
}



