#pragma once
#include <string>
#include <vector>
#include "block.hpp"

using std::string;
using std::vector;


string signMessage(const string& messageHex, const string& privateKeyHex);
bool verifySignature(const string& messageHex, const string& signatureHex, const string& publicKeyHex);
string sha256(const string& data);
double getBalance(const string& pubkey, const vector<Block>& chain);
double getEffectiveBalanceDuringMining(const string& pubKeyHex,const vector<Block>& chain,const vector<Transaction>& includedTxs);



