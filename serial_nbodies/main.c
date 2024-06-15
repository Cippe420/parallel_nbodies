#include <stdio.h>
#include <stdlib.h>
#include "particles.h"
#include <string.h>
#include <getopt.h>
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
    char simulation_name[32]="default";

    while((opt = getopt(argc, argv, "t:S:")) != -1){
        switch(opt){
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
                exit(1);

        }
    }

    struct body *bodies=NULL;

    bodies=simulation__init(simulation_name, bodies, &n_bodies);
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

        for (int i = 0; i < n_bodies; i++)
        {
            double force[2] = {0, 0};
            // calculate the total force acting on a particle
            for (int j = 0; j < n_bodies; j++)
            {
                if (i != j)
                {
                    compute_force(bodies[i], bodies[j], G, force);
                }
            }
            // calculate the acceleration of a particle
            double acc[2] = {0, 0};
            compute_acceleration(bodies[i], force, acc);
            // update the velocity of a particle
            bodies[i].vel[0] += acc[0]*DELTA_T;
            bodies[i].vel[1] += acc[1]*DELTA_T;
            // update the position of a particle
            bodies[i].pos[0] += bodies[i].vel[0]*DELTA_T;
            bodies[i].pos[1] += bodies[i].vel[1]*DELTA_T;
        }
        if (t % 1000000 == 0)
        {
            print_sim(bodies, n_bodies);
        }
    }
    free(bodies);
    return 0;
}
