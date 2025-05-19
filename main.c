#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "shared.h"

// פונקציית זמן
double get_time_diff(struct timeval start) {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1e6;
}

// פונקציית זמן אקראי לפי התפלגות מעריכית
float nextTime(float rateParameter) {
    return -logf(1.0f - (float)rand() / ((float)RAND_MAX + 1.0f)) / rateParameter;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s numOfMachines avg_arrive_time avg_wash_time run_time\n", argv[0]);
        exit(1);
    }

    int num_machines = atoi(argv[1]);
    float lambda_arrive = 1.0f / atof(argv[2]);
    float lambda_wash = 1.0f / atof(argv[3]);
    int run_time = atoi(argv[4]);

    srand(time(NULL));

    int shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    SharedData* shared = (SharedData*)shmat(shmid, NULL, 0);
    if (shared == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }

    // אתחול תור וסמפורים
    shared->front = shared->rear = shared->count = 0;
    shared->washed_count = 0;
    shared->total_wait_time = 0.0;

    sem_init(&shared->mutex, 1, 1);
    sem_init(&shared->available_machines, 1, num_machines);
    sem_init(&shared->cars_waiting, 1, 0);

    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    shared->global_start_time = start_time;
    time_t sim_start = time(NULL);
    int car_id = 0;

    while ((time(NULL) - sim_start) < run_time) {
        pid_t pid = fork();
        if (pid == 0) {
            // תהליך רכב
            char id_str[10], lambda_str[20];
            sprintf(id_str, "%d", car_id);
            sprintf(lambda_str, "%f", lambda_wash);
            execl("./car", "car", id_str, lambda_str, NULL);
            perror("execl failed");
            exit(1);
        } else if (pid > 0) {
            car_id++;
            float delay = nextTime(lambda_arrive);
            usleep((useconds_t)(delay * 1e6));
        } else {
            perror("fork failed");
        }
    }

    while (wait(NULL) > 0);

    double runtime = get_time_diff(start_time);
    double avg_wait = shared->washed_count > 0 ?
                      shared->total_wait_time / shared->washed_count : 0.0;

    printf("Time taken %.5f seconds, avg wait time = %.5f, car washed = %d\n",
           runtime, avg_wait, shared->washed_count);

    // ניקוי
    sem_destroy(&shared->mutex);
    sem_destroy(&shared->available_machines);
    sem_destroy(&shared->cars_waiting);
    shmdt(shared);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

