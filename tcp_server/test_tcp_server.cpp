#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <vector>

using namespace std;

#define PORT 8080
#define NUM_CLIENTS 10

// raw_responses[client id] -> server response
vector<string> raw_responses;

// non-blocking client request
void client_task(int id) {
    
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "[client " << id << "] socket creation failed\n";
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "[client " << id << "] invalid address\n";
        return;
    }

    // connect to server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "[client " << id << "] connection failed\n";
        return;
    }

    // send message
    const char* msg = "hello from client";
    send(sock, msg, strlen(msg), 0);

    // block and read server response
    ssize_t n = read(sock, buffer, 1024);
    if (n > 0) {
        string response = string(buffer, n);
        std::cout << "[client " << id << "] received: " << response << "\n";
        raw_responses[id] = response;

    } else {
        std::cerr << "[client " << id << "] read failed or connection closed\n";
    }

    close(sock);
}

int main() {

    // preemptively raw responses so that we don't need a mutex for repeatedly appending to the vector w/ multiple threads
    raw_responses.resize(NUM_CLIENTS);

    // send parallel requests
    vector<thread> client_threads;
    for (int i = 0; i < NUM_CLIENTS; i++) {
        client_threads.emplace_back(client_task, i);
    }

    // join client threads
    for (auto& t : client_threads) {
        t.join();
    }

    // display
    cout << "client id | raw response" << endl;
    for (int i = 0; i < NUM_CLIENTS; i++) {
        cout << i << " | " << raw_responses[i];
    }
    cout << "------\n\n";

    return 0;
}