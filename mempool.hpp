// mempool.hpp
#pragma once
#include "transaction.hpp"

#include <vector>
#include <string>

using std::string;
using std::vector;

void appendToMempool(const Transaction& tx);
void printMempool();
vector<Transaction> readMempool();
