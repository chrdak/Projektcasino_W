#include <SDL/SDL.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

void init(SDL_Surface ** screen)
{

  /*The following code does the initialization for Audio and Video*/
  int i_error=SDL_Init( SDL_INIT_EVERYTHING );

  /*If initialization is unsuccessful, then quit */
  if(i_error==-1)
    exit(1);

  atexit(SDL_Quit);

  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

  *screen = SDL_SetVideoMode(640, 480, 24, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_OPENGL );

  if ( *screen == NULL )
  {
    fprintf(stderr, "Couldn't set 640x480x8 video mode: %s.\n",SDL_GetError());
    exit(1);
  }
  
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
  float r, g, b;
  SDL_Surface * screen;
};

void render_ball(struct ball * the_ball)
{
  int k;

  glFlush();

  glBegin(GL_LINES);
  glColor3f(the_ball->r,the_ball->g,the_ball->b);
  for(k=0;k<20;k++)
  {
    glVertex3f(the_ball->x+20*cos(3.1415926*k/10),the_ball->y+20*sin(3.1415926*k/10),0);
    glVertex3f(the_ball->x+20*cos(3.1415926*(k+1)/10),the_ball->y+20*sin(3.1415926*(k+1)/10),0);
  }
  glEnd();

  SDL_GL_SwapBuffers();
  
}

float myabs(float x) { if(x>0)return x; else return -x; }


Uint32 move_ball(Uint32 interval, void * param)
{
  struct ball * the_ball = (struct ball *) param;

  if( myabs(the_ball->x - the_ball->destx) > myabs(the_ball->dx) )
    the_ball->x += the_ball->dx;
  
  if( myabs(the_ball->y - the_ball->desty) > myabs(the_ball->dy) )
    the_ball->y += the_ball->dy;

  render_ball(the_ball);
  
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
  red_ball.startx=50; red_ball.starty=50; red_ball.destx=50; red_ball.desty=50;
  red_ball.x = (float)red_ball.startx; red_ball.y = (float)red_ball.starty;
  red_ball.dx = 0.0; red_ball.dy = 0.0;
  red_ball.r = 1; red_ball.g = 0; red_ball.b = 0;
  red_ball.screen = screen;
  
  blue_ball.startx=100; blue_ball.starty=50; blue_ball.destx=100; blue_ball.desty=50;
  blue_ball.x = (float)blue_ball.startx; blue_ball.y = (float)blue_ball.starty;
  blue_ball.dx = 0.0; blue_ball.dy = 0.0;
  blue_ball.r = 0; blue_ball.g = 0; blue_ball.b = 1;
  blue_ball.screen = screen;

  render_ball(&red_ball);
  render_ball(&blue_ball);

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


