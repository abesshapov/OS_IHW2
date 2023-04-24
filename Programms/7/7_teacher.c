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
    data->studentsLeft = 100;
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
    sleep(15);
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