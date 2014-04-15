#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include "timer.h"
#include "square.h"


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int SCREEN_BPP = 32;

const int FRAMES_PER_SECOND = 60;

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
    square_move (&red_square, SCREEN_WIDTH, SCREEN_HEIGHT);
    square_move (&blue_square, SCREEN_WIDTH, SCREEN_HEIGHT);
 
    //Clear the screen
    glClear( GL_COLOR_BUFFER_BIT );
 
    //Show the squares
    square_show(&red_square);
    square_show(&blue_square);

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


