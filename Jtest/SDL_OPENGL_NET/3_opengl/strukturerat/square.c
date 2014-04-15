#ifndef _SQUARE_C
#define _SQUARE_C

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include "square.h"
#include <math.h>

float myabs (float x)
{
  if (x>0) return x; else return -x;
}

float sq_dist (float x1, float y1, float x2, float y2)
{
  return (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2);
}


void square_set_xy (struct square_t * square, float x, float y)
{
  square->x = x;
  square->y = y;
}

void square_set_velocity (struct square_t * square, float xv, float yv)
{
  square->xVel = xv;
  square->yVel = yv;
}

void square_set_destination (struct square_t * square, float destx, float desty)
{
  float normal_vector_length;

  square->destx = destx;
  square->desty = desty;

  normal_vector_length = sqrt(sq_dist(square->x, square->y, square->destx, square->desty));

  square_set_velocity(square,(square->destx - square->x) / normal_vector_length, 
    (square->desty - square->y) / normal_vector_length );

}

void square_set_rgb(struct square_t * square, float r, float g, float b)
{
  square->r = r;
  square->g = g;
  square->b = b;
}

void square_move(struct square_t * square, int screen_maxx, int screen_maxy)
{
    //Move the square left or right
    square->x += square->xVel;
 
    //If the square went too far
    if( ( square->x < 0 ) || ( square->x + SQUARE_WIDTH > screen_maxx ) )
    {
        //Move back
        square->x -= square->xVel;
    }
 
    //Move the square up or down
    square->y += square->yVel;
 
    //If the square went too far
    if( ( square->y < 0 ) || ( square->y + SQUARE_HEIGHT > screen_maxy ) )
    {
        //Move back
        square->y -= square->yVel;
    }

    if(sq_dist(square->x, square->y, square->destx,square->desty)<sq_dist(square->xVel,square->yVel,0,0)+1)
    {
      square->xVel = square->yVel = 0;
    }

}
 

void square_show(struct square_t * square)
{
    //Move to offset
    glTranslatef( square->x, square->y, 0 );
 
    glBegin( GL_QUADS );
        //Set color to white
        glColor4f( square->r, square->g, square->b, 1.0 );
 
        //Draw square
        glVertex3f( 0,            0,             0 );
        glVertex3f( SQUARE_WIDTH, 0,             0 );
        glVertex3f( SQUARE_WIDTH, SQUARE_HEIGHT, 0 );
        glVertex3f( 0,            SQUARE_HEIGHT, 0 );

    glEnd();
 
    //Reset
    glLoadIdentity();
 
}

#endif
