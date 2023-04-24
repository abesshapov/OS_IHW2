#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdbool.h>

#define SHM_KEY 6789
#define SEMR_KEY 7890
#define SEMW_KEY 7891

char questions[6][100] = {{"What is 5x5?"}, {"F(x) = x^2. What is f(x)?"}, {"Square root of 36?"}, {"Let f(x) = x+4, f(y) = 5, what is y?"}, {"What is 9/3?"}, {"This is...?"}};
char answers[6][100] = {{"It is 25"}, {"It is 2x"}, {"It is 6"}, {"It is 1"}, {"It is 3"}, {"An exemplary behavior of threads and semaphores"}};

struct ticket {
    int number;
    bool isTaken;
};

struct shared {
    struct ticket tickets[6];
    bool areTicketsTaken;
    int studentsLeft;
};

int main(int argc, char **argv) {
    int k = atoi(argv[1]);
    int shmid, sem_read, sem_write;
    pid_t pid;
    struct shared* data;
    struct sembuf sem_buf;
    shmid = shmget(SHM_KEY, sizeof(struct shared), IPC_CREAT | 0666);
    if (shmid == -1) {
        printf("Error getting shared memory.\n");
        exit(1);
    }
    data = (struct shared *) shmat(shmid, NULL, 0);
    if (data == (struct shared *) -1) {
        printf("Error creating tickets.\n");
        exit(EXIT_FAILURE);
    }
    data->areTicketsTaken = false;
    data->studentsLeft = 100;
    for (int i = 0; i < 6; ++i) {
        struct ticket newTicket = {i + 1, false};
        data->tickets[i] = newTicket;
    }
    sem_read = semget(SEMR_KEY, 1, IPC_CREAT | 0666);
    if (sem_read == -1) {
        printf("Error creating semaphore.\n");
        exit(EXIT_FAILURE);
    }
    semctl(sem_read, 0, SETVAL, 0);
    sem_write = semget(SEMW_KEY, 1, IPC_CREAT | 0666);
    if (sem_write == -1) {
        printf("Error creating semaphore.\n");
        exit(EXIT_FAILURE);
    }
    semctl(sem_write, 0, SETVAL, 1);
    sleep(15);
    while (data->studentsLeft > 0) {
        sem_buf.sem_num = 0;
        sem_buf.sem_op = -1;
        sem_buf.sem_flg = 0;
        semop(sem_write, &sem_buf, 1);
        bool ticketsTaken = true;
        for (int i = 0; i < 6; ++i) {
            if (data->tickets[i].isTaken == false) {
                ticketsTaken = false;
                break;
            }
        }
        data->areTicketsTaken = ticketsTaken;
        sem_buf.sem_op = 1;
        semop(sem_read, &sem_buf, 1);
    }
    semctl(sem_read, 0, IPC_RMID, 0);
    semctl(sem_write, 0, IPC_RMID, 0);
    shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}