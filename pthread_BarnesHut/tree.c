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

// given two bodies,found on the same node, divide them 
void insert_inPlace(struct node *root,struct body *body)
{
    double centreX,centreY;
    centreX = (root->minX + root->maxX)/2;
    centreY = (root->minY + root->maxY)/2;
    // having found a body, insert the two bodies on that node child, creating the node if it's not present
    if (body->pos[0] >= root->com[0] && body->pos[1] >= root->com[1])
    {
        if (!root->ne)
        {
            root->ne = calloc(1, sizeof(struct node));
            root->ne->minX = centreX;
            root->ne->minY = centreY;
            root->ne->maxX = root->maxX;
            root->ne->maxY = root->maxY;
        }
        root->ne->body = body;
        return;
    }
    else if (body->pos[0] >= root->com[0] && body->pos[1] < root->com[1])
    {   
        if (!root->se)
        {
            root->se = calloc(1, sizeof(struct node));
            root->se->minX = centreX;
            root->se->minY = root->minY;
            root->se->maxX = root->maxX;
            root->maxY = centreY;            
        }
        root->se->body = body;
        return;
    }
    else if (body->pos[0] < root->com[0] && body->pos[1] >= root->com[1])
    {
        if(!root->nw)
        {
            root->nw = calloc(1, sizeof(struct node));
            root->nw->minX = root->minX;
            root->nw->minY = centreY;
            root->nw->maxX = centreX;
            root->nw->maxY = root->maxY;
            
        }
        root->nw->body = body;
        return;
    }
    else if (body->pos[0] < root->com[0] && body->pos[1] < root->com[1])
    {
        if(!root->sw)
        {
            root->sw = calloc(1, sizeof(struct node));
            root->sw->minX = root->minX;
            root->sw->minY = root->minY;
            root->sw->maxX = centreX;
            root->sw->maxY = centreY;
        }
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


        // update subspace limit positions
        if (old_body->pos[0] <= body ->pos[0])
        {
            root->minX = old_body->pos[0];
            root->maxX = body->pos[0];
        }
        else
        {
            root->maxX = old_body->pos[0];
            root->minX = body->pos[0];
        }
        if(old_body->pos[1] <= body->pos[1])
        {
            root->minY = old_body->pos[1];
            root->maxY = body->pos[1];
        }
        else
        {
            root->minY = body->pos[1];
            root->maxY = old_body->pos[1];
        }

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

            
            // update subspace limits while inserting
            if (body->pos[0] < root->minX)
            {
                root->minX = body->pos[0];
            }
            if (body->pos[0] > root->maxX)
            {
                root->maxX = body->pos[0];
            }
            if (body->pos[1] < root->minY)
            {
                root->minY = body->pos[1];
            }
            if (body->pos[1] > root->maxY)
            {
                root->maxY = body->pos[1];
            }

        }   
    
        // recursively insert the body in the appropriate quadrant , creating the subspace if needed
        if (body->pos[0] >= root->com[0] && body->pos[1] >= root->com[1])
        {
            if (!root->ne)
            {
                root->ne = calloc(1, sizeof(struct node));
            }
            insert__Tree(root->ne, body);
            return;
        }
        else if (body->pos[0] >= root->com[0] && body->pos[1] < root->com[1])
        {
            if(!root->se)
            {
                root->se = calloc(1, sizeof(struct node));
            }
            insert__Tree(root->se, body);
            return;
        }
        else if (body->pos[0] < root->com[0] && body->pos[1] >= root->com[1])
        {
            if (!root->nw)
            {
                root->nw = calloc(1, sizeof(struct node));
            }
            insert__Tree(root->nw, body);
            return;
        }
        else if (body->pos[0] < root->com[0] && body->pos[1] < root->com[1])
        {
            if(!root->sw)
            {
                root->sw = calloc(1, sizeof(struct node));            
            }
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
        //printf("sono arrivato alla foglia con il corpo: %p\n",root->body);

        if (!root->body)
        {
            return;
        }
        if (root->body == body)
        {
            return;
        }
        else if (root->body != NULL)
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
            struct body fake_body = {0};
            fake_body.mass = root->mass;
            fake_body.pos[0] = root->com[0];
            fake_body.pos[1] = root->com[1];
            //printf("Ho approssimato con massa : %f \n posizioni: %f,%f\n\n",fake_body.mass,fake_body.pos[0],fake_body.pos[1]);
            compute_force(*body, fake_body, G, force);
            return;
        }
        else
        {
            // altrimenti chiama ricorsivamente su i 4 figli se ci sono corpi dentro
            if(root->ne)
            {
                Tree__calculate_force(root->ne, body, theta, G, force);
            }
            if(root->se)
            {
                Tree__calculate_force(root->se, body, theta, G, force);
            }
            if(root->nw)
            {
                Tree__calculate_force(root->nw, body, theta, G, force);
            }
            if(root->sw)
            {
                Tree__calculate_force(root->sw, body, theta, G, force);
            }
            return;
        }
    }

}



