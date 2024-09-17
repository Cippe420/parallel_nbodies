#include "tree.h"
#include <stdlib.h>
#include <stdio.h>
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
    node->body = NULL;
    node->ne = NULL;
    node->se = NULL;
    node->nw = NULL;
    node->sw = NULL;
}


void print_tree(struct node *root)
{
    if(!root){
        return;
        }
    if(!root->nw && !root->sw && !root->ne && !root->se){
        printf("Node %p, Body: %p\n\n",root,root->body);
        return;
    }

    printf("Node: %p , Total Mass: %f, Center of Mass : (%f,%f) \n",root,root->mass,root->com[0],root->com[1]);
    printf("Children : %p, %p, %p, %p\n",root->ne,root->se,root->nw,root->sw);
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
        // TODO: fix the entire mem free...
        root->body = NULL;
        //free(root->body);
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
        // update center of mass
        double oldmass = root->mass;
        root->mass += body->mass;
        root->com[0] = (root->com[0] * oldmass + body->pos[0] * body->mass) / root->mass;
        root->com[1] = (root->com[1] * oldmass + body->pos[1] * body->mass) / root->mass;
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
        // subdivide the region further, creating 4 children
        // initialize center of mass of the node
        // TODO: check for node initialization, memory can be dirty when mallocing stuff...__init_node__
        root->ne = NULL;
        root->ne = (struct node*)calloc(1,sizeof(struct node));
        init_node(root->ne);
        root->se = NULL;    
        root->se = (struct node*)calloc(1,sizeof(struct node));
        init_node(root->se);
        root->nw = NULL;
        root->nw = (struct node*)calloc(1,sizeof(struct node));
        init_node(root->nw);
        root->sw = NULL;
        root->sw = (struct node*)calloc(1,sizeof(struct node));
        init_node(root->sw);
        // seems kinda working(?) gotta check for the center of mass tho...aint even sure man
        // set the root mass, as total mass of those two bodies
        root->mass = body->mass + root->body->mass;
        // take the old body to insert it later (also usefull to access the stuff without calling a double pointer :O)
        struct body *old_body = root->body;
        // get outta here boy
        root->body = NULL;   
        
        // proceed to calc the node center of mass
        root->com[0] = ( (old_body->mass*old_body->pos[0]) + (body->mass*body->pos[0])  ) / root->mass;
        root->com[1] = ((old_body->mass*old_body->pos[1]) + (body->mass*body->pos[1]) ) / root->mass;
        root->minX = fmin(root->minX,body->pos[0]);
        root->minY = fmin(root->minY,body->pos[1]);
        root->maxX = fmax(root->maxX,body->pos[0]);
        root->maxY = fmax(root->maxY,body->pos[1]);


        // recursively insert both b and c into the appropriate quadrant
        // TODO: this means that when it recurses,the root is actively null(?)
        // still recursing
        if (old_body->pos[0] >= root->com[0] && old_body->pos[1] >= root->com[1])
        {
            Tree__insert(root->ne,old_body);
        }
        else if(old_body->pos[0] >= root->com[0] && old_body->pos[1] < root->com[1])
        {
            Tree__insert(root->se,old_body);

        }
        else if(old_body->pos[0] < root->com[0] && old_body->pos[1] >= root->com[1])
        {
            Tree__insert(root->nw,old_body);
        }
        else if (old_body->pos[0] < root->com[0] && old_body->pos[1] < root->com[1]){
            Tree__insert(root->sw,old_body);
        }

        if(body->pos[0] >= root->com[0] && body->pos[1] >= root->com[1])
        {
            Tree__insert(root->ne,body);
        }
        else if(body->pos[0] >= root->com[0] && body->pos[1] < root->com[1])
        {
            Tree__insert(root->se,body);
        }
        else if(body->pos[0] < root->com[0] && body->pos[1] >= root->com[1])
        {
            Tree__insert(root->nw,body);
        }
        else if(body->pos[0] < root->com[1] && body->pos[1] < root->com[1])
        {
            Tree__insert(root->sw,body);
        }

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
    // TODO: SOMEHOW NOT CALCING RIGHT

    double sX = root->maxX - root->minX;
    double sY = root->maxY - root->minX;
    // not another way to get the absolute
    if (sX < 0 ){
        sX*=-1;
    }
    if(sY<0){
        sY*=-1;
    }
    double s = fmax(sX,sY);
    double distanceX = root->com[0] - body->pos[0];
    double distanceY = root->com[1] - body->pos[1];
    double d = sqrt(distanceX * distanceX + distanceY * distanceY);
    double s_d = s/d;
    // if s/d is less than theta, treat the internal node as a single body, and calculate the force it exerts on body b
    // TODO: approssimando perdo l'info del corpo in questione, in particolare, sia N il corpo approssimazione di n1 ed n2
    // se n1 compone la maggior parte di N, calcola la forza che n1 esercita su se stesso,cosa che dio porco non dovrebbe fare
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