#ifndef _H_SQUARE
#define _H_SQUARE

#define SQUARE_WIDTH 15
#define SQUARE_HEIGHT 15

struct square_t
{
  float x, y;
  float destx, desty;
  float xVel, yVel;
  float r, g, b;
};

void square_set_xy (struct square_t * square, float x, float y);

void square_set_velocity (struct square_t * square, float xv, float yv);

void square_set_destination (struct square_t * square, float destx, float desty);

void square_set_rgb(struct square_t * square, float r, float g, float b);

void square_move(struct square_t * square, int screen_maxx, int screen_maxy);
 
void square_show(struct square_t * square);

#endif

