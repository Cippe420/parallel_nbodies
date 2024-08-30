#ifndef __PARTICLES_H__
#define __PARTICLES_H__ 

struct body
{    
    double mass;
    double vel[2];
    double pos[2];
};

// initialize the simulation
struct body* simulation__init(char *simulation_name, struct body *bodies, int *n_bodies);
// use case triangle test
struct body* earth_sun__init(struct body *bodies, int *n_bodies);
// use case square test
struct body* square__init(struct body *bodies, int *n_bodies);
// use case triangle test
struct body* triangle__init(struct body *bodies, int *n_bodies);

struct body* random_init(struct body*bodies, int *n_bodies);
// compute the force between two particles
void compute_force(const struct body body1,const struct body body2,double G,double force[]);
// compute the acceleration of a particle
void compute_acceleration(struct body bodies, double force[],double acc[]);

void print_bodies(struct body *bodies,int n_bodies);





#endif