#include <iostream>
#include <thread>
#include <mutex>
#include <deque>
#include <random>
#include <condition_variable>

using namespace std;

mutex task_mutex, cout_mutex;
condition_variable cv;

deque<int> task_queue;

bool done = false;

// Simulated "thread runner"
void thread_worker(int thread_id) {
    while (true) {
        int task_id = -1;

        // [annotation]: the task mutex gets dropped after this section
        {
            unique_lock<mutex> lock(task_mutex);
            cv.notify_all();

            // Wait for a task or shutdown
            /*
                [annotation]:
                
                - wait (block) on the condition variable (cv)
                - release lock while waiting so other threads can use it
                - wakes up only when there is a task (!task_queue.empty()) or we are shutting down (done)
                - while blocked, the thread is sleeping and uses no CPU
                - when 'cv.notify_all()' is called, the thread:
                    - reacquires the task_mutex lock
                    - checks the lambda predicate '!task_queue.empty() || done'
            */
            cv.wait(lock, [&]() {
                return !task_queue.empty() || done;
            });

            // [annotation]: if no more tasks remain, we can exit this function, essentially killing the thread
            if (done && task_queue.empty()) return;

            // Grab a task
            task_id = task_queue.front();
            task_queue.pop_front();
        }

        // [annotation]: the cout mutex gets droppped after this section
        // Do work
        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Thread " << thread_id << " processing task " << task_id << endl;
        }

        // [annotation]: simulate work being done by sleeping for a random amount of time between [0, 2] seconds
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dist(0, 2);
        this_thread::sleep_for(chrono::seconds(dist(gen)));
    }
}

int main() {
    const int num_threads = 4;
    const int num_tasks = 10;

    vector<thread> pool;                        // [annotation]: these are just references to threads, not necessarily meaning they are available, we don't need a queue because of the condition variable

    // Launch threads
    for (int i = 0; i < num_threads; ++i) {
        pool.emplace_back(thread_worker, i);
    }

    // Feed tasks
    for (int i = 0; i < num_tasks; ++i) {
        {
            lock_guard<mutex> lock(task_mutex);
            task_queue.push_back(i);
        }
        cv.notify_all();
        this_thread::sleep_for(chrono::milliseconds(100)); // simulate staggered arrival
    }

    // Finish
    {
        lock_guard<mutex> lock(task_mutex);
        done = true;
    }
    cv.notify_all();

    /*
        [annotation]:
        - we notice that immedietly after enqueuing the tasks, the main thread ISN'T blocked and sets done = true
        - however, we wait for all the threads that we spawned in the thread pool to finish below by using joins
        - we notice in the thread worker function that the thread stops only if done=true AND the task queue is empty, so we are actually fine to not block the main thread
        - 'done' serves to indicate that all the tasks have been enqueued, and any locks held by a thread are dropped when it dies in this case
    */
    for (auto& t : pool) t.join();

    cout << "All tasks complete." << endl;

    return 0;
}
