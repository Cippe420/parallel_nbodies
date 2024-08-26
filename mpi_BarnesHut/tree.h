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

void print_tree(struct node *root);

void getBoundCoordinates(struct body *bodies, int n_bodies, double *minx, double *miny, double *maxx, double *maxy);

void init_node(struct node *node);

void insert_node(struct node *root,struct body *body);

void build_tree(struct node *root, struct body *bodies, int n_bodies, double minx, double miny, double maxx, double maxy);

void tree__insert(struct node *root, struct body *body);

void Tree__free(struct node *root);

void Node__free(struct node *root);

void Tree__insert(struct node *root, struct body *body);

void Tree__merge( struct node *root, struct node *recv_node);

void Tree__calculate_force(struct node *root, struct body *body, double theta,double *force, double G);


#endif