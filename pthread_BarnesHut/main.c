#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include <math.h>
#include <pthread.h>
#include <getopt.h>
#include "timer.h"
#include <pthread.h>
#define THETA 0.5
#define G 6.674e-11
#define DELTA_T 0.1
#define FILENAME "data.csv"
#define NUM_THREADS 4
#define TIMERFILE "pthread-bh-times.csv"

// needing barrier to coordinate threads on operations
pthread_barrier_t barrier;

struct thread_data
{
    // the list of bodies 
    struct body *bodies;
    // the root of the tree
    struct node *root;
    // the thread identifier, always useful
    int tid;
    // where to start on the body list
    int start;
    // where to end
    int end;
    // how many bodies are there
    int n_bodies;
    // for how many steps simulate
    int n_step;
    // where to save the data
    FILE *fp;
};

void print_sim(struct body bodies[], int n_bodies)
{
    FILE *fp;

    fp = fopen(FILENAME, "a");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }
    for (int i = 0; i < n_bodies; i++)
    {
        fprintf(fp, "%f,%f,%f,%f\n", bodies[i].pos[0], bodies[i].pos[1], bodies[i].vel[0], bodies[i].vel[1]);
    }
    fprintf(fp, "\n");
    fclose(fp);
}

void print_times(double time,int steps,int bodies)
{
    FILE *fp;

    fp = fopen(TIMERFILE, "a");
    printf("aperto file\n");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    fprintf(fp,"[t=%d,n=%d] Elapsed time : %f\n",steps,bodies,time);
    fclose(fp);
}

// thread routine
void *calculate_subset(void *threaddata)
{
    // upon receiving data, parse it and start the routine
    struct thread_data *data = (struct thread_data *)threaddata;
    struct body *bodies = data->bodies;
    int start = data->start;
    int end = data->end;
    int n_bodies = data->n_bodies;
    int n_step = data->n_step;
    int tid = data->tid;

    for (int t = 0; t < n_step; t++)
    {
        struct node *root = (struct node *)calloc(1,sizeof(struct node));

        for (int i = 0; i < n_bodies; i++)
        {
            insert__Tree(root, &bodies[i]);
        }
        
        // calculate force for bodies set
        for (int i = start; i < end; i++)
        {
            // no need for a lock cause they are reading force from the tree
            double force[2] = {0, 0};
            Tree__calculate_force(root, &bodies[i], THETA, G,force);
            bodies[i].vel[0] += (force[0] / bodies[i].mass) * DELTA_T;
            bodies[i].vel[1] += (force[1] / bodies[i].mass) * DELTA_T;
            bodies[i].pos[0] += bodies[i].vel[0] * DELTA_T;
            bodies[i].pos[1] += bodies[i].vel[1] * DELTA_T;
        }
        
        Tree__free(root);

        // print into the csv
        if (tid == 0 && t % 1000000 == 0)
        {
            //print_sim(bodies, n_bodies);
        }

        pthread_barrier_wait(&barrier);

    }

    return NULL;
}

int main(int argc, char **argv)
{

    // parse optargs to start the program
    // initialize data for simulation
    int opt;
    struct body *bodies = NULL;
    int n_bodies = 0;
    int n_step = 10000;
    int width, height;
    char *saveptr;
    char simulation_name[32] = "earth_sun";
    double starTime,finishTime,elapsed;

    while ((opt = getopt(argc, argv, "t:n:S:C:")) != -1)
    {
        switch (opt)
        {
        case 't':
            n_step = atoi(optarg);
            break;
        case 'n':
            n_bodies = atoi(optarg);
            break;
        case 'S':
            strcpy(simulation_name, optarg);
            break;
        case 'C':
            width = atoi(strtok_r(optarg, "-", &saveptr));
            height = atoi(strtok_r(NULL, "-", &saveptr));
            if (!(width && height))
            {
                fprintf(stderr, "è necessario inserire altezza e larghezza");
                return -1;
            }
            break;
        case '?':
            printf("Usage: %s [-t n_step] [-n n_bodies] [-S simulation_name]\n", argv[0]);
            exit(1);
        }
    }

    // initialize program data
    bodies = simulation__init(simulation_name, bodies, &n_bodies);

    if (bodies == NULL || n_bodies == 0)
    {
        printf("Error initializing simulation\n");
        exit(1);
    }

    FILE *fp;
    fp = fopen(FILENAME, "w");
    fclose(fp);

    // initialize thread data
    pthread_t threads[NUM_THREADS];
    struct thread_data thread_data_array[NUM_THREADS];
    int bodies_per_thread = n_bodies / NUM_THREADS;
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    struct node *root = (struct node *)calloc(1, sizeof(struct node));
    init_node(root);

    // fill the thread data with everything needed to start the thread routine
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_data_array[i].tid = i;
        thread_data_array[i].bodies = bodies;
        thread_data_array[i].start = i * bodies_per_thread;
        thread_data_array[i].end = (i == NUM_THREADS - 1) ? n_bodies : (i + 1) * bodies_per_thread;
        thread_data_array[i].n_bodies = n_bodies;
        thread_data_array[i].n_step = n_step;
        thread_data_array[i].fp = fp;
        thread_data_array[i].root = root;
    }

    GET_TIME(starTime);


    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, calculate_subset, (void *)&thread_data_array[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    GET_TIME(finishTime);

    elapsed = finishTime - starTime;

    print_times(elapsed,n_step,n_bodies);

    pthread_barrier_destroy(&barrier);

    free(bodies);
    return 0;
}