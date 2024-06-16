#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include <math.h>
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

    struct body *bodies = NULL;
    int n_bodies = 0;
    char simulation_name[32] = "triangle";
    int n_step = 1;
    bodies = simulation__init(simulation_name, bodies, &n_bodies);
    FILE *fp;
    fp = fopen(FILENAME, "w");
    fclose(fp);

    for (int t = 0; t < n_step; t++)
    {

        struct node *root = (struct node *)malloc(sizeof(struct node));
        // inserisco ogni corpo nell'albero
        init_node(root);
        for (int i = 0; i < n_bodies; i++)
        {
            Tree__insert(root, &bodies[i]);
        }

        for (int i = 0; i < n_bodies; i++)
        {
            // TODO: fix the force calculation ( the sun goes fucking fast for no reason at all)
            double force[2] = {0, 0};
            Tree__calculate_force(root, &bodies[i], THETA, force, G);
            //printf("force acting on body %p with mass %f : %f,%f\n",&bodies[i],bodies[i].mass,force[0],force[1]);
            bodies[i].vel[0] += (force[0] / bodies[i].mass) * DELTA_T;
            bodies[i].vel[1] += (force[1] / bodies[i].mass) * DELTA_T;
            //printf("velocity of body %p: %f,%f\n",&bodies[i],bodies[i].vel[0],bodies[i].vel[1]);
            bodies[i].pos[0] += bodies[i].vel[0] * DELTA_T;
            bodies[i].pos[1] += bodies[i].vel[1] * DELTA_T;
            //printf("position of body %p : %f,%f\n\n",&bodies[i],bodies[i].pos[0],bodies[i].pos[1]);

        }

        Tree__free(root);
        if(t%1000000==0){
        print_sim(bodies, n_bodies);
        }
    }
    free(bodies);

    return 0;
}