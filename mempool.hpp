// mempool.hpp
#pragma once
#include "transaction.hpp"

void appendToMempool(const Transaction& tx);
void printMempool();
std::vector<Transaction> readMempool();
