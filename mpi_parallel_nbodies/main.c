#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <mpi.h>
#include "particles.h"

#define G 6.67259e-11
#define FILENAME "data.csv"
#define DELTA_T 0.1

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
    int opt;
    int n_step = 10000;
    int n_bodies = 0;
    char simulation_name[32] = "default";
    int width,height;
    char *saveptr;
    fopen(FILENAME,"w");

    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

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
            if (rank == 0)
            {
                printf("Usage: %s [-t n_step] [-n n_bodies] [-S simulation_name]\n", argv[0]);
            }
            MPI_Finalize();
            exit(1);
        }
    }

    struct body *bodies = NULL;

    if (rank == 0)
    {
        bodies = simulation__init(simulation_name, bodies, &n_bodies,width,height);
        if (bodies == NULL || n_bodies == 0)
        {
            printf("Error initializing simulation\n");
            MPI_Finalize();
            exit(1);
        }
    }

    // Broadcast number of bodies to all processes
    MPI_Bcast(&n_bodies, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0)
    {
        bodies = (struct body *)malloc(n_bodies * sizeof(struct body));
    }

    // Broadcast the initial bodies to all processes
    MPI_Bcast(bodies, n_bodies * sizeof(struct body), MPI_BYTE, 0, MPI_COMM_WORLD);

    int bodies_per_proc = n_bodies / size;
    int start = rank * bodies_per_proc;
    int end = (rank == size - 1) ? n_bodies : start + bodies_per_proc;

    for (int t = 0; t < n_step; t++)
    {
        for (int i = start; i < end; i++)
        {
            double force[2] = {0, 0};
            for (int j = 0; j < n_bodies; j++)
            {
                if (i != j)
                {
                    compute_force(bodies[i], bodies[j], G, force);
                }
            }
            double acc[2] = {0, 0};
            compute_acceleration(bodies[i], force, acc);
            bodies[i].vel[0] += acc[0] * DELTA_T;
            bodies[i].vel[1] += acc[1] * DELTA_T;
            bodies[i].pos[0] += bodies[i].vel[0] * DELTA_T;
            bodies[i].pos[1] += bodies[i].vel[1] * DELTA_T;
        }

        // Gather all updated bodies from all processes
        MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, bodies, bodies_per_proc * sizeof(struct body), MPI_BYTE, MPI_COMM_WORLD);

        if (t % 10000 == 0)
        {
            if (rank == 0)
                print_sim(bodies, n_bodies);
        }
    }

    free(bodies);
    MPI_Finalize();
    return 0;
}
