#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <mpi.h>
#include "tree.h"
#include "timer.h"

#define THETA 0.5
#define G 6.674e-11
#define DELTA_T 0.1
#define FILENAME "data.csv"
#define TIMERFILE "mpi-bh-times.csv"
#define PROFILERFILE "mpi-bh-profiler.csv"

void profile_file(double time,int operation)
{
    FILE *fp;
    fp = fopen(PROFILERFILE, "a");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    if(operation)
    {
        fprintf(fp,"calculate_force: %f\n",time);
    }
    else
    {
        fprintf(fp,"insert: %f\n",time);
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

int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int opt;
    struct body *bodies = NULL;
    int n_bodies = 0;
    char simulation_name[32] = "default";
    int n_step = 10000;
    double start_t,finish,elapsed;

    while ((opt = getopt(argc, argv, "t:n:S:")) != -1)
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
        case '?':
            printf("Usage: %s [-t n_step] [-n n_bodies] [-S simulation_name]\n", argv[0]);
            MPI_Finalize();
            exit(1);
        }
    }


    if (rank == 0)
    {
        bodies = simulation__init(simulation_name, bodies, &n_bodies);
        if (!bodies)
        {
            fprintf(stderr, "errore durante l'inizializzazione della simulazione\n");
            MPI_Finalize();
            return -1;
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
    }

    // Broadcast the number of bodies to all processes
    MPI_Bcast(&n_bodies, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Allocate memory for bodies on all processes
    if (rank != 0)
    {
        bodies = (struct body *)calloc(n_bodies, sizeof(struct body));
    }

    // Broadcast the initial state of bodies to all processes
    MPI_Bcast(bodies, n_bodies * sizeof(struct body), MPI_BYTE, 0, MPI_COMM_WORLD);

    // Determine the range of bodies each process will handle
    int bodies_per_proc = n_bodies / size;
    int start = rank * bodies_per_proc;
    int end = (rank == size - 1) ? n_bodies : start + bodies_per_proc;

    // definita una struct temporanea per mantenere localmente i dati
    typedef struct velocity{
        double x;
        double y;
    }velocity;

    typedef struct positions
    {
        double x;
        double y;
    }positions;

    // creato array locale per mantenere le velocità
    velocity tempVelocities[end-start];
    positions tempPositions[end-start];

    // each process locally saves initial velocities and positions
    for (int i = start; i < end; i++)
    {
        tempVelocities[i-start].x = bodies[i].vel[0];
        tempVelocities[i-start].y = bodies[i].vel[1];
        tempPositions[i-start].x = bodies[i].pos[0];
        tempPositions[i-start].y = bodies[i].pos[1];
    }

    GET_TIME(start_t);
    // for each timestep
    for (int t = 0; t < n_step; t++)
    {


        // Each process builds the whole tree
        struct node *root = (struct node *)calloc(1, sizeof(struct node));
        for (int i = 0; i < n_bodies; i++)
        {
            insert__Tree(root, &bodies[i]);
        }

        // Each process calculates the force acting on it's own bodies
        for (int i = start; i < end; i++)
        {
            // initialize forces array
            double force[2] = {0, 0};
            // calcola la forza sul tuo sotto-array di forze
            Tree__calculate_force(root, &bodies[i], THETA, G,force);
            // update velocities and position of bodies
            tempVelocities[i-start].x = tempVelocities[i-start].x + (force[0] / bodies[i].mass) * DELTA_T;
            tempVelocities[i-start].y = tempVelocities[i-start].y + (force[1] / bodies[i].mass) * DELTA_T;
            tempPositions[i-start].x = tempPositions[i-start].x + tempVelocities[i-start].x * DELTA_T;
            tempPositions[i-start].y = tempPositions[i-start].y + tempVelocities[i-start].y * DELTA_T;
        }

        for (int i = start; i < end ; i++)
        {
            // update velocity and position of bodies
            bodies[i].vel[0] = tempVelocities[i-start].x;
            bodies[i].vel[1] = tempVelocities[i-start].y;
            bodies[i].pos[0] = tempPositions[i-start].x;
            bodies[i].pos[1] = tempPositions[i-start].y;
        }

        // Gather all updated bodies from all processes
        MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, bodies, bodies_per_proc * sizeof(struct body), MPI_BYTE, MPI_COMM_WORLD);

        if (rank == 0)
        {
            if (t % 1000 == 0)
            {
                //print_sim(bodies, n_bodies);
                // printf("%.2f%%\r", ((float)t / n_step) * 100);
                // fflush(stdout);
            }
        }

        Tree__free(root);
    }
    if(rank==0)
    {
        GET_TIME(finish);
        elapsed = finish - start_t;
        print_times(elapsed,n_step,n_bodies);
    }

    free(bodies);
    MPI_Finalize();

    return 0;
}
