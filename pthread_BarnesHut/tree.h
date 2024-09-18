#ifndef __TREE_H__
#define __TREE_H__
#include "../serial_nbodies/particles.h"

struct node
{
    double com[2];
    double mass;
    double minX;
    double minY;
    double maxX;
    double maxY;
    struct body *body;
    struct node *ne;
    struct node *se;
    struct node *nw;
    struct node *sw;
};
// unused
void calculate_force(struct node *root, struct body *body, double theta, double *force,double G);
// print the whole tree (debug purposes)
void print_tree(struct node *root);
// unused
void getBoundCoordinates(struct body *bodies, int n_bodies, double *minx, double *miny, double *maxx, double *maxy);
// initialize a node
void init_node(struct node *node);
// unused
void insert_node(struct node *root,struct body *body);
// unused
void build_tree(struct node *root, struct body *bodies, int n_bodies, double minx, double miny, double maxx, double maxy);
// unused
void tree__insert(struct node *root, struct body *body);
// hopefully the final version of the tree insert
void insert__Tree(struct node *root, struct body *body);
// free the whole memory allocated for the tree
void Tree__free(struct node *root);
// free a single node
void Node__free(struct node *root);
// insert a node into the tree
void Tree__insert(struct node *root, struct body *body);
// calculate total force acting on a body using the BH-tree
void Tree__calculate_force(struct node *root, struct body *body, double theta,double G,double *force );


#endif