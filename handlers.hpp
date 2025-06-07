#pragma once

#include <string>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

#include "validate.hpp"
#include "mempool.hpp"
#include "transaction.hpp"
#include "block.hpp"
#include "blockchain.hpp"
#include "chainset.hpp"
#include "p2p.hpp"
#include "sync.hpp"

using std::string;
using std::vector;
using std::function;
using json = nlohmann::json;

void handleMessage(const json& j);

void handleBlock(Block bk);
void handleTransaction(Transaction tx);
// add more later