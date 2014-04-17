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

struct player_pos_value{
    int score, x1, y1,x2,y2;
};
typedef struct player_pos_value PLAYER;

/*FUNKTIONS PROTOTYPER*/
bool loadMedia(DECK card[]); // Function for loading images unconverted
void card_init(DECK card[],PLAYER usr[]); // Initialize the card deck
void SDL_initializer();
void game_running(DECK card[],PLAYER usr[]);
void shuffleDeck(DECK card[]);
void deal_cards(PLAYER usr[],DECK card[]);
//-------------------------------------------------

/*Global variables*/
SDL_Surface* loadSurface(char* path); //Loads individual image
SDL_Surface* table_img = NULL;        //Loaded converted image
SDL_Window* window = NULL;            //The window
SDL_Surface* screen = NULL;           // The window surface
SDL_Event event;                      //Event- When user closes the window
_Bool running = true;                 // Game loop flag

char table[50]="grafik/casino_v3.bmp";

//************************************ MAIN *********************************************

int main( int argc, char* args[] ) {
    srand(time(NULL));
    DECK card[60];      // struct array (path,type,value)
    PLAYER usr[5];
    card_init(card,usr);
    SDL_initializer();
    shuffleDeck(card);

    if (!loadMedia(card)){ // Calling function for loading 24-bit images in to the memory
        printf("Cant load img.\n");
    }
    SDL_BlitSurface( table_img, NULL, screen, NULL ); // Draw the gametable to the screen
    game_running(card,usr);


 // Free the allocated space
 SDL_FreeSurface(table_img);
 //SDL_FreeSurface( back_img );
 //SDL_FreeSurface( r6_img );
 SDL_DestroyWindow( window );
 SDL_Quit();
 return 0;
}
//*************************************************************************************
void deal_cards(PLAYER usr[],DECK card[]){
    int i=0,j=0;
    for(i=0;i<5;++i){
        // Rectangles for positioning
        card[i].CardPos.x=usr[j].x1;
        card[i].CardPos.y=usr[j].y1;
        card[i].CardPos.w=70;
        card[i].CardPos.h=106;
        SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);// Draw card image to screen and scale
        ++j;
        SDL_UpdateWindowSurface(window);
    }
    j=0;
        for(i=5;i<10;++i){
        // Rectangles for positioning
        card[i].CardPos.x=usr[j].x2;
        card[i].CardPos.y=usr[j].y2;
        card[i].CardPos.w=75;
        card[i].CardPos.h=111;
        SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);// Draw card image to screen and scale
        ++j;
        SDL_UpdateWindowSurface(window);
   }

SDL_UpdateWindowSurface(window);
    //Update the surface

}

void game_running(DECK card[],PLAYER usr[]){
    while(running){
        deal_cards(usr,card);
        sleep(2);
        while( SDL_PollEvent( &event ) != 0 ) // Check if user is closing the window --> then call quit
          {
             if( event.type == SDL_QUIT ){
                running = false; // Gameloop flag false
             }
          }
     }
}

void shuffleDeck(DECK card[]){
    int i,j;
    DECK tmp[60];
    for (i=1; i<53;++i){
        j= rand()%52+1;
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


void card_init(DECK card[], PLAYER usr[]){
    int i=0,j=1;
    char tmp[5];
    for (i=0;i<54;++i){
        strcpy(card[i].path,"grafik/cards/");
        snprintf(tmp,5,"%d",i);
        strcat(card[i].path,tmp);
        strcat(card[i].path,".bmp");
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
    }

    // Initializeing card positions for each player, from the left.
    //Dealer
    usr[0].x1=500; usr[0].y1=90;
    usr[0].x2=600; usr[0].y2=90;
    // Player 1
    usr[1].x1=160; usr[1].y1=350;
    usr[1].x2=240; usr[1].y2=350;
    //Player 2
    usr[2].x1=380; usr[2].y1=400;
    usr[2].x2=460; usr[2].y2=400;
    // Player 3
    usr[3].x1=600; usr[3].y1=400;
    usr[3].x2=680; usr[3].y2=400;
    // Player 4
    usr[4].x1=840; usr[4].y1=350;
    usr[4].x2=920; usr[4].y2=350;


}
