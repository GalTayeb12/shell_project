#ifndef SHARED_H
#define SHARED_H

#include <semaphore.h>

#define MAX_CARS 100
#define SHM_KEY 1234

typedef struct {
    int car_ids[MAX_CARS];
    double arrive_times[MAX_CARS];
    int front;
    int rear;
    int count;

    int washed_count;
    double total_wait_time;

    sem_t mutex;
    sem_t available_machines;
    sem_t cars_waiting;
    struct timeval global_start_time;
} SharedData;

#endif

