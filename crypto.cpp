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
#include <cctype>


static string toHex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    return oss.str();
}

static bool isValidHex(const string& hex) {
    if (hex.length() % 2 != 0) return false;
    for (unsigned char c : hex)
        if (!std::isxdigit(c)) return false;
    return true;
}

static vector<unsigned char> fromHex(const string& hex) {
    vector<unsigned char> bytes;
    bytes.reserve(hex.length() / 2);
    for (size_t i = 0; i < hex.length(); i += 2)
        bytes.push_back(static_cast<unsigned char>(strtol(hex.substr(i, 2).c_str(), nullptr, 16)));
    return bytes;
}

string signMessage(const string& message, const string& privHex) {
    vector<unsigned char> privBytes = fromHex(privHex);

    EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
    BIGNUM* priv = BN_bin2bn(privBytes.data(), 32, nullptr);
    (void)EC_KEY_set_private_key(key, priv);

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
    if (!isValidHex(pubHex) || !isValidHex(sigHex)) return false;

    vector<unsigned char> pubBytes = fromHex(pubHex);
    vector<unsigned char> sigBytes = fromHex(sigHex);

    // Uncompressed secp256k1 public key: 0x04 prefix + 32 bytes X + 32 bytes Y = 65 bytes
    if (pubBytes.size() != 65 || pubBytes[0] != 0x04) return false;
    // Compact (r,s) signature: 32 + 32 = 64 bytes
    if (sigBytes.size() != 64) return false;

    EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
    EC_POINT* pub = EC_POINT_new(EC_KEY_get0_group(key));
    if (EC_POINT_oct2point(EC_KEY_get0_group(key), pub, pubBytes.data(), pubBytes.size(), nullptr) != 1) {
        EC_POINT_free(pub);
        EC_KEY_free(key);
        return false;
    }
    (void)EC_KEY_set_public_key(key, pub);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)message.c_str(), message.length(), hash);

    BIGNUM* r = BN_bin2bn(sigBytes.data(), 32, nullptr);
    BIGNUM* s = BN_bin2bn(sigBytes.data() + 32, 32, nullptr);
    ECDSA_SIG* sig = ECDSA_SIG_new();
    (void)ECDSA_SIG_set0(sig, r, s);

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

int64_t getBalance(const string& pubkey, const vector<Block>& chain) {
    int64_t balance = 0;
    for (const auto& block : chain) {
        for (const auto& tx : block.transactions) {
            if (tx.toPublicKeyHex == pubkey) balance += tx.amount;
            if (tx.fromPublicKeyHex == pubkey) balance -= tx.amount;
        }
    }
    return balance;
}

int64_t getEffectiveBalanceDuringMining(const string& pubKeyHex, const vector<Block>& chain,
                                        const vector<Transaction>& includedTxs) {
    int64_t confirmed = getBalance(pubKeyHex, chain);
    int64_t pendingSpent = 0;
    for (const auto& tx : includedTxs) {
        if (tx.fromPublicKeyHex == pubKeyHex)
            pendingSpent += tx.amount;
    }
    return confirmed - pendingSpent;
}
