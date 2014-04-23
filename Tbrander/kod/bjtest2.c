#include <SDL2/SDL.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include<X11/X.h>
#include<X11/Xlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>



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
    int hand[12];
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

char table[50]="grafik/casino_v2.bmp";

//************************************ MAIN *********************************************

int main( int argc, char* args[] ) {
    srand(time(NULL));
    DECK card[60];      // struct array (path,type,value)
    PLAYER usr[2];
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
    int i=0,dealerx=500,dealery=100, playerx=915, playery=390; // card positions on screen
    int j = 0; // dealer counter for card in hand
    int k = 0; // player counter for card in hand
     usr[0].score = 0;
     usr[1].score = 0;
    for(i=1;i<4;i++){
        // Rectangles for positioning
        if(i==1) { //delar first card
            card[i].CardPos.x= dealerx;
            card[i].CardPos.y= dealery;
            card[i].CardPos.w=75;
            card[i].CardPos.h=111;
            usr[0].score += card[i].value;
            usr[0].hand[j] += card[i].value;
            ++j;

        }

        if(i>1) { //player first two cards
            card[i].CardPos.x= playerx;
            card[i].CardPos.y= playery;
            card[i].CardPos.w=75;
            card[i].CardPos.h=111;
            playerx += 12;
            playery += 12;
            usr[1].score +=card[i].value;
            usr[1].hand[k] = card[i].value;
            ++k;
        }
        SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);// Draw card image to screen and scale

        SDL_UpdateWindowSurface( window );
    }

       /* for(i=5;i<11;i++){
        // Rectangles for positioning
        card[i].CardPos.x=usr[j].x2;
        card[i].CardPos.y=usr[j].y2;
        card[i].CardPos.w=75;
        card[i].CardPos.h=111;
        SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);// Draw card image to screen and scale
        ++j;
        //SDL_UpdateWindowSurface( window );
   }
   */
SDL_UpdateWindowSurface( window );
    //Update the surface

}

void game_running(DECK card[],PLAYER usr[]){
    Uint32 color = SDL_MapRGB(screen->format,0xFF,0xFF,0x32); //color
    Uint32 color2 = SDL_MapRGB(screen->format,0xFF,0x33,0x32);
    Uint32 color3 = SDL_MapRGB(screen->format,0x65,0x33,0x32);

    int x = 0; //x position
    int y = 0; //y position
    int i = 3; // start value for next card
    int j = 1;
    int k = 2; // player hand count
    int l = 0;
    int hand[20]; // hand value storage
    int stop = 0; // boolean value
    int playerx=939, playery=414, dealerx=512,dealery=112; // card position on screen
    //button hit
    SDL_Rect rect;
    rect.x = 550;
    rect.y = 550;
    rect.w = 80;
    rect.h = 40;
    //button stay
    SDL_Rect rect2;
    rect2.x = 640;
    rect2.y = 550;
    rect2.w = 80;
    rect2.h = 40;
    //rect3 for win or lose. Red = lost, yellow = win
    SDL_Rect rect3;
    rect3.x = 915;
    rect3.y = 200;
    rect3.w = 80;
    rect3.h = 40;

    SDL_Rect rect4;
    rect4.x = 0;
    rect4.y = 0;
    rect4.w = 80;
    rect4.h = 40;


    deal_cards(usr,card);

    while(running){


        if(stop == 0){

                SDL_FillRect(screen,&rect,color);
                SDL_FillRect(screen,&rect2,color2);

        }
        if(stop == 1) {//Rules for player win
                if(usr[1].score > usr[0].score && usr[1].score < 22 || usr[0].score > 21 ) {
                                SDL_FillRect(screen,&rect3,color);

                                playerx=939;
                                playery=414;
                                dealerx=512;
                                dealery=112;
                                SDL_FillRect(screen,&rect4,color3);


                }//Rules for player lost
                if(usr[0].score > usr[1].score && usr[0].score < 22 || usr[0].score == usr[1].score || usr[1].score > 21) {
                                SDL_FillRect(screen,&rect3,color2);
                                playerx=939;
                                playery=414;
                                dealerx=512;
                                dealery=112;
                                SDL_FillRect(screen,&rect4,color3);


                }
        }

                SDL_UpdateWindowSurface( window );

            //SDL_BlitSurface( table_img, NULL, screen, NULL ); // Draw the gametable to the screen

            //SDL_UpdateWindowSurface( window );
            //SDL_Delay(50);


        while( SDL_PollEvent( &event )) { // Check if user is closing the window --> then call quit

             switch(event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:// checks button click on different rect
                    x = event.button.x;
                    y = event.button.y;

                    if(x>rect.x && x< rect.x+rect.w && y>rect.y && y<rect.y+rect.h) {
                        if(i>52) {
                            shuffleDeck(card); // shuffle deck and start next card in position 1
                            i=1;

                        }

                        card[i].CardPos.x= playerx;//set card pos
                        card[i].CardPos.y= playery;//set card pos
                        card[i].CardPos.w=75;
                        card[i].CardPos.h=111;
                        playerx += 12;//change card pos
                        playery += 12;//change card pos



                        usr[1].hand[k] = card[i].value; // stores current card value in postion k of hand array
                        ++k;



                        usr[1].score +=card[i].value; // adds card value to score


                        if(usr[1].score > 21) { // if current score is greater than 21
                           for(l=0;l<k+1;l++){ //if current hand has card equal to 11 and player score is greater than 21
                                if(usr[1].hand[l] == 11 && usr[1].score > 21){
                                    usr[1].score -= 10; // decrement total score with 10
                                    usr[1].hand[l] = 1; // ace in hand gets the value 1
                                }

                           }
                        }

                        if(usr[1].score > 21) {
                            stop = 1; // boolean value
                        }
                        SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);// Draw card image to screen and scale
                        ++i;
                        printf("Player: %d\n",usr[1].score );

                    }

                    if(x>rect2.x && x< rect2.x+rect2.w && y>rect2.y && y<rect2.y+rect2.h) {
                        //dealer
                        while(usr[0].score < 17 && stop == 0) {
                            if(i>52) {
                                shuffleDeck(card);
                                i=1;

                            }
                            card[i].CardPos.x= dealerx;
                            card[i].CardPos.y= dealery;
                            card[i].CardPos.w=75;
                            card[i].CardPos.h=111;
                            dealerx += 12;
                            dealery += 12;


                            usr[0].score +=card[i].value;
                            SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);// Draw card image to screen and scale
                            ++i;

                            if(usr[0].score > 16) { // set boolean value if dealer have more than 16
                                stop = 1;

                            }
                            printf("Dealer: %d\n",usr[0].score );
                            //SDL_Delay(100);
                        }
                    }

                    if(x>rect4.x && x< rect4.x+rect4.w && y>rect4.y && y<rect4.y+rect4.h) {

                        //new game button on upper left corner
                        SDL_BlitSurface( table_img, NULL, screen, NULL );
                        shuffleDeck(card);
                        deal_cards(usr,card);
                        playerx=939;
                        playery=414;
                        dealerx=512;
                        dealery=112;

                        int j = 1;
                        int k = 2;

                        stop = 0;

                    }
                    break;

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

    }

     card[1].value=11;
    card[14].value=11;
    card[27].value=11;
    card[40].value=11;


    // Initializeing card positions for each player.
    //Dealer


}
