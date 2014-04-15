#include <SDL/SDL.h>
#include <stdio.h>

int main(int argc,char* argv[])
{
  SDL_Surface *screen;
  SDL_Surface *ball_bmp;
  int stop, done;

  /*The following code does the initialization for Audio and Video*/
  int i_error=SDL_Init(SDL_INIT_VIDEO);

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
  ball_bmp = SDL_LoadBMP("ball.bmp");
  if (ball_bmp == NULL) {
    fprintf(stderr, "Couldn't load %s: %sn", "ball.bmp", SDL_GetError());
    exit(1);
  }

  /*
   * Palettized screen modes will have a default palette (a standard
   * 8*8*4 colour cube), but if the image is palettized as well we can
   * use that palette for a nicer colour matching.
   * */
			         
  if (ball_bmp->format->palette && screen->format->palette) {
    SDL_SetColors(screen, ball_bmp->format->palette->colors, 0, ball_bmp->format->palette->ncolors);
  }

  /* Blit onto the screen surface */
  if(SDL_BlitSurface(ball_bmp, NULL, screen, NULL) < 0)
    fprintf(stderr, "BlitSurface error: %sn", SDL_GetError());

  SDL_UpdateRect(screen, 0, 0, ball_bmp->w, ball_bmp->h);

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
	      SDL_Rect whereTo;
	      whereTo.x=event.button.x; whereTo.y=event.button.y;
	      whereTo.w=ball_bmp->w; whereTo.h=ball_bmp->h;
	      
              if(SDL_BlitSurface(ball_bmp, NULL, screen, &whereTo) < 0)
	        fprintf(stderr, "BlitSurface error: %sn", SDL_GetError());

              SDL_UpdateRect(screen, whereTo.x, whereTo.y, ball_bmp->w, ball_bmp->h);

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
  SDL_FreeSurface(ball_bmp);

}
