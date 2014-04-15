#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int SCREEN_BPP = 32;

const int FRAMES_PER_SECOND = 60;

const int SQUARE_WIDTH = 15;
const int SQUARE_HEIGHT = 15;

int init_GL()
{
    //Set clear color
    glClearColor( 0, 0, 0, 0 );
    //Set projection
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1, 1 );
    //Initialize modelview matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    //If there was any errors
    if( glGetError() != GL_NO_ERROR )
    {
        return 0;
    }

    return 1;
}
 
int init()
{
    if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
    {
        return 0;
    }
 
    if( SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_OPENGL ) == NULL )
    {
        return 0;
    }
 
    if( init_GL() == 0 )
    {
        return 0;
    }
 
    SDL_WM_SetCaption( "OpenGL Test", NULL );
 
    return 1;
 
}
 
void clean_up()
{
    SDL_Quit();
}
 

struct square_t
{
  float x, y;
  float destx, desty;
  float xVel, yVel;
  float r, g, b;
};

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
  square->destx = destx;
  square->desty = desty;

  square_set_velocity(square,(square->destx - square->x) / 50.0, (square->desty - square->y) / 50.0 );

}

void square_set_rgb(struct square_t * square, float r, float g, float b)
{
  square->r = r;
  square->g = g;
  square->b = b;
}

void move(struct square_t * square)
{
    //Move the square left or right
    square->x += square->xVel;
 
    //If the square went too far
    if( ( square->x < 0 ) || ( square->x + SQUARE_WIDTH > SCREEN_WIDTH ) )
    {
        //Move back
        square->x -= square->xVel;
    }
 
    //Move the square up or down
    square->y += square->yVel;
 
    //If the square went too far
    if( ( square->y < 0 ) || ( square->y + SQUARE_HEIGHT > SCREEN_HEIGHT ) )
    {
        //Move back
        square->y -= square->yVel;
    }
}
 

void show(struct square_t * square)
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

struct timer_t2 
{
  int startTicks;
  int pausedTicks;
  int paused;
  int started;
};

void timer_init (struct timer_t2 * timer)
{
  timer->startTicks=0;
  timer->pausedTicks=0;
  timer->paused=0;
  timer->started=0;
};

void timer_start(struct timer_t2 * timer)
{
    //Start the timer
    timer->started = 1;
    //Unpause the timer
    timer->paused = 0;
    //Get the current clock time
    timer->startTicks = SDL_GetTicks();
};
 
void timer_stop(struct timer_t2 * timer)
{
    //Stop the timer
    timer->started = 0;
    //Unpause the timer
    timer->paused = 0;
};
 
void timer_pause(struct timer_t2 * timer)
{
    //If the timer is running and isn't already paused 
    if( ( timer->started == 1 ) && ( timer->paused == 0 ) )
    {
        //Pause the timer
        timer->paused = 1;
        //Calculate the paused ticks
        timer->pausedTicks = SDL_GetTicks() - timer->startTicks;
    }
};
 
void timer_unpause(struct timer_t2 *timer)
{
    //If the timer is paused
    if( timer->paused == 1 )
    {
        //Unpause the timer
        timer->paused = 0;
        //Reset the starting ticks
        timer->startTicks = SDL_GetTicks() - timer->pausedTicks;
        //Reset the paused ticks
        timer->pausedTicks = 0;
    }
};
 
int timer_get_ticks(struct timer_t2 *timer)
{
    //If the timer is running
    if( timer->started == 1 )
    {
        //If the timer is paused
        if( timer->paused == 1 )
        {
            //Return the number of ticks when the timer was paused
            return timer->pausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            return SDL_GetTicks() - timer->startTicks;
        }
    }
 
    //If the timer isn't running
    return 0;
 
};
 
int timer_is_started(struct timer_t2 *timer)
{
    return timer->started;
};
 
int timer_is_paused(struct timer_t2 *timer)
{
    return timer->paused;
};

int main( int argc, char *argv[] )
{
 
  int quit = 0;

  //Initialize 
  if( init() == 0 )
    return 1;

  SDL_Event event;

  struct square_t red_square;

  square_set_xy (&red_square, 100, 100);
  square_set_destination (&red_square, 100, 100);
  square_set_velocity (&red_square,0,0);
  square_set_rgb (&red_square, 1, 0, 0);

  struct square_t blue_square;

  square_set_xy (&blue_square, 200, 100);
  square_set_destination (&blue_square, 200, 100);
  square_set_velocity(&blue_square, 0, 0);
  square_set_rgb(&blue_square, 0, 0, 1);


  //The frame rate regulator
  struct timer_t2 fps;
 
  //Wait for user exit
 
  while( quit == 0 )
  {
    //Start the frame timer
    timer_start(&fps);
 
    //While there are events to handle
    while( SDL_PollEvent( &event ) )
    {
      //Handle events:

      switch(event.type)
      {
        case SDL_MOUSEBUTTONDOWN:
	  switch(event.button.button)
	  {
	    case SDL_BUTTON_LEFT: // Change direction of red square
	      square_set_destination ( &red_square, event.button.x, event.button.y);
	    break;

	    case SDL_BUTTON_RIGHT: // Change direction of blue square
	      square_set_destination ( &blue_square, event.button.x, event.button.y );
	    break;
	  }
	
	break;
      }

      if( event.type == SDL_QUIT )
      {
        quit = 1;
      }
 
    }
  
    //Move the squares
    move(&red_square);
    move(&blue_square);
 
    //Clear the screen
    glClear( GL_COLOR_BUFFER_BIT );
 
    //Show the squares
    show(&red_square);
    show(&blue_square);

    glFlush();
 
    //Update screen 
    SDL_GL_SwapBuffers();
 
    //Capture the frame rate
    if( timer_get_ticks(&fps) < 1000 / FRAMES_PER_SECOND )
    {
      SDL_Delay( ( 1000 / FRAMES_PER_SECOND ) - timer_get_ticks(&fps) );
    }
 
  }
 

  clean_up();
 
  return 0;
 
}


