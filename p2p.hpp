#pragma once

#include <string>
#include <vector>
#include <functional>
#include <thread>

using std::string;
using std::vector;

vector<string> loadIPs(const string& filename = "ips.dat");
void broadcastMessage(const string& message, int port = 8080);
void listenOnPort(int port, const std::function<void(const std::string&)>& handler);
