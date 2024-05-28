#include <stdio.h>
#include <stdlib.h>
#include "particles.h"
#define G 6.67259e-11
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

int main(void)
{

    struct body bodies[2];
    init_earth_sun(bodies, 2);
    FILE *fp;
    fp = fopen(FILENAME, "w");
    fclose(fp);

    for (int t = 0; t < 100000; t++)
    {

        for (int i = 0; i < 2; i++)
        {
            double force[2] = {0, 0};
            // calculate the total force acting on a particle
            for (int j = 0; j < 2; j++)
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
            bodies[i].vel[0] += acc[0];
            bodies[i].vel[1] += acc[1];
            // update the position of a particle
            bodies[i].pos[0] += bodies[i].vel[0];
            bodies[i].pos[1] += bodies[i].vel[1];
        }
        if (t % 1000 == 0)
        {
            print_sim(bodies, 2);
        }
    }
    return 0;
}
