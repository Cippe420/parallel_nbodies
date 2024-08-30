#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "particles.h"
#include <getopt.h>
#include <string.h>
#include "timer.h"

#define NUM_THREADS 8
#define G 6.67259e-11
#define FILENAME "data.csv"
#define TIMERFILE "pthread-parallel-times.csv"
#define DELTA_T 0.1

pthread_barrier_t barrier; //barriera condivisa per sincronizzare i thread

struct thread_data {
    struct body *bodies;
    int tid;
    int start;
    int end;
    int n_bodies;
    int n_step;
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

// the function to be executed as a thread
void *calculate_subset(void *arg)
{
    // profiling the thread function
    double start_t,finish,elapsed;
    struct thread_data *data = (struct thread_data*) arg;
    struct body *bodies = data->bodies;
    int start = data->start;
    int end = data->end;
    int n_bodies = data->n_bodies;
    int n_step = data->n_step;
    int tid = data->tid;

    GET_TIME(start_t);


    for (int t = 0; t < n_step;t++)
    {
        for (int i = start ; i<end ; i++)
        
        {

            double force[2] = {0,0};

            for (int j =0; j<n_bodies;j++)
            {
                if (i!=j)
                {
                    // acquisisce lock in lettura su j  
                    pthread_rwlock_rdlock(&bodies[j].lock);
                    compute_force(bodies[i],bodies[j],G,force);
                    double acc[2] = {0, 0};
                    compute_acceleration(bodies[i], force, acc);
                    pthread_rwlock_unlock(&bodies[j].lock);

                    // locks
                    pthread_rwlock_wrlock(&bodies[i].lock);
                    bodies[i].vel[0] += acc[0]*DELTA_T;
                    bodies[i].vel[1] += acc[1]*DELTA_T;
                    bodies[i].pos[0] += bodies[i].vel[0]*DELTA_T;
                    bodies[i].pos[1] += bodies[i].vel[1]*DELTA_T;
                    // unlocks
                    pthread_rwlock_unlock(&bodies[i].lock);
                }

            }

        }

        if (tid ==0 && t % 1000000 == 0){
            print_sim(bodies,n_bodies);
        }

        pthread_barrier_wait(&barrier);

    }

    GET_TIME(finish);

    elapsed = finish-start_t;

    if (tid == 0)
    {
        print_times(elapsed,n_step,n_bodies);
    }
    
    return NULL;
}



int main(int argc, char **argv)
{
    // initialize data for simulation
    int opt;
    struct body *bodies=NULL;
    int n_bodies = 0;
    int n_step = 10000;
    int width,height;
    char *saveptr;
    char simulation_name[32]="earth_sun";

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
            width = atoi(strtok_r(optarg, "-",&saveptr));
            height = atoi(strtok_r(NULL, "-",&saveptr)); 
            if (!(width && height)){
                fprintf(stderr,"è necessario inserire altezza e larghezza"); 
                return -1;
            }
            break;
        case '?':
            printf("Usage: %s [-t n_step] [-n n_bodies] [-S simulation_name]\n", argv[0]);
            exit(1);
        }
    }


    bodies = simulation__init(simulation_name,bodies,&n_bodies);

    if (bodies == NULL || n_bodies == 0)
    {
        printf("Error initializing simulation\n");
        exit(1);
    }
    // define the file pointer to which write the values
    FILE *fp;
    fp = fopen(FILENAME, "w");
    fclose(fp);

    pthread_t threads[NUM_THREADS];
    struct thread_data thread_data_array[NUM_THREADS];
    int bodies_per_thread = n_bodies / NUM_THREADS;
    pthread_barrier_init(&barrier,NULL,NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; i++) 
    {   
        thread_data_array[i].tid = i;
        thread_data_array[i].bodies = bodies;
        thread_data_array[i].start = i * bodies_per_thread;
        thread_data_array[i].end = (i == NUM_THREADS - 1) ? n_bodies : (i + 1) * bodies_per_thread;
        thread_data_array[i].n_bodies = n_bodies;
        thread_data_array[i].n_step = n_step;
        thread_data_array[i].fp = fp;
    }
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, calculate_subset, (void *)&thread_data_array[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    pthread_barrier_destroy(&barrier);

    free(bodies);
    return 0;

}