#pragma once

#include <string>
#include <vector>
#include <functional>
#include <thread>

using std::string;
using std::vector;

const int DEFAULT_PORT = 8080;
const string IPS_FILE = "ips.dat";

vector<string> loadIPs(const string& filename);
void broadcastMessage(const string& message, int port);
void listenOnPort(int port, const std::function<void(const std::string&)>& handler);
