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
#define PROFILERFILE "pthread-bh-profiler.csv"

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

void profile_file(double time, int operation)
{
    FILE *fp;
    fp = fopen(PROFILERFILE, "a");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    if (operation)
    {
        fprintf(fp, "calculate_force: %f\n",time);
    }
    else
    {
        fprintf(fp, "insert: %f\n", time);
    }
    fclose(fp);
}

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

void count_nodes(struct node *root,int *count)
{
    if(!root){
        *count = *count+1;
        return;
    }
    if(root)
        *count = *count+1;
    count_nodes(root->ne,count);
    count_nodes(root->nw,count);
    count_nodes(root->se,count);
    count_nodes(root->sw,count);

}


void print_times(double time,int steps,int bodies)
{
    FILE *fp;

    fp = fopen(TIMERFILE, "a");
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
    // paarsing data sent to thread
    int start = data->start;
    int end = data->end;
    int n_bodies = data->n_bodies;
    int n_step = data->n_step;
    int tid = data->tid;
    double start_timer,finish,elapsed;



    // use private daata array to calc velocities and positions of particles
    typedef struct velocity
    {
        double x;
        double y;
    } velocity;

    typedef struct position
    {
        double x;
        double y;
    } position;

    velocity tempVelocities[end-start];
    position tempPositions[end-start];

    // each process locally saves initial velocities and positions
    for (unsigned long long i = start; i < end; i++)
    {
        tempVelocities[i-start].x = bodies[i].vel[0];
        tempVelocities[i-start].y = bodies[i].vel[1];
        tempPositions[i-start].x = bodies[i].pos[0];
        tempPositions[i-start].y = bodies[i].pos[1];
    }

    for (unsigned long long t = 0; t < n_step; t++)
    {
        struct node *root = (struct node *)calloc(1,sizeof(struct node));
    
        for (unsigned long long i = 0; i < n_bodies; i++)
        {
            insert__Tree(root, &bodies[i]);
        }

        // calculate force for bodies set
        for (unsigned long long i = start; i < end; i++)
        {
            double force[2] = {0, 0};
            Tree__calculate_force(root, &bodies[i], THETA, G,force);
            tempVelocities[i-start].x = tempVelocities[i-start].x + (force[0] / bodies[i].mass) * DELTA_T;
            tempVelocities[i-start].y = tempVelocities[i-start].y + (force[1] / bodies[i].mass) * DELTA_T;
            tempPositions[i-start].x = tempPositions[i-start].x + tempVelocities[i-start].x * DELTA_T;
            tempPositions[i-start].y = tempPositions[i-start].y + tempVelocities[i-start].y * DELTA_T;
        }   

        for (unsigned long long i = start; i < end; i++)
        {
            bodies[i].vel[0] = tempVelocities[i-start].x;
            bodies[i].vel[1] = tempVelocities[i-start].y;
            bodies[i].pos[0] = tempPositions[i-start].x;
            bodies[i].pos[1] = tempPositions[i-start].y;
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

    FILE *fp2;
    fp2 = fopen(TIMERFILE, "w");
    fclose(fp2);

    // FILE *fp3;
    // fp3 = fopen(PROFILERFILE, "w");
    // fclose(fp3);


    // initialize thread data
    pthread_t threads[NUM_THREADS];
    struct thread_data thread_data_array[NUM_THREADS];
    int bodies_per_thread = n_bodies / NUM_THREADS;
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    struct node *root = (struct node *)calloc(1, sizeof(struct node));

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

    double start_t, finish, elapsed;
    GET_TIME(start_t);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, calculate_subset, (void *)&thread_data_array[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    GET_TIME(finish);

    elapsed = finish - start_t;

    print_times(elapsed, n_step, n_bodies);

    pthread_barrier_destroy(&barrier);

    free(bodies);
    return 0;
}