#include <SDL2/SDL.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "Projekt Casino"

struct card{
    char path[100];
    int type; //(Back piece=0, Hearts=1, Clubbs=2, Diamonds=3, Spades=4)
    int value;
    SDL_Surface* card_img;
    SDL_Rect CardPos;
};
typedef struct card DECK;


/*FUNKTIONS PROTOTYPER*/
bool loadMedia(DECK card[]); // Function for loading images unconverted
void card_init(DECK card[]); // Initialize the card deck
void SDL_initializer();
void game_running(DECK card[]);
void shuffleDeck(DECK card[]);
//-------------------------------------------------

/*Global variables*/
SDL_Surface* loadSurface(char* path); //Loads individual image
SDL_Surface* table_img = NULL;        //Loaded converted image
SDL_Window* window = NULL;            //The window
SDL_Surface* screen = NULL;           // The window surface
SDL_Event event;                      //Event- When user closes the window
_Bool running = true;                 // Game loop flag

char table[50]="grafik/casino_v2.bmp";

//************************************ MAIN *********************************************

int main( int argc, char* args[] ) {
    srand(time(NULL));
    DECK card[60];      // struct array (path,type,value)
    card_init(card);
    SDL_initializer();
    shuffleDeck(card);

    if (!loadMedia(card)){ // Calling function for loading 24-bit images in to the memory
        printf("Cant load img.\n");
    }
    SDL_BlitSurface( table_img, NULL, screen, NULL ); // Draw the gametable to the screen

    game_running(card);


 // Free the allocated space
 SDL_FreeSurface(table_img);
 //SDL_FreeSurface( back_img );
 //SDL_FreeSurface( r6_img );
 SDL_DestroyWindow( window );
 SDL_Quit();
 return 0;
}
//*************************************************************************************

void game_running(DECK card[]){
    int i=0, random_card=0;
    while(running){
        // Rectangles for positioning
        card[4].CardPos.x=500;
        card[4].CardPos.y=400;
        card[4].CardPos.w=75;
        card[4].CardPos.h=111;
        SDL_BlitScaled(card[4].card_img, NULL, screen, &card[4].CardPos);
        //SDL_BlitSurface( card[3].card_img, NULL, screen, &card[3].CardPos); // Draw card image to screen
        ++i;
        //Update the surface
        SDL_UpdateWindowSurface( window );

        sleep(2);

          while( SDL_PollEvent( &event ) != 0 ) // Check if user is closing the window --> then call quit
          {
             if( event.type == SDL_QUIT )
             {
                running = false; // Gameloop flag false
             }
          }
     }
}

void shuffleDeck(DECK card[]){
    int i,j;
    DECK tmp[60];
    for (i=1; i<53;++i){
        j= rand()%52;
        tmp[i]=card[i];
        card[i]=card[j];
        card[j]=tmp[i];
    }
}


void SDL_initializer(){

    if( SDL_Init( SDL_INIT_VIDEO |SDL_INIT_AUDIO ) < 0 ) // Initialize video and audio
        {
        printf( "SDL2 could not initialize! SDL2_Error: %s\n", SDL_GetError() );
        exit(1);
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
    }

}



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



bool loadMedia(DECK card[]){
    //Loading success flag
    bool success = true;
    int i=0;

    /* ------ Convert the 24bit image to the optimized 32bit ------ */

    table_img=loadSurface(table);
    if( table_img == NULL ){
        printf( "Failed to load image!\n" );
        success = false;
    }

    for(i=0;i<54;++i){
        card[i].card_img=loadSurface(card[i].path);
        if( card[i].card_img == NULL ){
            printf( "Failed to load image!\n");
            success = false;
        }
    }
    return success;
}



void card_init(DECK card[]){
    int i=0,j=0;
    char tmp[5];
    for (i=0;i<53;++i){
        strcpy(card[i].path,"grafik/cards/");
        snprintf(tmp,5,"%d",i);
        strcat(card[i].path,tmp);
        strcat(card[i].path,".bmp");
        //printf("%s\n",card[i].path);
    }

    card[0].value=0; card[0].type=0; // Value and type for the back piece (blue)
    card[53].value=0; card[53].type=0; // Value and type for the back piece (red)

    for (i=1;i<53;++i){
        // Hearts
        if(i<10){ // 0-9 (0=back)
            card[i].value=j;
            ++j;
            card[i].type=1; // Hearts =1
        }
        else if (i>9 && i<14){ //10-13
            j=1;
            card[i].value=10;
            card[i].type=1; // Hearts =1
            }

        //Clubbs
        else if(i>13 && i < 23){ // 14-22
            card[i].value=j;
            ++j;
            card[i].type=2; // Clubbs =2
        }
        else if(i>22 && i<27){ // 23-26
            j=1;
            card[i].value=10;
            card[i].type=2; // Clubbs =2
            }
        //Diamonds
        else if(i>26 && i < 36){ // 27-35
            card[i].value=j;
            ++j;
            card[i].type=3; // Diamonds =3
            }
        else if(i>35 && i<40){ // 36-39
            j=1;
            card[i].value=10;
            card[i].type=3; // Diamonds =3
            }
            //Spades
        else if(i>39 && i < 49){ // 40-48
            card[i].value=j;
            ++j;
            card[i].type=4; // Spades =4
            }
        else if(i>48 && i<53){ // 49-52
            j=1;
            card[i].value=10;
            card[i].type=4; // Spades =4
            }
        //printf("%d\n",card[i].value);
    }

}
