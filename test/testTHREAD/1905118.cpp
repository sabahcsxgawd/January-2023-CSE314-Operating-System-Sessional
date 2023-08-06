#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>

#define TIME std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time).count()
#define NUM_PS 4
#define NUM_BS 2
#define NUM_STAFF 2
#define DID_NOT_ARRIVE 0
#define WAITING 1
#define PRINTING 2
#define PRINTED 3
#define GET_GROUP(id) ((((id)-1) / M) + 1)
#define GET_PRINTER(id) (((id) % NUM_PS) + 1)

int N, M, w, x, y;

std::chrono::high_resolution_clock::time_point start_time;

sem_t printer_mutex[NUM_PS + 1];
sem_t binding_station_sem;
sem_t binding_station_mutex;
sem_t entry_book_mutex;
sem_t stuff_mutex;
int stuff_count;
int down_once_printer[NUM_PS + 1];
int binding_station_use[NUM_BS + 1];
int submission_count;
std::vector<int> printer_print_count;
std::vector<sem_t> student_printing_sem;
std::vector<sem_t> leader_binding_start_sem;
std::vector<int> student_states;

void *student(void *id)
{

    int student_id = *(int *)id;
    free(id);
    int printer_station = GET_PRINTER(student_id);
    int group_id = GET_GROUP(student_id);

    sleep((rand() % 3) + 1);
    student_states[student_id] = WAITING;
    printf("Student %d has arrived at the print station at time %ld\n", student_id, TIME);

    sem_wait(&printer_mutex[printer_station]);

    sem_wait(&student_printing_sem[student_id]);
    student_states[student_id] = PRINTING;
    sleep(w);
    student_states[student_id] = PRINTED;
    printer_print_count[group_id]++;
    if (printer_print_count[group_id] == M)
    {
        sem_post(&leader_binding_start_sem[group_id]);
    }
    printf("Student %d has finished printing at time %ld\n", student_id, TIME);

    // down all once
    if (down_once_printer[printer_station] == 0)
    {
        down_once_printer[printer_station] = 1;
        for (int i = (printer_station - 1 == 0 ? NUM_PS : printer_station - 1); i <= N; i += NUM_PS)
        {
            if (student_states[i] == WAITING)
            {
                sem_wait(&student_printing_sem[i]);
            }
        }
    }

    sem_post(&printer_mutex[printer_station]);

    // up grp mates first then others
    for (int i = (printer_station - 1 == 0 ? NUM_PS : printer_station - 1); i <= N; i += NUM_PS)
    {
        if (student_states[i] == WAITING && group_id == GET_GROUP(i))
        {
            sem_post(&student_printing_sem[i]);
        }
    }
    for (int i = (printer_station - 1 == 0 ? NUM_PS : printer_station - 1); i <= N; i += NUM_PS)
    {
        if (student_states[i] == WAITING && group_id != GET_GROUP(i))
        {
            sem_post(&student_printing_sem[i]);
        }
    }

    if (student_id % M == 0)
    {
        sem_wait(&leader_binding_start_sem[group_id]);
        printf("Group %d has finished printing at time %ld\n", group_id, TIME);

        int bs = 0;
        sleep((rand() % 3) + 1);

        sem_wait(&binding_station_sem);

        sem_wait(&binding_station_mutex);

        for (int i = 1; i <= NUM_BS; i++)
        {
            if (binding_station_use[i] == 0)
            {
                binding_station_use[i] = 1;
                bs = i;
                sem_post(&binding_station_mutex);
                break;
            }
        }

        printf("Group %d has started binding at time %ld\n", group_id, TIME);
        sleep(x);
        printf("Group %d has finished binding at time %ld\n", group_id, TIME);
        binding_station_use[bs] = 0;

        sem_post(&binding_station_sem);

        sleep((rand() % 3) + 1);

        sem_wait(&entry_book_mutex);
        sleep(y);
        printf("Group %d has submitted the report at time %ld\n", group_id, TIME);
        submission_count++;
        sem_post(&entry_book_mutex);
    }
}

void *staff(void *id)
{
    int staff_id = *(int *)id;
    free(id);

    for (;;)
    {
        sem_wait(&stuff_mutex);
        stuff_count++;
        if (stuff_count == 1)
        {
            sem_post(&entry_book_mutex);
        }
        sem_post(&stuff_mutex);
        sleep((rand() % 3) + 1);
        printf("Staff %d has started reading the entry book at time %ld. No. of submission = %d\n", staff_id, TIME, submission_count);
        sleep(y);
        sem_wait(&stuff_mutex);
        stuff_count--;
        if (stuff_count == 0)
        {
            sem_post(&entry_book_mutex);
        }
        sem_post(&stuff_mutex);
    }
}

int main(int argc, char *argv[])
{
    FILE *fp;

    if ((fp = fopen("in.txt", "r")) == NULL)
    {
        printf("Error Opening file\n");
        exit(1);
    }

    fscanf(fp, "%d%d%d%d%d", &N, &M, &w, &x, &y);

    student_printing_sem.resize(N + 1);
    student_states.resize(N + 1, DID_NOT_ARRIVE);
    printer_print_count.resize(1 + (N / M));
    leader_binding_start_sem.resize(1 + (N / M));

    for (int i = 1; i <= NUM_PS; i++)
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

    sem_init(&binding_station_sem, 0, NUM_BS);
    sem_init(&binding_station_mutex, 0, 1);
    sem_init(&entry_book_mutex, 0, 1);
    sem_init(&stuff_mutex, 0, 1);

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

    for (int i = 1; i <= NUM_PS; i++)
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
    sem_destroy(&stuff_mutex);

    fclose(fp);
    return 0;
}