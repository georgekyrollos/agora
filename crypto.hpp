#pragma once
#include <string>

std::string signMessage(const std::string& messageHex, const std::string& privateKeyHex);
bool verifySignature(const std::string& messageHex, const std::string& signatureHex, const std::string& publicKeyHex);
std::string sha256(const std::string& data);
