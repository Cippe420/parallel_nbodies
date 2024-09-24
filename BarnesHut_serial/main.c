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
#define PROFILERFILE "serial-bh-profiler.csv"
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

void print_times(double time,int steps,int bodies)
{
    FILE *fp;

    fp = fopen(TIMERFILE, "a");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    fprintf(fp,"[t=%d,n=%d] Elapsed time : %f\n",steps,bodies,time);
    fclose(fp);
}

void count_nodes(struct node *root,int *count)
{
    if(!root){
        *count = *count+1;
        return;
        }
    if(root)
        *count=*count+1;
    count_nodes(root->ne,count);
    count_nodes(root->nw,count);
    count_nodes(root->se,count);
    count_nodes(root->sw,count);

}
// scrive dentro al profiler i tempi di esecuzione in base a profiling essendo True o False
void write_profiler(double time,int operation)
{
    // operation being 0 for insert and 1 for calculate_force

    FILE *fp;
    fp = fopen(PROFILERFILE, "a");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    if(operation)
    {
        fprintf(fp,"calculate_force: %f\n\n",time);
    }
    else
    {
        fprintf(fp,"insert: %f\n",time);
    }
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
    int profiling=0;

    // opzione --profiler di optarg
    static struct option long_options[] = {
        {"profiler", required_argument, 0, 1}, 
        {0, 0, 0, 0} 
    };

    while ((opt = getopt_long(argc, argv, "t:n:S:C:",long_options,NULL)) != -1)
    {
        switch (opt)
        {
        case 1 :
            // setta il flag in base a --profiler essendo True o False
            if (strcmp(optarg, "True") == 0) {
                profiling = 1;
            } else if (strcmp(optarg, "False") == 0) {
                profiling = 0; 
            } else {
                fprintf(stderr, "Usage: --profiler must be True or False\n");
                exit(EXIT_FAILURE);
            }
            break;
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

    bodies = simulation__init(simulation_name, bodies, &n_bodies);

    if (bodies == NULL || n_bodies == 0)
    {
        printf("Error initializing simulation\n");
        exit(1);
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
    double startimer,finishtimer,elapsedtimer;

    GET_TIME(startimer);
    // starta la simulazione, se profiling è True (1) , salva all'interno del profiler file i tempi di esecuzione
    for (int t = 0; t < n_step; t++)
    {
        struct node *root = (struct node *)calloc(1, sizeof(struct node));
        // inserisco ogni corpo nell'albero
        if (profiling)
            GET_TIME(start);

        for (int i = 0; i < n_bodies; i++)
        {

            insert__Tree(root, &bodies[i]);
        }

        if (profiling)
        {
            GET_TIME(finish);
            elapsed = finish - start;
            write_profiler(elapsed,0);
        }

        
        if(profiling && n_bodies < 30)
            print_tree(root);
        int count = 0;

        if (profiling)
        {
            //write_profiler(elapsed,0,0);
            // ristarta il timer per profilare la calculate force
            GET_TIME(start);
        }

        for (int i = 0; i < n_bodies; i++)
        {
            double force[2] = {0, 0};
            int count = 0;
            Tree__calculate_force(root, &bodies[i], THETA, G,force,&count);
            bodies[i].vel[0] += (force[0] / bodies[i].mass) * DELTA_T;
            bodies[i].vel[1] += (force[1] / bodies[i].mass) * DELTA_T;
            bodies[i].pos[0] += bodies[i].vel[0] * DELTA_T;
            bodies[i].pos[1] += bodies[i].vel[1] * DELTA_T;
            count++;
        }

        if (profiling)
        {
            // stoppa il timer e scrive il tempo di esecuzione
            GET_TIME(finish);
            elapsed = finish - start;
            write_profiler(elapsed,1);
        }
        Tree__free(root);
        if (t % 1000000 == 0)
        {
            // print_sim(bodies, n_bodies);
            // printf("%.2f%%\r", ((float)t / n_step) * 100);
            // fflush(stdout);
        }
    }
    if (profiling)
    {
        GET_TIME(finish);
        elapsed = finish - start;
    }
    GET_TIME(finishtimer);
    elapsedtimer = finishtimer - startimer;
    print_times(elapsedtimer,n_step,n_bodies);
    free(bodies);

    return 0;
}