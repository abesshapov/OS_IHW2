#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>

#define SHM_NAME "/myshm"
#define SEM_WRITE_NAME "sem_write"
#define SEM_READ_NAME "sem_read"

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
    int fd;
    sem_t *semaphore_write, *semaphore_read;
    pid_t pid;
    shm_unlink(SHM_NAME);
    fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        printf("Error making shared memory.\n");
        exit(EXIT_FAILURE);
    }
    ftruncate(fd, sizeof(struct shared));
    struct shared *data = mmap(NULL, sizeof(struct shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        printf("Error making tickets.\n");
        exit(EXIT_FAILURE);
    }
    data->areTicketsTaken = false;
    data->studentsLeft = k;
    for (int i = 0; i < 6; ++i) {
        struct ticket newTicket = {i + 1, false};
        data->tickets[i] = newTicket;
    }
    sem_unlink(SEM_WRITE_NAME);
    sem_unlink(SEM_READ_NAME);
    semaphore_write = sem_open(SEM_WRITE_NAME, O_CREAT | O_RDWR, 0666, 1);
    if (semaphore_write == SEM_FAILED) {
        printf("Error creating semaphore.\n");
        exit(EXIT_FAILURE);
    }
    semaphore_read = sem_open(SEM_READ_NAME, O_CREAT | O_RDWR, 0666, 0);
    if (semaphore_read == SEM_FAILED) {
        printf("Error creating semaphore.\n");
        exit(EXIT_FAILURE);
    }
    for (int j = 0; j < k; ++j) {
        pid = fork();
        if (pid == -1) {
            printf("Error forking.\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            for (;;) {
                sleep(1);
                sem_wait(semaphore_read);
                if (data->areTicketsTaken == true) {
                    printf("Student %d is waiting to be called.\n", j + 1);
                    fflush(stdout);
                    sem_post(semaphore_write);
                    sleep(10);
                }
                else {
                    int ticket_number = rand() % 6;
                    while (data->tickets[ticket_number].isTaken) {
                        ticket_number = rand() % 6;
                    }
                    data->tickets[ticket_number].isTaken = true;
                    sleep(1);
                    printf("Student %d has taken ticket number %d, the question: %s.\n", j + 1, ticket_number + 1, questions[ticket_number]);
                    fflush(stdout);
                    sem_post(semaphore_write);
                    sleep(10);
                    sem_wait(semaphore_read);
                    int knowsAnswer = rand() % 100;
                    int knowsAnswerSecondCheck = rand() % 100;
                    if (knowsAnswer + knowsAnswerSecondCheck >= 87) {
                        printf("Student %d: ticket number %d, answer: %s. Teacher: Mark - 5.\n", j + 1, ticket_number + 1, answers[ticket_number]);
                        fflush(stdout);
                    } else {
                        printf("Student %d: ticket number %d, I do not know the answer. Teacher: Mark - 2.\n", j + 1, ticket_number + 1);
                        fflush(stdout);
                    }
                    data->studentsLeft--;
                    data->tickets[ticket_number].isTaken = false;
                    sem_post(semaphore_write);
                    exit(EXIT_SUCCESS);
                }
            }
        }
    }
    while (data->studentsLeft > 0) {
        sem_wait(semaphore_write);
        bool ticketsTaken = true;
        for (int i = 0; i < 6; ++i) {
            if (data->tickets[i].isTaken == false) {
                ticketsTaken = false;
                break;
            }
        }
        data->areTicketsTaken = ticketsTaken;
        sem_post(semaphore_read);
    }
    sem_close(semaphore_write);
    sem_close(semaphore_read);
    sem_unlink(SEM_WRITE_NAME);
    sem_unlink(SEM_READ_NAME);
    munmap(data, sizeof(struct shared));
    shm_unlink(SHM_NAME);
    return 0;
}