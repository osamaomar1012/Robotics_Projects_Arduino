#ifndef ROBOT_GLOBAL_DEFINITIONS_H
#define ROBOT_GLOBAL_DEFINITIONS_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <queue>
#include <string>

// --- Configuration & Protocol ---
#include "AppConfig.h"
#include "Protocol.h"

// ===================================================================================
//  DATA QUEUE TEMPLATE
// ===================================================================================

/**
 * @brief Thread-safe queue for passing data between FreeRTOS tasks.
 * @tparam T Data type to store (e.g., String, int).
 * @details Uses a mutex to protect push/pop operations from concurrent access.
 */
template <typename T>
class DataQueue {
private:
    std::queue<T> q;
    int maxSize;
    bool calculateStats;
    T maxVal;
    SemaphoreHandle_t mutex;

public:
    DataQueue(int size, bool stats = false) : maxSize(size), calculateStats(stats), maxVal() {
        mutex = xSemaphoreCreateMutex();
    }

    void enterForced(T msg) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        if (q.size() >= maxSize) {
            q.pop();
        }
        q.push(msg);
        
        if (calculateStats) {
            if (msg > maxVal) maxVal = msg;
        }
        xSemaphoreGive(mutex);
    }

    T out() {
        xSemaphoreTake(mutex, portMAX_DELAY);
        T msg = T();
        if (!q.empty()) {
            msg = q.front();
            q.pop();
        }
        xSemaphoreGive(mutex);
        return msg;
    }

    bool isEmpty() {
        xSemaphoreTake(mutex, portMAX_DELAY);
        bool empty = q.empty();
        xSemaphoreGive(mutex);
        return empty;
    }

    int length() {
        return q.size();
    }

    T getMax() {
        return maxVal;
    }
};

// ===================================================================================
//  GLOBAL EXTERNS
// ===================================================================================

extern DataQueue<String> mqMotion;
extern DataQueue<String> mqInfo;
extern DataQueue<String> mqTx;
extern DataQueue<int> mqBz;

void enterMessageQueue(String msg);
String getRobotId();

inline void enterMessageQueue(std::string msg) {
    enterMessageQueue(String(msg.c_str()));
}

#endif