void modified_insert(struct node * root, struct body *body)
{
    /*
        Given the bounds of the whole area : (a,b) (c,d) --> a-c are the x and b-d are the y

        the 4 subspaces have the coordinates:

            - nE -> (a+c)/2 , (b+d)/2 - (c),(d)
            - sE -> (a+c)/2 , (b) - (c),(d+b)/2
            - nW -> (a) , (b+d)/2 - (a+c)/2,(d)
            - sW -> (a) , (b) - (a+c)/2,(b+d)/2

     */

    // i'm on a leaf,insert the body here
    if (!root->ne && !root->se && !root->nw && ! root->sw)
    {
        if (!root->body)
        {
            root->body = body;
            return;
        }else
        {
            // i found a body on a leaf, calculate the 4 subspaces coordinates and create nodes accordingly
            // be careful when bodies are on the axis subdivision
            struct body *old_body = root->body;
            root->body = NULL;
            root->mass = old_body->mass + body->mass;
            root->com[0] = ((old_body->mass * old_body->pos[0]) + (body->mass * body->pos[0])) / root->mass;
            root->com[1] = ((old_body->mass * old_body->pos[1]) + (body->mass * body->pos[1])) / root->mass;
            // insert based on center of mass of the two bodies, but update subspaces limits normally
            insert_inPlace(root,body);
            insert_inPlace(root,old_body);
        }
        
    }else
    {
        // else, update total mass and center of mass, based on coordinates decide where to move
        root->mass += body->mass;
        root->com[0] = (root->com[0] * root->mass + body->pos[0] * body->mass) / (root->mass + body->mass);
        root->com[1] = (root->com[1] * root->mass + body->pos[1] * body->mass) / (root->mass + body->mass);
        // for clarity, i could use root->ne->(minX,minY) , but i want the in clear sight in my current iteration
        double centreX,centreY;
        centreX = (root->minX + root->maxX)/2;
        centreY = (root->minY + root->maxY)/2;

        // axis incidents should be handled by equals, node initialization should also be handled here
        if (body->pos[0] >= centreX && body->pos[1] >=centreY)
        {
            if (!root->ne)
            {   
                root->ne = calloc(1, sizeof(struct node));
                root->ne->minX = centreX;
                root->ne->minY = centreY;
                root->ne->maxX = root->maxX;
                root->ne->maxY = root->maxY;
            }
            modified_insert(root->ne,body);
        }
        else if (body->pos[0] >= centreX && body->pos[0] < centreY)
        {
            if(!root->se)
            {
                root->se = calloc(1, sizeof(struct node));
                root->se->minX = centreX;
                root->se->minY = root->minY;
                root->se->maxX = root->maxX;
                root->maxY = centreY;
            }
            modified_insert(root->se,body);
        }
        else if (body->pos[0]<centreX && body->pos[0] >= centreY)
        {
            if (!root->nw)
            {
                root->nw = calloc(1, sizeof(struct node));
                root->nw->minX = root->minX;
                root->nw->minY = centreY;
                root->nw->maxX = centreX;
                root->nw->maxY = root->maxY;
            }
            modified_insert(root->nw,body);
        }
        else if(body->pos[0]<centreX && body->pos[1] < centreY)
        {
            if (!root->sw)
            {
                root->sw = calloc(1, sizeof(struct node));
                root->sw->minX = root->minX;
                root->sw->minY = root->minY;
                root->sw->maxX = centreX;
                root->sw->maxY = centreY;
            }
            modified_insert(root->sw,body);
        }

    }

    return;
}