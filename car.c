#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include "shared.h"

// זמן מרגע ההתחלה
double get_time_diff(struct timeval start) {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1e6;
}

// זמן רנדומלי לפי התפלגות מעריכית
float nextTime(float rateParameter) {
    return -logf(1.0f - (float)rand() / ((float)RAND_MAX + 1.0f)) / rateParameter;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: car <car_id> <lambda_wash>\n");
        exit(1);
    }

    int car_id = getpid();
    float lambda_wash = atof(argv[1]);

    int shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    SharedData* shared = (SharedData*)shmat(shmid, NULL, 0);
    if (shared == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }

    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    double arrive_time = get_time_diff(shared->global_start_time);

    printf("car : %d, Arrive at time %.6f\n", car_id, arrive_time);

    sem_wait(&shared->mutex);
    shared->car_ids[shared->rear] = car_id;
    shared->arrive_times[shared->rear] = arrive_time;
    shared->rear = (shared->rear + 1) % MAX_CARS;
    shared->count++;
    sem_post(&shared->mutex);

    sem_post(&shared->cars_waiting);
    sem_wait(&shared->available_machines);

    sem_wait(&shared->mutex);
    double entry_time = get_time_diff(start_time);
    double wait_time = entry_time - shared->arrive_times[shared->front];
    shared->front = (shared->front + 1) % MAX_CARS;
    shared->count--;
    shared->washed_count++;
    shared->total_wait_time += wait_time;
    sem_post(&shared->mutex);

    printf("car : %d , in the washing machine , Time : %.6f\n", car_id, entry_time);

    float wash_time = nextTime(lambda_wash);
    usleep((useconds_t)(wash_time * 1e6));

    double leave_time = get_time_diff(start_time);
    printf("car : %d , leave the washing machine! , Time : %.6f\n", car_id, leave_time);

    sem_post(&shared->available_machines);
    shmdt(shared);
    return 0;
}

