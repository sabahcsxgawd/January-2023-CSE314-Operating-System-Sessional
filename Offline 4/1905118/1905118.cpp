#include <cstdio>
#include <chrono>
#include <random>
#include <vector>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

#define TIME std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time).count()

#define NUM_PRINTING_STATION 4
#define NUM_BINDING_STATION 2
#define NUM_STAFF 2

#define DID_NOT_ARRIVE 0
#define WAITING 1
#define PRINTING 2
#define PRINTED 3

#define GET_GROUP(id) ((((id)-1) / M) + 1)
#define GET_PRINTER(id) (((id) % NUM_PRINTING_STATION) + 1)
#define my_RAND (dist(rng) + 1)

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
;
std::poisson_distribution<int> dist(4);

int N, M, w, x, y;

std::chrono::high_resolution_clock::time_point start_time;

sem_t printer_mutex[NUM_PRINTING_STATION + 1];
sem_t binding_station_sem;
sem_t binding_station_mutex;
sem_t entry_book_mutex;
sem_t staff_mutex;

int staff_count;
int down_once_printer[NUM_PRINTING_STATION + 1];
int binding_station_use[NUM_BINDING_STATION + 1];
int submission_count;

std::vector<int> groupwise_print_count;
std::vector<sem_t> student_printing_sem;
std::vector<sem_t> leader_binding_start_sem;
std::vector<int> student_states;

FILE *fp1, *fp2;

void *student(void *id)
{

    int student_id = *(int *)id;
    free(id);
    int printer_station = GET_PRINTER(student_id);
    int group_id = GET_GROUP(student_id);

    sleep((my_RAND % 5) + 1);
    student_states[student_id] = WAITING;
    fprintf(fp2, "Student %d has arrived at the print station at time %ld\n", student_id, TIME);

    sem_wait(&printer_mutex[printer_station]);

    sem_wait(&student_printing_sem[student_id]);
    // fprintf(fp2, "-------------------WOKE STD %d------------------\n", student_id);
    student_states[student_id] = PRINTING;
    sleep(w);
    student_states[student_id] = PRINTED;
    groupwise_print_count[group_id]++;
    fprintf(fp2, "Student %d has finished printing at time %ld\n", student_id, TIME);
    if (groupwise_print_count[group_id] == M)
    {
        sem_post(&leader_binding_start_sem[group_id]);
    }

    // down all once
    if (down_once_printer[printer_station] == 0)
    {
        down_once_printer[printer_station] = 1;
        for (int i = (printer_station - 1 == 0 ? NUM_PRINTING_STATION : printer_station - 1); i <= N; i += NUM_PRINTING_STATION)
        {
            if ((student_states[i] == WAITING) && (i != student_id))
            {
                sem_wait(&student_printing_sem[i]);
            }
        }
    }

    sem_post(&printer_mutex[printer_station]);

    // up grp mates first then others
    for (int i = ((printer_station - 1) == 0 ? NUM_PRINTING_STATION : (printer_station - 1)); i <= N; i ++)
    {
        if ((student_states[i] == WAITING) && (group_id == GET_GROUP(i)))
        {
            sem_post(&student_printing_sem[i]);
            sleep(1);
        }
    }
    sleep((my_RAND % 2) + 1);
    for (int i = ((printer_station - 1) == 0 ? NUM_PRINTING_STATION : (printer_station - 1)); i <= N; i ++)
    {
        if ((student_states[i] == WAITING) && (group_id != GET_GROUP(i)))
        {
            sem_post(&student_printing_sem[i]);
            sleep(1);
        }
    }

    if (student_id % M == 0)
    {
        sem_wait(&leader_binding_start_sem[group_id]);
        fprintf(fp2, "Group %d has finished printing at time %ld\n", group_id, TIME);

        int bs = 0;
        sleep((my_RAND % 3) + 1);

        sem_wait(&binding_station_sem);

        sem_wait(&binding_station_mutex);

        for (int i = 1; i <= NUM_BINDING_STATION; i++)
        {
            if (binding_station_use[i] == 0)
            {
                binding_station_use[i] = 1;
                bs = i;
                sem_post(&binding_station_mutex);
                break;
            }
        }

        fprintf(fp2, "Group %d has started binding at time %ld\n", group_id, TIME);
        sleep(x);
        fprintf(fp2, "Group %d has finished binding at time %ld\n", group_id, TIME);

        sem_wait(&binding_station_mutex);
        binding_station_use[bs] = 0;
        sem_post(&binding_station_mutex);

        sem_post(&binding_station_sem);

        sleep((my_RAND % 4) + 1);

        sem_wait(&entry_book_mutex);
        sleep(y);
        fprintf(fp2, "Group %d has submitted the report at time %ld\n", group_id, TIME);
        submission_count++;
        sem_post(&entry_book_mutex);
    }
    return NULL;
}

