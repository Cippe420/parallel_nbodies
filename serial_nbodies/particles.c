#include "particles.h"
#include <stdio.h>
#include <math.h>
// initialize the particles
void init_bodies(struct body bodies[], int n_bodies)
{

    for (int i = 0; i < n_bodies; i++)
    {
        if (i == 5)
        {
            bodies[i].mass = 5.9e10;
            bodies[i].pos[0] = i * 10;
            bodies[i].pos[1] = i * 10;
        }
        else
        {
            bodies[i].mass = 5.9e3;
            bodies[i].pos[0] = i * 10;
            bodies[i].pos[1] = i * 10;
        }
        bodies[i].vel[0] = 0;
        bodies[i].vel[1] = 0;
    }
}

void init_square(struct body *bodies, int n_bodies)
{

    for (int i = 0; i < n_bodies; i++)
    {
        if (i == 4)
        {
            bodies[i].mass = 5.9e10;
            bodies[i].pos[0] = 50;
            bodies[i].pos[1] = 50;
        }
        else
        {
            bodies[i].mass = 5.9e3;
            if (i == 0)
            {
                bodies[i].pos[0] = 10;
                bodies[i].pos[1] = 10;
            }
            else if (i == 1)
            {
                bodies[i].pos[0] = 10;
                bodies[i].pos[1] = 90;
            }
            else if (i == 3)
            {
                bodies[i].pos[0] = 90;
                bodies[i].pos[1] = 10;
            }
            else if (i == 2)
            {
                bodies[i].pos[0] = 90;
                bodies[i].pos[1] = 90;
            }
        }
        bodies[i].vel[0] = 0;
        bodies[i].vel[1] = 0;
    }
}

void init_earth_sun(struct body *bodies, int n_bodies){

    for (int i = 0; i<n_bodies;i++){

        if(i == 0){

            bodies[i].mass = 2e30;
            bodies[i].pos[0] = 0;
            bodies[i].pos[1] = 0;
        }
        else if(i==1){
            bodies[i].mass = 5e20;
            bodies[i].pos[0] = 0;
            bodies[i].pos[1] = 7e10;
        }

    }

}

// compute the force between two particles
void compute_force(const struct body body1, const struct body body2, double G, double *force)
{
    // Calculate distance (x,y) between particles
    double distanceX = body1.pos[0] - body2.pos[0];
    double distanceY = body1.pos[1] - body2.pos[1];

    // radix(x^2 + y^2)
    double distance = sqrt(distanceX * distanceX + distanceY * distanceY);
    if (distance < 1){
        distance = 1;
    }
    // normalize the distance
    double dist_cubed = distance * distance * distance;
    // calculate force(x,y) between particles
    force[0] -= G * body1.mass * body2.mass / dist_cubed * distanceX;
    force[1] -= G * body1.mass * body2.mass / dist_cubed * distanceY;
}

void compute_acceleration(struct body body, double force[], double *acc)
{

    acc[0] = force[0] / body.mass;
    acc[1] = force[1] / body.mass;
}

void print_bodies(struct body *bodies, int n_bodies)
{
    for (int i = 0; i < n_bodies; i++)
    {
        printf("Body %d\n", i);
        printf("Mass: %f\n", bodies[i].mass);
        printf("Velocity: %f %f\n", bodies[i].vel[0], bodies[i].vel[1]);
        printf("Position: %f %f\n", bodies[i].pos[0], bodies[i].pos[1]);
        printf("|----------------------------------------------------|\n\n");
    }
}