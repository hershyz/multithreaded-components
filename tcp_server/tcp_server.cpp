#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <chrono>
#include <sstream>

using namespace std;

#define PORT 8080

// executed via child thread, non-blocking on main listener loop
void handle_client(int client_socket) {

    // allocate buffer
    char buffer[1024] = {0};

    // load client msg into buffer
    read(client_socket, buffer, 1024);

    // construct response (for experimentation purposes, just the microseconds since epoch)
    auto now = chrono::high_resolution_clock::now();
    auto us_since_epoch = chrono::duration_cast<chrono::microseconds>(now.time_since_epoch()).count();
    ostringstream oss;
    oss << "server response at " << us_since_epoch << " microseconds since epoch\n";
    string response = oss.str();

    // handle client
    send(client_socket, response.c_str(), response.size(), 0);
    cout << "[log]: sent client response at unique timestamp " << us_since_epoch << endl;
    close(client_socket);
}

int main() {

    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (::bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        return 1;
    }

    cout << "server listening on port " << PORT << "...\n";

    while (true) {
        
        int client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        std::thread t(handle_client, client_socket);
        t.detach();
    }

    return 0;
}