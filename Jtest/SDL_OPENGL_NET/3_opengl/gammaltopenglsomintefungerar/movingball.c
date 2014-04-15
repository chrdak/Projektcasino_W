#include <SDL/SDL.h>
#include <stdio.h>

struct ball
{
  int startx, starty;
  int destx, desty;
  float x, y;
  float dx, dy;
  SDL_Surface *ball_bmp;
  SDL_Surface *screen;
};

float myabs(float x) { if(x>0)return x; else return -x; }

void print_ball(struct ball * the_ball)
{
  printf("x: %5.2f, y: %5.2f\n", the_ball->x, the_ball->y);
  printf("destx: %d, desty: %d\n", the_ball->destx, the_ball->desty);
  printf("dx: %5.2f, dy: %5.2f\n", the_ball->dx, the_ball->dy);

}

Uint32 move_ball(Uint32 interval, void * param)
{
  struct ball * the_ball = (struct ball *) param;

  SDL_Rect clear_rect;

  clear_rect.x = the_ball->x;
  clear_rect.y = the_ball->y;
  clear_rect.w = the_ball->ball_bmp->w;
  clear_rect.h = the_ball->ball_bmp->h;

  SDL_FillRect( the_ball->screen, &clear_rect, 0 );
  SDL_UpdateRect(the_ball->screen, the_ball->x, the_ball->y, the_ball->ball_bmp->w, the_ball->ball_bmp->h);

  if( myabs(the_ball->x - the_ball->destx) > myabs(the_ball->dx) )
    the_ball->x += the_ball->dx;
  
  if( myabs(the_ball->y - the_ball->desty) > myabs(the_ball->dy) )
    the_ball->y += the_ball->dy;

  SDL_Rect whereTo;
  whereTo.x=the_ball->x; whereTo.y=the_ball->y;
  whereTo.w=the_ball->ball_bmp->w; whereTo.h=the_ball->ball_bmp->h;

  if(SDL_BlitSurface(the_ball->ball_bmp, NULL, the_ball->screen, &whereTo) < 0)
    fprintf(stderr, "BlitSurface error: %sn", SDL_GetError());
	    
  SDL_UpdateRect(the_ball->screen, the_ball->x, the_ball->y, the_ball->ball_bmp->w, the_ball->ball_bmp->h);
  

  return interval;
}

int main(int argc,char* argv[])
{
  SDL_Surface *screen;
  SDL_Surface *ball_bmp;

  struct ball red_ball;

  SDL_TimerID timer;

  int done;

  /*The following code does the initialization for Audio and Video*/
  int i_error=SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER );

  /*If initialization is unsuccessful, then quit */
  if(i_error==-1)
    exit(1);

  atexit(SDL_Quit);

  /*   Initialize the display in a 640x480 8-bit palettized mode,
   *   requesting a software surface
   *                                    */

  screen = SDL_SetVideoMode(640, 480, 8, SDL_SWSURFACE);

  if ( screen == NULL )
  {
    fprintf(stderr, "Couldn't set 640x480x8 video mode: %sn",SDL_GetError());
    exit(1);
  }

  /* Load the BMP file into a surface */
  red_ball.ball_bmp = SDL_LoadBMP("ball.bmp");
  if (red_ball.ball_bmp == NULL) {
    fprintf(stderr, "Couldn't load %s: %sn", "ball.bmp", SDL_GetError());
    exit(1);
  }

  /*
   * Palettized screen modes will have a default palette (a standard
   * 8*8*4 colour cube), but if the image is palettized as well we can
   * use that palette for a nicer colour matching.
   * */
			         
  if (red_ball.ball_bmp->format->palette && screen->format->palette) {
    SDL_SetColors(screen, ball_bmp->format->palette->colors, 0, ball_bmp->format->palette->ncolors);
  }

  /* Blit onto the screen surface */
  if(SDL_BlitSurface(red_ball.ball_bmp, NULL, screen, NULL) < 0)
    fprintf(stderr, "BlitSurface error: %sn", SDL_GetError());

  SDL_UpdateRect(screen, 0, 0, red_ball.ball_bmp->w, red_ball.ball_bmp->h);

  //This could be put in an init function:
  red_ball.startx=0; red_ball.starty=0; red_ball.destx=0; red_ball.desty=0;
  red_ball.x = (float)red_ball.startx; red_ball.y = (float)red_ball.starty;
  red_ball.dx = 0.0; red_ball.dy = 0.0;
  red_ball.screen = screen;

  timer = SDL_AddTimer(20, move_ball, &red_ball);

  //printf("So far.\n");

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

	      //SDL_Rect whereTo;
	      //whereTo.x=event.button.x; whereTo.y=event.button.y;
	      //whereTo.w=ball_bmp->w; whereTo.h=ball_bmp->h;
	      
              red_ball.destx = event.button.x;
	      red_ball.desty = event.button.y;

	      red_ball.dx = (red_ball.destx - red_ball.x)/50.0;
	      red_ball.dy = (red_ball.desty - red_ball.y)/50.0;

              print_ball(&red_ball);

              //if(SDL_BlitSurface(ball_bmp, NULL, screen, &whereTo) < 0)
	      //  fprintf(stderr, "BlitSurface error: %sn", SDL_GetError());


              //SDL_UpdateRect(screen, whereTo.x, whereTo.y, ball_bmp->w, ball_bmp->h);

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

  /* Free the allocated surface*/
  SDL_FreeSurface(red_ball.ball_bmp);

}
