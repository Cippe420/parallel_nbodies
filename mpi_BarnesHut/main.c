#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <mpi.h>
#include "tree.h"

#define THETA 0.5
#define G 6.674e-11
#define DELTA_T 0.1
#define FILENAME "data.csv"

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

    // for each timestep
    for (int t = 0; t < n_step; t++)
    {
        // Each process builds the whole tree
        struct node *root = (struct node *)calloc(1, sizeof(struct node));
        init_node(root);
        for (int i = 0; i < n_bodies; i++)
        {
            Tree__insert(root, &bodies[i]);
        }

        // Each process calculates the force acting on it's own bodies
        for (int i = start; i < end; i++)
        {
            // initialize forces array
            double force[2] = {0, 0};
            // calcola la forza sul tuo sotto-array di forze
            Tree__calculate_force(root, &bodies[i], THETA, force, G);

            // update velocities and position of bodies
            bodies[i].vel[0] += (force[0] / bodies[i].mass)*DELTA_T;
            bodies[i].vel[1] += (force[1]/bodies[i].mass)*DELTA_T;
            bodies[i].pos[0] += bodies[i].vel[0] * DELTA_T;
            bodies[i].pos[1] += bodies[i].vel[1] * DELTA_T;

            // se non sei il padre,manda il risultato al padre
        }

        // Gather all updated bodies from all processes
        MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, bodies, bodies_per_proc * sizeof(struct body), MPI_BYTE, MPI_COMM_WORLD);

        if (rank == 0)
        {
            if (t % 1000000 == 0)
            {
                print_sim(bodies, n_bodies);
                printf("%.2f%%\r", ((float)t / n_step) * 100);
                fflush(stdout);
            }
        }

        Tree__free(root);
    }

    free(bodies);
    MPI_Finalize();
    return 0;
}
