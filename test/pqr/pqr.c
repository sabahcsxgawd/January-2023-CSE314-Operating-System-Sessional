#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

sem_t sem_p, sem_q, sem_r;
int count;

/*
1 0 0
p
0 1 0 <<------------|
q                  .|     
0 0 1              .|
r                  .|     
0 0 1              .|
r                  .|     
1 0 0              .|
p                  .|     
0 1 0              .|
q                  .|     
0 1 0              .|
q                  .|     
0 0 1              .|
r                  .|     
1 0 0              .|
p                  .|     
....................|
*/

void* print_p(void* args) {
    while (1)
    {
        sem_wait(&sem_p);
        if (count < 2)
        {
            printf("p, ");
            count++;            
        }
        else {
            printf("p\n");
            count = 0;
            sleep(1);
            sem_post(&sem_p);
            continue;
        }
        sem_post(&sem_q);
    }
    return NULL;
}

void* print_q(void* args) {
    while (1)
    {
        sem_wait(&sem_q);
        if (count < 2)
        {
            printf("q, ");
            count++;            
        }
        else {
            printf("q\n");
            count = 0;
            sleep(1);
            sem_post(&sem_q);
            continue;
        }
        sem_post(&sem_r);
    }
    return NULL;
}

void* print_r(void* args) {
    while (1)
    {
        sem_wait(&sem_r);
        if (count < 2)
        {
            printf("r, ");
            count++;            
        }
        else {
            printf("r\n");
            count = 0;
            sleep(1);
            sem_post(&sem_r);
            continue;
        }
        sem_post(&sem_p);
    }
    return NULL;
}

int main(void) {
    sem_init(&sem_p, 0, 1);
    sem_init(&sem_q, 0, 0);
    sem_init(&sem_r, 0, 0);

    pthread_t th_p, th_q, th_r;

    if(pthread_create(&th_p, NULL, print_p, NULL)) {
        perror("Error creating thread\n");
        exit(1);
    }

    if(pthread_create(&th_q, NULL, print_q, NULL)) {
        perror("Error creating thread\n");
        exit(1);
    }

    if(pthread_create(&th_r, NULL, print_r, NULL)) {
        perror("Error creating thread\n");
        exit(1);
    }

    if(pthread_join(th_p, NULL)) {
        perror("Error joining thread\n");
        exit(1);
    }

    if(pthread_join(th_q, NULL)) {
        perror("Error joining thread\n");
        exit(1);
    }

    if(pthread_join(th_r, NULL)) {
        perror("Error joining thread\n");
        exit(1);
    }


    sem_destroy(&sem_p);
    sem_destroy(&sem_q);
    sem_destroy(&sem_r);
    
    return 0;
}