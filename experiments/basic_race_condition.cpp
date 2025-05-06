#include <iostream>
#include <thread>

using namespace std;

void sayHello(int thread) {
    for (int i = 0; i < 5; i++) {
        cout << "Hello from thread" << thread << endl;
    }
}

int main() {

    thread t1(sayHello, 1);
    thread t2(sayHello, 2);

    t1.join();
    t2.join();

    return 0;
}