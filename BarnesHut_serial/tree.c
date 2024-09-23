#include "tree.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// build the Barnes-Hut tree

// calculate the ratio s/d
double calculate_ratio(struct node *node, struct body *body)
{
    double sX = node->maxX - node->minX;
    double sY = node->maxY - node->minY;

    // handcraft the absolute
    if (sX < 0)
    {
        sX *= -1;
    }
    if (sY < 0)
    {
        sY *= -1;
    }
    double s = fmax(sX, sY);
    double distanceX = node->com[0] - body->pos[0];
    double distanceY = node->com[1] - body->pos[1];
    double d = sqrt(distanceX * distanceX + distanceY * distanceY);
    return s / d;
}

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


void print_node(struct node *root)
{
    if (!root)
    {
        return;
    }
    if (!root->nw && !root->sw && !root->ne && !root->se){
        printf("\033[0;31m Node: %p , Total Mass: %f, Center of Mass : (%f,%f) \033[0m \n", root, root->mass, root->com[0], root->com[1]);
        printf("\033[0;35m Children : %p, %p, %p, %p \033[0m \n", root->ne, root->se, root->nw, root->sw);
        printf("\033[0;36m minX: %f, minY: %f,\n maxX: %f, maxY: %f \033[0m \n", root->minX, root->minY, root->maxX, root->maxY);
        printf(" Body: %p\n\n",root->body);
    }
    else
    {
        // print the parent in green
        printf("\033[0;32m Node: %p , Total Mass: %f, Center of Mass : (%f,%f) \033[0m \n", root, root->mass, root->com[0], root->com[1]);    
        printf("\033[0;35m Children : %p, %p, %p, %p \033[0m \n", root->ne, root->se, root->nw, root->sw);
        printf(" Body: %p\n",root->body);        
        printf("\033[0;36m minX: %f, minY: %f,\n maxX: %f, maxY: %f \033[0m \n\n", root->minX, root->minY, root->maxX, root->maxY);

    }
}

void print_tree(struct node *root)
{
    if(!root){return;}
    print_node(root);
    print_tree(root->ne);
    print_tree(root->se);
    print_tree(root->nw);
    print_tree(root->sw);
}

void Tree__free(struct node *root)
{
    if (!root)
    {
        return;
    }
    Tree__free(root->ne);
    Tree__free(root->se);
    Tree__free(root->nw);
    Tree__free(root->sw);
    if (root->body != NULL)
    {
        // TODO: fix the entire mem free...
        root->body = NULL;
        // free(root->body);
    }
    free(root);
}

void insert_inPlace(struct node *root,struct body *body)
{
    // having found a body, insert the two bodies on that node child
        // recursively insert the body in the appropriate quadrant
    if (body->pos[0] >= root->com[0] && body->pos[1] >= root->com[1])
    {
        root->ne->body = body;
        return;
    }
    else if (body->pos[0] >= root->com[0] && body->pos[1] < root->com[1])
    {
        root->se->body = body;
        return;
    }
    else if (body->pos[0] < root->com[0] && body->pos[1] >= root->com[1])
    {
        root->nw->body = body;
        return;
    }
    else if (body->pos[0] < root->com[0] && body->pos[1] < root->com[1])
    {
        root->sw->body = body;
        return;
    }

}

void insert__Tree(struct node *root, struct body *body)
{
    // if node x is a leaf, insert the body
    if (!root->ne && !root->se && !root->nw && !root->sw)
    {
        
        // if there is no body, insert the body and return
        if (!root->body)
        {
            root->body = body;
            return;
        }
        // if there is a body, update the data of the node and insert both bodies in the appropriate quadrant
        struct body *old_body = root->body;
        root->body = NULL;
        root->mass = old_body->mass + body->mass;
        root->com[0] = ((old_body->mass * old_body->pos[0]) + (body->mass * body->pos[0])) / root->mass;
        root->com[1] = ((old_body->mass * old_body->pos[1]) + (body->mass * body->pos[1])) / root->mass;


        root->minX = fmin(old_body->pos[0], body->pos[0]);
        root->minY = fmin(old_body->pos[1], body->pos[1]);
        root->maxX = fmax(old_body->pos[0], body->pos[0]);
        root->maxY = fmax(old_body->pos[1], body->pos[1]);

        // create the 4 subspaces
        root->ne = calloc(1, sizeof(struct node));
        root->se = calloc(1, sizeof(struct node));
        root->nw = calloc(1, sizeof(struct node));
        root->sw = calloc(1, sizeof(struct node));

        // maybe i dont have to check for inserting here
        insert_inPlace(root,body);
        insert_inPlace(root,old_body);
    }
    else
    {

        // if node x is an internal node, update the center of mass and total mass, min and max coordinates
        if (root->mass == 0)
        {
            root->mass = body->mass;
        }
        else
        {
            root->mass += body->mass;
            root->com[0] = (root->com[0] * root->mass + body->pos[0] * body->mass) / (root->mass + body->mass);
            root->com[1] = (root->com[1] * root->mass + body->pos[1] * body->mass) / (root->mass + body->mass);

            root->minX = fmin(root->minX, body->pos[0]);
            root->minY = fmin(root->minY, body->pos[1]);
            root->maxX = fmax(root->maxX, body->pos[0]);
            root->maxY = fmax(root->maxY, body->pos[1]);
        }   
    
        // recursively insert the body in the appropriate quadrant
        if (body->pos[0] >= root->com[0] && body->pos[1] >= root->com[1])
        {
            insert__Tree(root->ne, body);
            return;
        }
        else if (body->pos[0] >= root->com[0] && body->pos[1] < root->com[1])
        {
            insert__Tree(root->se, body);
            return;
        }
        else if (body->pos[0] < root->com[0] && body->pos[1] >= root->com[1])
        {
            insert__Tree(root->nw, body);
            return;
        }
        else if (body->pos[0] < root->com[0] && body->pos[1] < root->com[1])
        {
            insert__Tree(root->sw, body);
            return;
        }

    }
    return;
}


void Tree__calculate_force(struct node *root,struct body *body, double theta, double G, double *force)
{

    // sono su una foglia
    if (!root->ne && !root->nw && !root->se && !root->sw)
    {

        if (!root->body)
        {
            return;
        }
        if (root->body == body)
        {
            return;
        }
        else
        {
            compute_force(*body, *root->body, G, force);
        }

    }
    else
    {  
        double ratio = calculate_ratio(root, body);
        if (ratio <= theta)
        {
            // se il rapporto è minore di theta, approssima questo nodo come fosse un corpo e calcolane la forza
            struct body fake_body;
            fake_body.mass = root->mass;
            fake_body.pos[0] = root->com[0];
            fake_body.pos[1] = root->com[1];
            compute_force(*body, fake_body, G, force);
            return;
        }
        else
        {
            // altrimenti chiama ricorsivamente su i 4 figli se ci sono corpi dentro
            if(root->ne && (root->ne->mass > 0 || root->ne->body != NULL))
            {
            Tree__calculate_force(root->ne, body, theta, G, force);
            }
            if(root->se && (root->se->mass > 0 || root->se->body != NULL))
            {
            Tree__calculate_force(root->se, body, theta, G, force);
            }
            if(root->nw && (root->nw->mass > 0 || root->nw->body != NULL))
            {
            Tree__calculate_force(root->nw, body, theta, G, force);
            }
            if(root->sw && (root->sw->mass > 0 || root->sw->body != NULL))
            {
            Tree__calculate_force(root->sw, body, theta, G, force);
            }
            return;
        }
    }

}