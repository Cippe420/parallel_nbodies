#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <mpi.h>
#include <tree.h>

#define THETA 0.5
#define G 6.674e-11
#define DELTA_T 0.1
#define FILENAME "data.csv"

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

        bodies = simulation_init(simulation_name, bodies, &n_bodies);

        if (!bodies)
        {
            fprintf(stderr, "errore durante l'inizializzazione della simulazione \n");
            MPI_Finalize();
            return -1;
        }

        FILE *fp;
        fp = fopen(FILENAME, "w");
        fclose(fp);
    }

    // manda il numero di corpi a tutti i processi
    MPI_Bcast(&n_bodies, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // alloca memoria per i corpi su tutti i processi
    if (rank != 0)
    {
        bodies = (struct body *)calloc(n_bodies, sizeof(struct body));
    }

    // broadcast the initial state of bodies to all processes
    MPI_Bcast(bodies, n_bodies*sizeof(struct body),MPI_BYTE,0,MPI_COMM_WORLD);

    for(int t = 0 ; t < n_step; t++)
    {
        struct node *root = (struct node *)calloc(1,sizeof(struct node));
        init_node(root);

        // ogni processo costruisce il proprio albero
        for(int i = rank; i<n_bodies;i+=size)
        {
            Tree__insert(root,&bodies[i]);
        }

        // il padre chiede la forza per ogni particella
        for (int i = 0; i<n_bodies; i++)
        {
            
        }
        // i figli calcolano la forza del loro albero,esercitata sulla particella

        // il padre aggiorna la sua lista di forze

    }
}