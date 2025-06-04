#include "crypto.hpp"
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>


static std::string toHex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    return oss.str();
}

static std::vector<unsigned char> fromHex(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.length(); i += 2)
        bytes.push_back((unsigned char) strtol(hex.substr(i, 2).c_str(), nullptr, 16));
    return bytes;
}

std::string signMessage(const std::string& message, const std::string& privHex) {
    std::vector<unsigned char> privBytes = fromHex(privHex);

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

    std::string hexSig = toHex(rbin, 32) + toHex(sbin, 32);

    ECDSA_SIG_free(sig);
    BN_free(priv);
    EC_KEY_free(key);
    return hexSig;
}

bool verifySignature(const std::string& message, const std::string& sigHex, const std::string& pubHex) {
    std::vector<unsigned char> pubBytes = fromHex(pubHex);
    std::vector<unsigned char> sigBytes = fromHex(sigHex);

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

std::string sha256(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)data.c_str(), data.size(), hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return oss.str();
}
