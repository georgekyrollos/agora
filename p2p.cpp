#include "p2p.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <functional>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using std::thread;

vector<string> loadIPs(const string& filename) {
    vector<string> ips;
    std::ifstream file(filename);
    string line;
    while (std::getline(file, line)) {
        if (!line.empty()) ips.push_back(line);
    }
    return ips;
}

void sendToPeer(const string& message, const string& ip, int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "Socket creation failed for " << ip << "\n";
            return;
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid IP address: " << ip << "\n";
            close(sock);
            return;
        }

        if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Connection failed to " << ip << "\n";
            close(sock);
            return;
        }

        send(sock, message.c_str(), message.size(), 0);
        close(sock);
}

void broadcastMessage(const string& message, int port) {
    vector<string> peers = loadIPs(IPS_FILE);

    for (const auto& ip : peers) {
        sendToPeer(message, ip, port);
    }
}

void listenOnPort(int port, const std::function<void(const string&)>& handler) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        return;
    }

    std::cout << "[Listening] on port " << port << "...\n";

    while (true) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        thread([new_socket, handler]() {
            char buffer[4096] = {0};
            ssize_t valread = read(new_socket, buffer, sizeof(buffer));
            if (valread > 0) {
                string message(buffer, valread);
                handler(message);
            }
            close(new_socket);
        }).detach(); 
    }
}
