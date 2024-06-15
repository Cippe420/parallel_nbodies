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
    char simulation_name[32] = "earth_sun";
    int n_step = 1;
    bodies = simulation__init(simulation_name, bodies, &n_bodies);
    FILE *fp;
    fp = fopen(FILENAME, "w");
    fclose(fp);

    for(int t = 0; t<n_step; t++){
    
    struct node *root = (struct node *)malloc(sizeof(struct node));
    // inserisco ogni corpo nell'albero
    init_node(root);
    for (int i = 0; i < n_bodies; i++)
    {
        Tree__insert(root, &bodies[i]);
    }

    print_tree(root);

    Tree__free(root);
    print_sim(bodies, n_bodies);
    }
    free(bodies);
    
    return 0;
}