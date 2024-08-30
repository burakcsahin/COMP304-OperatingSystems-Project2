#ifndef ATC_SIMULATOR_H
#define ATC_SIMULATOR_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <fstream>

// Global variables
extern std::queue<int> landingQueue;
extern std::queue<int> departureQueue;
extern std::mutex mtx;
extern std::condition_variable cv;
extern bool runwayFree;
extern int planeID;
extern std::ofstream planesLog;

// Function declarations
void* planeThread(void* arg);
void* towerThread(void* arg);

#endif // ATC_SIMULATOR_H
