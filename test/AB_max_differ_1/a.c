#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

sem_t sem_a, sem_b, mutex;
int cnta, cntb;

void* print_a(void* args) {
    while (1)
    {
        sem_wait(&sem_a);
        sem_wait(&mutex);
        if(cnta == cntb) {
            printf("A");
            cnta++;
            sem_post(&sem_b);
        }
                
        else if(cntb - cnta == 1) {
            printf("A");
            cnta++;
            sem_post(&sem_a);
            sem_post(&sem_b);
        }
        sem_post(&mutex);
        usleep(1000);
    }
    return NULL;
}

void* print_b(void* args) {
    while (1)
    {
        sem_wait(&sem_b);
        sem_wait(&mutex);
        if(cnta == cntb) {
            printf("B");
            cntb++;
            sem_post(&sem_a);
        }
                
        else if(cnta - cntb == 1) {
            printf("B");
            cntb++;
            sem_post(&sem_a);
            sem_post(&sem_b);
        }
        sem_post(&mutex);
        usleep(1000);
    }
    return NULL;
}

int main(void) {
    sem_init(&sem_a, 0, 1);
    sem_init(&sem_b, 0, 1);
    sem_init(&mutex, 0, 1);

    pthread_t th_a, th_b;

    if(pthread_create(&th_a, NULL, print_a, NULL)) {
        perror("Error creating thread\n");
        exit(1);
    }

    if(pthread_create(&th_b, NULL, print_b, NULL)) {
        perror("Error creating thread\n");
        exit(1);
    }

    if(pthread_join(th_a, NULL)) {
        perror("Error joining thread\n");
        exit(1);
    }

    if(pthread_join(th_b, NULL)) {
        perror("Error joining thread\n");
        exit(1);
    }

    sem_destroy(&sem_a);
    sem_destroy(&sem_b);
    sem_destroy(&mutex);
    
    return 0;
}