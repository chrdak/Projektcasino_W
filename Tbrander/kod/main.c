#include <SDL2/SDL.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
/* Window resolution */
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 600

/* Window title */
#define WINDOW_TITLE "Projekt Casino"

/* The window */
SDL_Window* window = NULL;

/* The window surface */
SDL_Surface* screen = NULL;

/* The event structure */
SDL_Event event;

//Loads individual image
SDL_Surface* loadSurface();
//Loads media
bool loadMedia();

//The final optimized image
SDL_Surface* optimizedSurface = NULL;

char r6[]="grafik/r6.bmp";
char table[]="grafik/table.bmp";
char back[]="grafik/back.bmp";


/* The game loop flag */
_Bool running = true;

/* to put the loaded image */
SDL_Surface* table_img = NULL;
SDL_Surface* r6_img = NULL;
SDL_Surface* back_img = NULL;

//************************************ MAIN *********************************************

int main( int argc, char* args[] )
{
 if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
 {
   printf( "SDL2 could not initialize! SDL2_Error: %s\n", SDL_GetError() );
 }
 else
 {
   window = SDL_CreateWindow(
       WINDOW_TITLE,
       SDL_WINDOWPOS_CENTERED,
       SDL_WINDOWPOS_CENTERED,
       WINDOW_WIDTH,
       WINDOW_HEIGHT,
       SDL_WINDOW_SHOWN);

   screen = SDL_GetWindowSurface( window );
   /*
   if (!loadMedia){
        printf("Cant load img.\n");
   }
*/
   table_img = SDL_LoadBMP( "grafik/table.bmp" ); // Black Jack bordet
   r6_img = SDL_LoadBMP( "grafik/r6.bmp" ); // ruter 6
   back_img = SDL_LoadBMP( "grafik/back.bmp" ); // upp och ner vänt kort


    SDL_BlitSurface( table_img, NULL, screen, NULL ); // Bord ritas upp

    //SDL_UpdateWindowSurface( window );

while(running)
   {
//Apply the image stretched
				SDL_Rect r6_Rect;
				SDL_Rect blueback_Rect;

				blueback_Rect.x = 280;
				blueback_Rect.y = 315;
				blueback_Rect.w = 59;
				blueback_Rect.h = 95;
				SDL_BlitScaled( back_img, NULL, screen, &blueback_Rect );

				r6_Rect.x = 265;
				r6_Rect.y = 295;
				r6_Rect.w = 59;
				r6_Rect.h = 95;
				SDL_BlitScaled( r6_img, NULL, screen, &r6_Rect );


                /*
                SDL_BlitSurface( r6_img, NULL, screen, NULL );
                SDL_BlitSurface( back_img, &blue_back, screen, NULL );
                */

                //Update the surface
                SDL_UpdateWindowSurface( window );

                sleep(1);

      while( SDL_PollEvent( &event ) != 0 )
      {
         if( event.type == SDL_QUIT )
         {
            running = false;
         }
      }
   }



 }
 SDL_FreeSurface( table_img );
 SDL_FreeSurface( back_img );
 SDL_FreeSurface( r6_img );
 SDL_DestroyWindow( window );
 SDL_Quit();
 return 0;
}
//*************************************************************************************
/*

SDL_Surface* loadSurface(char path[])
{

	//The final optimized image
	SDL_Surface* optimizedSurface = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = SDL_LoadBMP("grafik/table.bmp");
	if( loadedSurface == NULL ){
		printf( "Unable to load image %s! SDL Error: %s\n", path, SDL_GetError() );
	}
	else
	{
		//Convert surface to screen format
		optimizedSurface = SDL_ConvertSurface( loadedSurface, screen->format, NULL );
		if( optimizedSurface == NULL ){
			printf( "Unable to optimize image %s! SDL Error: %s\n", path, SDL_GetError() );
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	return optimizedSurface;
}



bool loadMedia(){
    //Loading success flag
    bool success = true;

    //Load stretching surface
    table_img=loadSurface(table);
    if( table_img == NULL ){
        printf( "Failed to load image!\n" );
        success = false;
    }
    r6_img=loadSurface(r6);
    if( r6_img == NULL ){
        printf( "Failed to load image!\n" );
        success = false;
    }
    back_img=loadSurface(back);
    if( back_img == NULL ){
        printf( "Failed to load image!\n" );
        success = false;
    }

    return success;
}
*/
