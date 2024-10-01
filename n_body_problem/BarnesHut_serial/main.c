#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include <math.h>
#include <getopt.h>
#define THETA 0.5
#define G 6.674e-11
#define DELTA_T 0.1
#define FILENAME "data.csv"
#define TIMERFILE "serial-bh-times.csv"
#include "timer.h"

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

void print_times(double time, int steps, int bodies)
{
    FILE *fp;

    fp = fopen(TIMERFILE, "a");
    printf("aperto file\n");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    fprintf(fp, "[t=%d,n=%d] Elapsed time : %f\n", steps, bodies, time);
    fclose(fp);
}

int main(int argc, char **argv)
{

    // initialize data for simulation
    int opt;
    struct body *bodies = NULL;
    int n_bodies = 0;
    int n_step = 10000;
    int width, height;
    char *saveptr;
    char simulation_name[32] = "earth_sun";
    double start, finish, elapsed;

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

    GET_TIME(start);

    bodies = simulation__init(simulation_name, bodies, &n_bodies);

    if (bodies == NULL || n_bodies == 0)
    {
        printf("Error initializing simulation\n");
        exit(1);
    }

    FILE *fp;
    fp = fopen(FILENAME, "w");
    fclose(fp);
    for (int t = 0; t < n_step; t++)
    {
        struct node *root = (struct node *)calloc(1, sizeof(struct node));
        // inserisco ogni corpo nell'albero
        init_node(root);
        for (int i = 0; i < n_bodies; i++)
        {
            Tree__insert(root, &bodies[i]);
        }

        for (int i = 0; i < n_bodies; i++)
        {
            double force[2] = {0, 0};
            Tree__calculate_force(root, &bodies[i], THETA, force, G);
            bodies[i].vel[0] += (force[0] / bodies[i].mass) * DELTA_T;
            bodies[i].vel[1] += (force[1] / bodies[i].mass) * DELTA_T;
            bodies[i].pos[0] += bodies[i].vel[0] * DELTA_T;
            bodies[i].pos[1] += bodies[i].vel[1] * DELTA_T;
        }
        Tree__free(root);
        if (t % 1000000 == 0)
        {
            print_sim(bodies, n_bodies);
            printf("%.2f%%\r", ((float)t / n_step) * 100);
            fflush(stdout);
        }
    }

    GET_TIME(finish);
    elapsed = finish - start;

    print_times(elapsed, n_step, n_bodies);

    free(bodies);

    return 0;
}