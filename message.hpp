#pragma once

#include <string>

#include "transaction.hpp"
#include "block.hpp"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

const string TX_FLAG = "TX";
const string BK_FLAG = "BK";

string formatTransactionMsg(Transaction tx);

string formatBlockMsg(Block bk);