#include <SDL2/SDL.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 650
#define WINDOW_TITLE "Projekt Casino"






/*FUNKTIONS PROTOTYPER*/
bool loadMedia(); // Function for loading images unconverted

//Pathway to bmp-files stored in arrays.
char r6[]="grafik/r6.bmp";
char table[]="grafik/table.bmp";
char back[]="grafik/back.bmp";


//Loads individual image
SDL_Surface* loadSurface(char* path);
SDL_Surface* table_img;
SDL_Surface* r6_img;
SDL_Surface* back_img;
SDL_Window* window = NULL; //The window
SDL_Surface* screen = NULL; // The window surface
SDL_Event event; //Event- When user closes the window

_Bool running = true; // Game loop flag

/*Where the optimized image is stored, 32-bit*/
SDL_Surface* table_img = NULL;
SDL_Surface* r6_img = NULL;
SDL_Surface* back_img = NULL;

//************************************ MAIN *********************************************

int main( int argc, char* args[] )
{

 const int FPS=30;
 Uint32 startValue;

 if( SDL_Init( SDL_INIT_VIDEO |SDL_INIT_AUDIO ) < 0 ) // Initialize video and audio
 {
   printf( "SDL2 could not initialize! SDL2_Error: %s\n", SDL_GetError() );
 }
 else
 { // Window created with measurments defined globally, centered
   window = SDL_CreateWindow(
       WINDOW_TITLE,
       SDL_WINDOWPOS_CENTERED,
       SDL_WINDOWPOS_CENTERED,
       WINDOW_WIDTH,
       WINDOW_HEIGHT,
       SDL_WINDOW_SHOWN);

   screen = SDL_GetWindowSurface( window );

   if (!loadMedia()){ // Calling function for loading 24-bit images in to the memory
        printf("Cant load img.\n");
   }

   SDL_BlitSurface( table_img, NULL, screen, NULL ); // Draw the gametable to the screen

while(running)
   {
                startValue = SDL_GetTicks(); //Start frame timer

                // Rectangles for positioning
				SDL_Rect r6_Rect; // Clubs of 6
				SDL_Rect blueback_Rect; // Backpiece

				blueback_Rect.x = 280;
				blueback_Rect.y = 315;
				blueback_Rect.w = 59;
				blueback_Rect.h = 95;
				SDL_BlitSurface( back_img, NULL, screen, &blueback_Rect ); // Draw card image to screen

				r6_Rect.x = 265;
				r6_Rect.y = 295;
				r6_Rect.w = 59;
				r6_Rect.h = 95;
                SDL_BlitSurface( r6_img, NULL, screen, &r6_Rect); // Draw card image to screen

                //Update the surface
                SDL_UpdateWindowSurface( window );

                sleep(1);

      while( SDL_PollEvent( &event ) != 0 ) // Check if user is closing the window --> then call quit
      {
         if( event.type == SDL_QUIT )
         {
            running = false; // Gameloop flag false
         }
      }

      if(1000/FPS>SDL_GetTicks()-startValue){ //Check if frame is produced within 33.33 milliseconds --> if not then delay
        SDL_Delay(1000/FPS-(SDL_GetTicks()-startValue));
      }
   }



 }
 // Free the allocated space
 SDL_FreeSurface( table_img );
 SDL_FreeSurface( back_img );
 SDL_FreeSurface( r6_img );
 SDL_DestroyWindow( window );
 SDL_Quit();
 return 0;
}
//*************************************************************************************


SDL_Surface* loadSurface(char* path) //Function to format the 24bit image to 32bit
{
	//The final optimized image
	SDL_Surface* optimizedSurface = NULL; // Här lagras den optimerade 32bitars bilden

	//Load image at specified path
	SDL_Surface* loadedSurface = SDL_LoadBMP(path);
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL Error: %s\n", path, SDL_GetError() );
	}
	else
	{
		//Convert surface to screen format
		optimizedSurface = SDL_ConvertSurface( loadedSurface, screen->format, SDL_SWSURFACE );
		if( optimizedSurface == NULL )
		{
			printf( "Unable to optimize image %s! SDL Error: %s\n", path, SDL_GetError() );
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface ); // Släng 24-bit
	}

	return optimizedSurface;
}



bool loadMedia(){
    //Loading success flag
    bool success = true;

    /* ------ Convert the 24bit image to the optimized 32bit ------ */

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

