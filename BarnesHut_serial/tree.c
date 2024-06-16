#include "tree.h"
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>

// build the Barnes-Hut tree

void init_node(struct node *node)
{
    node->com[0] = 0;
    node->com[1] = 0;
    node->mass = 0;
    node->minX = 0;
    node->minY = 0;
    node->maxX = 0;
    node->maxY = 0;
    node->ne = NULL;
    node->se = NULL;
    node->nw = NULL;
    node->sw = NULL;
}


void print_tree(struct node *root)
{
    if(!root){return;}
    
    
    
    print_tree(root->ne);
    print_tree(root->se);
    print_tree(root->nw);
    print_tree(root->sw);
}

void Tree__free(struct node* root){
    if(!root){
        return;
    }
    Tree__free(root->ne);
    Tree__free(root->se);
    Tree__free(root->nw);
    Tree__free(root->sw);
    if(root->body != NULL){
        root->body = NULL;
    }
    free(root);
}


void Tree__insert(struct node *root,struct body *body){

    // if node x does not contain a body, put the new body b here
    if(!root->body && !root->ne && !root->se && !root->nw && !root->sw){
        
        root->minX = body->pos[0];
        root->minY = body->pos[1];
        root->maxX = body->pos[0];
        root->maxY = body->pos[1];
        root->body = body;
        return;
    }

    // if node x is an internal node:
    if (!root->body && root->ne != NULL && root->se !=NULL && root->nw != NULL && root->sw != NULL)
    {
        /*
        // TODO: resetting mass on inserting breaks it into loop
        // save temporary data to update the current node
        double oldcomX = root->com[0];
        double oldcomY = root->com[1];
        double oldmass = root->mass;
        // update the center-of-mass and total mass of x
        root->mass += body->mass;
        root->com[0] = ( oldcomX * oldmass + body->pos[0]*body->mass) / root->mass;
        root->com[1] = (oldcomY * oldmass + body->pos[1] * body->mass) / root->mass;
        */
        root->mass += body->mass;
        root->com[0] = (root->com[0] * root->mass + body->pos[0] * body->mass) / root->mass;
        root->com[1] = (root->com[1] * root->mass + body->pos[1] * body->mass) / root->mass;


        // update min and max coordinates
        root->minX = fmin(root->minX,body->pos[0]);
        root->minY = fmin(root->minY,body->pos[1]);
        root->maxX = fmax(root->maxX,body->pos[0]);
        root->maxY = fmax(root->maxY,body->pos[1]);


        // recursively insert the body b in the appropriate quadrant
        if (body->pos[0] >= root->com[0] && body->pos[1] >= root->com[1])
        {
            
            Tree__insert(root->ne,body);
        }
        else if (body->pos[0] >= root->com[0] && body->pos[1] < root->com[1])
        {
            
            Tree__insert(root->se,body);
        }
        else if (body->pos[0] < root->com[0] && body->pos[1] >= root->com[1])
        {
            
            Tree__insert(root->nw,body);
        }
        else if (body->pos[0] < root->com[0] && body->pos[1] < root->com[1])
        {
            
            Tree__insert(root->sw,body);
        }
        return;
    }

    // if node x is an external node (containing a body named c):
    if (root->body != NULL && !root->nw && !root->sw && !root->ne && !root->se)
    {
        // calculate min and max coordinates of the region

        // subdivide the region further, creating 4 children
        // initialize center of mass of the node
        
        root->mass = root->body->mass + body->mass;
        root->com[0] = (root->body->pos[0] * root->body->mass + body->pos[0] * body->mass) / root->mass;
        root->com[1] = (root->body->pos[1] * root->body->mass + body->pos[1] * body->mass) / root->mass;
        root->ne = NULL;
        root->ne = (struct node*)malloc(sizeof(struct node));
          
        root->se = NULL;    
        root->se = (struct node*)malloc(sizeof(struct node));
        
        root->nw = NULL;
        root->nw = (struct node*)malloc(sizeof(struct node));
        
        root->sw = NULL;
        root->sw = (struct node*)malloc(sizeof(struct node));
        
        // 
        struct body *old_body = root->body;
        root->body = NULL;   
        root->mass = 0;
        // recursively insert both b and c into the appropriate quadrant
        
        Tree__insert(root,old_body);
        Tree__insert(root,body);
        return;
    }

}

// TODO: calculate force of the system on a single particle
void Tree__calculate_force(struct node *root, struct body *body, double theta,double *force,double G){
    // if the current node is an external node (and not the body we are calculating the force on)
    // calculate the force exerted by the current node on body b, and add this amount to b's net force
    if(root->body != NULL && !root->ne && !root->se && !root->nw && !root->sw){
        // if the current node is the body we are calculating the force on,return
        if(root->body == body){
            return;
        }
        // calculate the force exerted by the current node on body b, and add this amount to b's net force
        
        
        compute_force(*body,*root->body,G,force);
        return;
    }

    // if the current node is an internal node
    // calculate the ratio s/d
    double s = root->maxX - root->minX;
    double distanceX = root->com[0] - body->pos[0];
    double distanceY = root->com[1] - body->pos[1];
    double d = sqrt(distanceX * distanceX + distanceY * distanceY);
    double s_d = abs(s/d);

    // if s/d is less than theta, treat the internal node as a single body, and calculate the force it exerts on body b
    if (s_d < theta){
        // build a fake body using the node's center of mass and total mass
        struct body fake_body;
        fake_body.mass = root->mass;
        fake_body.pos[0] = root->com[0];
        fake_body.pos[1] = root->com[1];
        // calculate the force exerted by the current node on body b, and add this amount to b's net force
        compute_force(*body,fake_body,G,force);
        return;
    }
    // otherwise, run the procedure recursively on each of the current node's children

    if(root->ne != NULL){
        Tree__calculate_force(root->ne,body,theta,force,G);
    }
    if(root->se != NULL){
        Tree__calculate_force(root->se,body,theta,force,G);
    }
    if(root->nw != NULL){
        Tree__calculate_force(root->nw,body,theta,force,G);
    }
    if(root->sw != NULL){
        Tree__calculate_force(root->sw,body,theta,force,G);
    }

    return;
}