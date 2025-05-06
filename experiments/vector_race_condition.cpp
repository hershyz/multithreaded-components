#include <iostream>
#include <mutex>
#include <thread>

using namespace std;

vector<int> v;
mutex v_mutex;

void append(int threadID) {
    for (int i = 0; i < 3; i++) {
        v_mutex.lock();
        v.push_back(threadID);
        v_mutex.unlock();
        // this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main() {

    vector<thread> spawned_threads;

    for (int i = 1; i <= 5; i++) {
        spawned_threads.push_back(thread(append, i));
    }

    for (auto& t : spawned_threads) {
        t.join();
    }

    for (int n : v) {
        cout << n << " ";
    }
    cout << endl;

    return 0;
}