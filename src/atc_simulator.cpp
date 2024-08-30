#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <random>
#include <chrono>
#include "atc_simulator.h"
#include "pthread_sleep.h"
#include "time.h"
#include <atomic>

std::atomic<bool> terminateFlag(false);
std::queue<int> landingQueue;
std::queue<int> departureQueue;
std::mutex mtx;
std::condition_variable cv;
bool runwayFree = true;
bool landFree = true;
bool departFree = true;
int departureID = 1;
int landingID = 2;
std::ofstream planesLog("planes.log");
int turn = 0;

//Plane struct to keep times correctly
struct Plane{
	int id;
	char status;
	std::string requestTime;
	int completionTime;
};

// Part 2 starvation prevention
int groundCounter = 0;
int consecutiveCounter = 0;

void* landingThread(void* arg) {
    int id = *(int*)arg;
    Plane *plane = new  Plane;

    std::unique_lock<std::mutex> lock(mtx);

    auto requestTime = std::chrono::system_clock::now();
    time_t requestTimeT = std::chrono::system_clock::to_time_t(requestTime);
    std::string timeStr = std::ctime(&requestTimeT);
    timeStr.pop_back();
    plane->requestTime = timeStr;
    plane->status = 'L';

    // Notify the tower
    landingQueue.push(id);
    cv.notify_all();

    // Wait for the runway to be free and permission from the tower
    cv.wait(lock, [id]() { return terminateFlag.load() || (landFree && runwayFree && landingQueue.front() == id); });
    if (terminateFlag.load()) {
        return NULL;
    }

    // Use the runway
    runwayFree = false;
    lock.unlock();

    pthread_sleep(2);  // Simulate time on the runway

    lock.lock();
    auto runwayTime = std::chrono::system_clock::now();
    time_t runwayTimeT = std::chrono::system_clock::to_time_t(runwayTime);

    std::string complete = std::ctime(&runwayTimeT);
    complete.pop_back();

    planesLog << "Plane " << id << " ===>" << plane->status << " " << "Request Time: " << plane->requestTime << "|| Runway Time: " << complete << "|| Wait time: " << difftime(runwayTimeT, requestTimeT) << " seconds\n";
    planesLog.flush();


    // Remove plane from queue
    landingQueue.pop();
    consecutiveCounter = 0;    
    landFree = false;
    runwayFree = true;
    cv.notify_all();
    return NULL;
}

void* departureThread(void* arg) {
    int id = *(int*)arg;
    Plane *plane = new Plane;
    std::unique_lock<std::mutex> lock(mtx);

    auto requestTime = std::chrono::system_clock::now();
    time_t requestTimeT = std::chrono::system_clock::to_time_t(requestTime);
    std::string timeStr = std::ctime(&requestTimeT);
    timeStr.pop_back();
    plane->requestTime = timeStr;
    plane->status = 'D';

    // Notify the tower
    departureQueue.push(id);
    groundCounter++;
    cv.notify_all();

    // Wait for the runway to be free and permission from the tower
    cv.wait(lock, [id]() { return terminateFlag.load() || (departFree && runwayFree && departureQueue.front() == id); });
    if (terminateFlag.load()) {
        return NULL;
    }

    // Use the runway
    runwayFree = false;
    lock.unlock();

    pthread_sleep(2);  // Simulate time on the runway

    lock.lock();
    auto runwayTime = std::chrono::system_clock::now();
    time_t runwayTimeT = std::chrono::system_clock::to_time_t(runwayTime);
    std::string complete = std::ctime(&runwayTimeT);
    complete.pop_back();

    planesLog << "Plane " << id << " ===>" << plane->status << " " << "Request Time: " << plane->requestTime << "|| Runway Time: " << complete << "|| Wait time: " << difftime(runwayTimeT, requestTimeT) << " seconds\n";

    planesLog.flush();

    // Remove plane from queue
    departureQueue.pop();
    groundCounter--;  // Decrement groundCounter when a plane takes off
    consecutiveCounter++; // Increment consecutiveCounter each time a plane takes off
    departFree = false;
    runwayFree = true;
    cv.notify_all();
    return NULL;
}

void* towerThread(void* arg) {
    while (!terminateFlag.load()) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []() { return !landingQueue.empty() || !departureQueue.empty(); });
	
	//We will prioritize landing planes until there are 5 planes waiting to take off on the ground and then we will give a chance for planes to land after 2 consecutive departs. This way we prevent starvation of both landing and departing planes.
	if (!landingQueue.empty() && (consecutiveCounter >= 2 || groundCounter < 5)) {
	    landFree = true;
	    consecutiveCounter = 0; // Reset ground processed counter
            cv.notify_all();
        } else if (!departureQueue.empty()) {
	    departFree = true;
            cv.notify_all();
        }

        lock.unlock();
        pthread_sleep(1);  // Check queues every second
    }
    return NULL;
}
int main(int argc, char* argv[]) {
    int simulationTime = 200;  // Default simulation time
    double p = 0.5;            // Default probability
    int n = 0;                 // Default log snapshot time

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            simulationTime = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            p = atof(argv[++i]);
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            n = atoi(argv[++i]);
        }
    }

    pthread_t tower;
    pthread_create(&tower, NULL, towerThread, NULL);
	
    std::vector<pthread_t> planeThreads;

    int* id1 = new int(departureID);
    int* id2 = new int(landingID);

    departureID += 2;
    landingID += 2;

    pthread_t plane1;
    pthread_t plane2;

    pthread_create(&plane1, NULL, departureThread, id1);
    pthread_create(&plane2, NULL, landingThread, id2);

    planeThreads.push_back(plane1);
    planeThreads.push_back(plane2);

    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0, 1.0);

    for (int time = 0; time <= simulationTime; time++) {
        pthread_sleep(1);

        double prob = distribution(generator);
        if (prob < p) {
            int* id = new int(landingID);
	    landingID += 2;
            pthread_t plane;
            pthread_create(&plane, NULL, landingThread, id);
	    planeThreads.push_back(plane);
        }
	if (prob < 1-p){
	    int* id = new int(departureID);
	    departureID += 2;
	    pthread_t plane;
	    pthread_create(&plane, NULL, departureThread, id);
	    planeThreads.push_back(plane);
	}

        if (time % n == 0) {
            std::unique_lock<std::mutex> lock(mtx);
            std::cout << "At " << time << " sec ground: ";
            std::queue<int> tempQueue = departureQueue;
            while (!tempQueue.empty()) {
                std::cout << tempQueue.front() << " ";
                tempQueue.pop();
            }
            std::cout << "\nAt " << time << " sec air: ";
            tempQueue = landingQueue;
            while (!tempQueue.empty()) {
                std::cout << tempQueue.front() << " ";
                tempQueue.pop();
            }
            std::cout << std::endl;
        }
    }

    terminateFlag.store(true);
    cv.notify_all();

    pthread_join(tower,NULL);

    for (pthread_t planeThread : planeThreads) {
        pthread_join(planeThread, NULL);
    }

    planesLog.close();
    return 0;
}