void *staff(void *id)
{
    int staff_id = *(int *)id;
    free(id);

    for (;;)
    {
        sem_wait(&staff_mutex);
        staff_count++;
        if (staff_count == 1)
        {
            sem_wait(&entry_book_mutex);
        }
        sem_post(&staff_mutex);
        fprintf(fp2, "Staff %d has started reading the entry book at time %ld. No. of submission = %d\n", staff_id, TIME, submission_count);
        sleep(y);
        sem_wait(&staff_mutex);
        staff_count--;
        if (staff_count == 0)
        {
            if (submission_count == (N / M))
            {
                sem_post(&entry_book_mutex);
                sem_post(&staff_mutex);
                break;
            }
            sem_post(&entry_book_mutex);
        }
        sem_post(&staff_mutex);
        sleep((my_RAND % 8) + 4);
    }
    return NULL;
}

int main(int argc, char *argv[])
{

    if ((fp1 = fopen("in.txt", "r")) == NULL)
    {
        printf("Error Opening file\n");
        exit(1);
    }
    if ((fp2 = fopen("out.txt", "w")) == NULL)
    {
        printf("Error Opening file\n");
        exit(1);
    }

    fscanf(fp1, "%d%d%d%d%d", &N, &M, &w, &x, &y);

    assert(N > 0);
    assert(N > 0);
    assert(N % M == 0);
    assert(w > 0);
    assert(x > 0);
    assert(y > 0);

    student_printing_sem.resize(N + 1);
    student_states.resize(N + 1, DID_NOT_ARRIVE);
    groupwise_print_count.resize(1 + (N / M));
    leader_binding_start_sem.resize(1 + (N / M));

    for (int i = 1; i <= NUM_PRINTING_STATION; i++)
    {
        sem_init(&printer_mutex[i], 0, 1);
    }

    for (int i = 1; i <= N; i++)
    {
        sem_init(&student_printing_sem[i], 0, 1);
    }

    for (int i = 1; i <= (N / M); i++)
    {
        sem_init(&leader_binding_start_sem[i], 0, 0);
    }

    sem_init(&binding_station_sem, 0, NUM_BINDING_STATION);
    sem_init(&binding_station_mutex, 0, 1);
    sem_init(&entry_book_mutex, 0, 1);
    sem_init(&staff_mutex, 0, 1);

    pthread_t students[N], staffs[NUM_STAFF];

    start_time = std::chrono::high_resolution_clock::now();

    for (int i = 1; i <= N; i++)
    {
        int *tempID = (int *)malloc(sizeof(int));
        *tempID = i;
        if (pthread_create(&students[i - 1], NULL, student, (void *)tempID))
        {
            perror("Error creating thread\n");
            free(tempID);
            exit(1);
        }
    }

    for (int i = 1; i <= NUM_STAFF; i++)
    {
        int *tempID = (int *)malloc(sizeof(int));
        *tempID = i;
        if (pthread_create(&staffs[i - 1], NULL, staff, (void *)tempID))
        {
            perror("Error creating thread\n");
            free(tempID);
            exit(1);
        }
    }

    for (int i = 1; i <= N; i++)
    {
        if (pthread_join(students[i - 1], NULL))
        {
            perror("Error joining thread\n");
            exit(1);
        }
    }

    for (int i = 1; i <= NUM_STAFF; i++)
    {
        if (pthread_join(staffs[i - 1], NULL))
        {
            perror("Error joining thread\n");
            exit(1);
        }
    }

    for (int i = 1; i <= NUM_PRINTING_STATION; i++)
    {
        sem_destroy(&printer_mutex[i]);
    }

    for (int i = 1; i <= N; i++)
    {
        sem_destroy(&student_printing_sem[i]);
    }

    for (int i = 1; i <= (N / M); i++)
    {
        sem_destroy(&leader_binding_start_sem[i]);
    }

    sem_destroy(&binding_station_sem);
    sem_destroy(&binding_station_mutex);
    sem_destroy(&entry_book_mutex);
    sem_destroy(&staff_mutex);

    fclose(fp1);
    fclose(fp2);

    printf("DONE\n");

    return 0;
}