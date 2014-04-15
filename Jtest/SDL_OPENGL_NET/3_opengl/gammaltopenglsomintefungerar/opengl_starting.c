//Structuring the animation

#include <SDL/SDL.h>
#include <stdio.h>
#include <GL/glut.h>
#include <math.h>

void init(SDL_Surface ** screen)
{


  /*The following code does the initialization for Audio and Video*/
  int i_error=SDL_Init( SDL_INIT_EVERYTHING );

  /*If initialization is unsuccessful, then quit */
  if(i_error==-1)
    exit(1);

  atexit(SDL_Quit);

  /*   Initialize the display in a 640x480 8-bit palettized mode,
   *   requesting a software surface
   *                                    */

  *screen = SDL_SetVideoMode(640, 480, 8, SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL );

  if ( *screen == NULL )
  {
    fprintf(stderr, "Couldn't set 640x480x8 video mode: %sn",SDL_GetError());
    exit(1);
  }
  
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,        8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,      8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,       8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,      8);

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      16);
  SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,     32);

  SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,  8);
  SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE,    8);
  SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE,    8);

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);

  glClearColor(0, 0, 0, 0);
  glClearDepth(1.0f);
  glViewport(0, 0, 640, 480);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 640, 480, 0, 1, -1);
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_TEXTURE_2D);
  glLoadIdentity();

}

struct ball
{
  int startx, starty;
  int destx, desty;
  float x, y;
  float dx, dy;
};

float myabs(float x) { if(x>0)return x; else return -x; }

Uint32 move_ball(Uint32 interval, void * param)
{
  struct ball * the_ball = (struct ball *) param;

  if( myabs(the_ball->x - the_ball->destx) > myabs(the_ball->dx) )
    the_ball->x += the_ball->dx;
  
  if( myabs(the_ball->y - the_ball->desty) > myabs(the_ball->dy) )
    the_ball->y += the_ball->dy;

  return interval;
}


int main(int argc,char* argv[])
{
  SDL_Surface *screen;
  SDL_Surface *ball_bmp;

  int k;

  struct ball red_ball, blue_ball;

  SDL_TimerID timer, timer2;

  int done;

  init(&screen);

  //This could be put in an init function:
  red_ball.startx=0; red_ball.starty=0; red_ball.destx=0; red_ball.desty=0;
  red_ball.x = (float)red_ball.startx; red_ball.y = (float)red_ball.starty;
  red_ball.dx = 0.0; red_ball.dy = 0.0;

  blue_ball.startx=0; blue_ball.starty=0; blue_ball.destx=0; blue_ball.desty=0;
  blue_ball.x = (float)blue_ball.startx; blue_ball.y = (float)blue_ball.starty;
  blue_ball.dx = 0.0; blue_ball.dy = 0.0;

  glBegin(GL_QUADS);
    glColor3f(1, 0, 0); glVertex3f(0, 0, 0);
    glColor3f(1, 1, 0); glVertex3f(100, 0, 0);
    glColor3f(1, 0, 1); glVertex3f(100, 100, 0);
    glColor3f(1, 1, 1); glVertex3f(0, 100, 0);
  glEnd();

  glBegin(GL_QUADS);
    glColor3f(1, 0, 0); glVertex3f(300, 0, 0);
    glColor3f(1, 1, 0); glVertex3f(500, 0, 0);
    glColor3f(1, 0, 1); glVertex3f(500, 200, 0);
    glColor3f(1, 1, 1); glVertex3f(300, 200, 0);
  glEnd();

  glBegin(GL_LINES);
    glColor3f(1,1,1);
    for(k=0;k<20;k++)
    {
      glVertex3f(400+20*cos(3.1415926*k/10),400+20*sin(3.1415926*k/10),0);
      glVertex3f(400+20*cos(3.1415926*(k+1)/10),400+20*sin(3.1415926*(k+1)/10),0);
    }
  glEnd();


  SDL_GL_SwapBuffers();


  timer = SDL_AddTimer(20, move_ball, &red_ball);
  SDL_Delay(10);
  timer2 = SDL_AddTimer(20, move_ball, &blue_ball);

  /*Handle the keyboards events here. Catch the SDL_Quit event to exit*/
  done = 0;
  while (!done)
  {
    SDL_Event event;

    /* Check for events */
    while (SDL_PollEvent (&event))
    {
      switch (event.type)
      {
        case SDL_KEYDOWN:
	  break;

        case SDL_MOUSEBUTTONDOWN: 
	{
          switch(event.button.button) 
	  {
	    case SDL_BUTTON_LEFT: 
	    {
	      
              red_ball.destx = event.button.x;
	      red_ball.desty = event.button.y;

	      red_ball.dx = (red_ball.destx - red_ball.x)/50.0;
	      red_ball.dy = (red_ball.desty - red_ball.y)/50.0;

	      break;
	    }

	    case SDL_BUTTON_RIGHT:
	    {
              blue_ball.destx = event.button.x;
	      blue_ball.desty = event.button.y;

	      blue_ball.dx = (blue_ball.destx - blue_ball.x)/50.0;
	      blue_ball.dy = (blue_ball.desty - blue_ball.y)/50.0;

	      break;
	    }

	  }
	}
	break;

	case SDL_QUIT:
	  done = 1;
	  break;

	default:
	  break;

      }
    } 
  }
}
