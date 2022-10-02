#include <omp.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>

#define SIZE 5
#define NUMITER 26

sem_t empty, full, mutex ;

char buffer[SIZE];
int nextin = 0;
int nextout = 0;
int i, j;

void put(char item) {
    buffer[nextin] = item;
    nextin = (nextin + 1) % SIZE;
}

void producer(int tid) {
    char item;
    while (i < NUMITER) {
        sem_wait(&empty);
        sem_wait(&mutex);
        item = 'A' + (i % 26);
        put(item);
        i++;
        printf("%d Producing %c ...\n", tid, item);
        sem_post(&mutex);
        sem_post(&full);
        sleep(1);
    }
}

char get() {
    char item;
    item = buffer[nextout];
    nextout = (nextout + 1) % SIZE;
    return item;
}

void consumer(int tid) {
    char item;
    while (j < NUMITER) {
        sem_wait(&full);
        sem_wait(&mutex);
        item = get();
        j++;
        printf("%d ...Consuming %c\n", tid, item);
        sem_post(&mutex);
        sem_post(&empty);
        sleep(1);
    }
}

int main() {
    sem_init(&empty, 0, 5) ;
    sem_init(&full, 0, 0) ;
    sem_init(&mutex, 0, 1) ;

    int tid;
    i = j = 0;
    #pragma omp parallel private(tid) num_threads(4)
    {
        tid = omp_get_thread_num();

        if (tid != 1) {
            producer(tid);
        } else {
            consumer(tid);
        }
    }
}