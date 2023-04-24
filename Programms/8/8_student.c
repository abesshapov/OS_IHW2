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
    for (;;) {
        sleep(1);
        sem_buf.sem_num = 0;
        sem_buf.sem_op = -1;
        sem_buf.sem_flg = 0;
        semop(sem_read, &sem_buf, 1);
        if (data->areTicketsTaken == true) {
            printf("Student %d is waiting to be called.\n", k);
            fflush(stdout);
            sem_buf.sem_op = 1;
            semop(sem_write, &sem_buf, 1);
            sleep(10);
        }
        else {
            int ticket_number = rand() % 6;
            while (data->tickets[ticket_number].isTaken) {
                ticket_number = rand() % 6;
            }
            data->tickets[ticket_number].isTaken = true;
            sleep(1);
            printf("Student %d has taken ticket number %d, the question: %s.\n", k, ticket_number + 1, questions[ticket_number]);
            fflush(stdout);
            sem_buf.sem_op = 1;
            semop(sem_write, &sem_buf, 1);
            sleep(10);
            sem_buf.sem_num = 0;
            sem_buf.sem_op = -1;
            sem_buf.sem_flg = 0;
            semop(sem_read, &sem_buf, 1);
            int knowsAnswer = rand() % 100;
            int knowsAnswerSecondCheck = rand() % 100;
            if (knowsAnswer + knowsAnswerSecondCheck >= 87) {
                printf("Student %d: ticket number %d, answer: %s. Teacher: Mark - 5.\n", k, ticket_number + 1, answers[ticket_number]);
                fflush(stdout);
            } else {
                printf("Student %d: ticket number %d, I do not know the answer. Teacher: Mark - 2.\n", k, ticket_number + 1);
                fflush(stdout);
            }
            data->studentsLeft--;
            data->tickets[ticket_number].isTaken = false;
            sem_buf.sem_op = 1;
            semop(sem_write, &sem_buf, 1);
            semctl(sem_read, 0, IPC_RMID, 0);
            semctl(sem_write, 0, IPC_RMID, 0);
            shmdt(data);
            shmctl(shmid, IPC_RMID, NULL);
            exit(EXIT_SUCCESS);
        }
    }
}