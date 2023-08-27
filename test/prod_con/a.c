#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

struct queue
{
    int arr[16];
    int front;
    int rear;
    int size;
};

void queue_init(struct queue *q)
{
    q->front = q->rear = q->size = 0;
}
void queue_push(struct queue *q, int x)
{
    q->arr[q->rear] = x;
    q->rear = (q->rear + 1) % 16;
    q->size++;
}
int queue_front(struct queue *q)
{
    if (q->size <= 0)
    {
        return -1;
    }
    return q->arr[q->front];
}
void queue_pop(struct queue *q)
{
    q->front = (q->front + 1) % 16;
    q->size--;
}

sem_t empty, full, mx;
struct queue q;

void *ProducerFunc(void *arg)
{
    printf("%s\n", (char *)arg);
    int i;
    for (i = 1; i <= 10; i++)
    {
        // wait for semphore empty
        sem_wait(&empty);

        // wait for mutex lock
        sem_wait(&mx);

        sleep(1);
        queue_push(&q, i);
        printf("producer produced item %d\n", i);

        // unlock mutex lock
        sem_post(&mx);
        // post semaphore full
        sem_post(&full);
    }

    return NULL;
}

void *ConsumerFunc(void *arg)
{
    printf("%s\n", (char *)arg);
    int i;
    for (i = 1; i <= 10; i++)
    {
        // wait for semphore full
        sem_wait(&full);

        // wait for mutex lock
        sem_wait(&mx);

        sleep(1);
        int item = queue_front(&q);
        queue_pop(&q);
        printf("consumer consumed item %d\n", item);

        // unlock mutex lock
        sem_post(&mx);
        // post semaphore empty
        sem_post(&empty);
    }

    return NULL;
}

int main(void)
{
    sem_init(&empty, 0, 5);
    sem_init(&full, 0, 0);
    sem_init(&mx, 0, 1);

    pthread_t th_p, th_c;

    char * message1 = "i am producer";
	char * message2 = "i am consumer";

    if (pthread_create(&th_p, NULL, ProducerFunc, (void*)message1))
    {
        perror("Error creating thread\n");
        exit(1);
    }

    if (pthread_create(&th_c, NULL, ConsumerFunc, (void*)message2))
    {
        perror("Error creating thread\n");
        exit(1);
    }

    if (pthread_join(th_p, NULL))
    {
        perror("Error joining thread\n");
        exit(1);
    }

    if (pthread_join(th_c, NULL))
    {
        perror("Error joining thread\n");
        exit(1);
    }

    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mx);

    return 0;
}