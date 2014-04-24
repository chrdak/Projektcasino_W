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
    int real_value;
    SDL_Surface* card_img;
    SDL_Rect CardPos;
};
typedef struct card DECK;

struct player_pos_value{
    int score, x1, y1,x2,y2;
    int hand[12];
    int handPos;
};
typedef struct player_pos_value PLAYER;

/*FUNKTIONS PROTOTYPER*/
bool loadMedia(DECK card[]); // Function for loading images unconverted
void SDL_initializer();
void game_running(DECK card[],PLAYER usr[]);
void shuffleDeck(DECK card[]);
void deal_cards(PLAYER usr[],DECK card[]);

void checkAceValue(PLAYER [], DECK [], int user, int i);

void card_init(DECK card []);



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
    int dcc = 0;
    int pcc = 0;
    int *dealerCardCounter = &dcc; // dealer counter for card in hand
    int *playerCardCounter = &pcc; // player counter for card in hand
    DECK card[54];      // struct array (path,type,value)
    PLAYER usr[2];
    card_init(card);


    SDL_initializer();


    /*
    checkAceValue(usr, card, dealerCardCounter, 0, 27);
    checkAceValue(usr, card, dealerCardCounter, 0, 40);
    printf("%d\n", usr[0].score);
    printf("%d\n", usr[0].handPos);
    */

    shuffleDeck(card);

    if (!loadMedia(card)){ // Calling function for loading 24-bit images in to the memory
        printf("Cant load img.\n");
    }
    SDL_BlitSurface( table_img, NULL, screen, NULL ); // Draw the gametable to the screen
    game_running(card,usr);

    // Free the allocated space
    SDL_FreeSurface(table_img);
    SDL_DestroyWindow( window );
    SDL_Quit();


 return 0;
}
//*************************************************************************************
void deal_cards(PLAYER usr[],DECK card[]){
    int i=0,dealerx=500,dealery=100, playerx=915, playery=390; // card positions on screen
     usr[0].score = 0;
     usr[1].score = 0;
     usr[0].handPos = 0;
     usr[1].handPos = 0;

    for(i=1;i<4;i++){
        // Rectangles for positioning
        if(i==1) { //dealar first card
            card[i].CardPos.x= dealerx;
            card[i].CardPos.y= dealery;
            card[i].CardPos.w=75;
            card[i].CardPos.h=111;
            checkAceValue(usr, card, 0, i);
        }

        if(i>1) { //player first two cards
            card[i].CardPos.x= playerx;
            card[i].CardPos.y= playery;
            card[i].CardPos.w=75;
            card[i].CardPos.h=111;
            playerx += 12;
            playery += 12;
            checkAceValue(usr, card, 1, i);
        }
        SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);// Draw card image to screen and scale

        SDL_UpdateWindowSurface( window );
    }

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
    int l = 0;

    int hand[20]; // hand value storage
    int stop = 0; // boolean value
    int playerx=939, playery=414, dealerx=512,dealery=112; // card position on screen
    int newGame = 0;

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

        SDL_FillRect(screen,&rect,color);
        SDL_FillRect(screen,&rect2,color2);

        if(i == 3) {
            printf("Player: %d\n",usr[1].score );
            ++i;
        }

        if(usr[1].score == 21) {

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
                checkAceValue(usr, card,0, i);
                SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);// Draw card image to screen and scale
                ++i;

                if(usr[0].score > 16) { // set boolean value if dealer have more than 16
                    stop = 1;

                }
                printf("Dealer: %d\n",usr[0].score );
                            //SDL_Delay(100);
                }
                newGame = 1;
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

        while( SDL_PollEvent( &event )) { // Check if user is closing the window --> then call quit

             switch(event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:// checks button click on different rect
                    x = event.button.x;
                    y = event.button.y;

                    if(x>rect.x && x< rect.x+rect.w && y>rect.y && y<rect.y+rect.h && usr[1].score < 21 && newGame == 0) {
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

                        checkAceValue(usr, card, 1, i);

                        if(usr[1].score > 21) {
                            stop = 1; // boolean value
                            newGame = 1;
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

                            checkAceValue(usr, card, 0, i);

                            SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);// Draw card image to screen and scale
                            ++i;

                            if(usr[0].score > 16) { // set boolean value if dealer have more than 16
                                stop = 1;

                            }
                            printf("Dealer: %d\n",usr[0].score );

                        }
                        newGame = 1;
                    }

                    if(x>rect4.x && x< rect4.x+rect4.w && y>rect4.y && y<rect4.y+rect4.h && newGame == 1) {

                        //new game button on upper left corner
                        SDL_BlitSurface( table_img, NULL, screen, NULL );

                        shuffleDeck(card);

                        deal_cards(usr,card);
                        playerx=939;
                        playery=414;
                        dealerx=512;
                        dealery=112;
                        i = 3;

                        newGame = 0;
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


void card_init(DECK card []){
    int i;
    int gameValue = 1;
    int realValue = 1;

    for (i=0;i<54;++i){
        sprintf(card[i].path,"grafik/cards/%d.bmp",i);
        if(i > 1 && i < 10 || i > 14 && i < 23 || i > 27 && i < 36 || i > 40 && i < 49){ // all cards between 2 and 9
            card[i].value=gameValue;
            card[i].real_value=realValue;
            card[i].type=i/13 +1;
        }

        if (i>9 && i<14 || i>22 && i<27 || i>35 && i<40 || i>48 && i<53){ // All tens, jacks, queens and kings equal 10
            card[i].value = 10;
            card[i].real_value = realValue;
            card[i].type=i/14 +1;
        }

        if (i==1 || i==14 || i==27 || i==40){ // ACE:s
            card[i].value=11;
            card[i].type=i/13 +1;
            gameValue = 1;
            realValue = 1;
            card[i].real_value=1;
        }

        ++realValue;
        ++gameValue;
        card[53].real_value=0;card[53].type=0;card[53].value=0;
        printf(" Real Value = %d    Type = %d    Value = %d      Path = %s \n ",card[i].real_value,card[i].type,card[i].value, card[i].path);
    }
}

void checkAceValue(PLAYER usr[], DECK card[], int user, int i) {
    usr[user].hand[ usr[user].handPos ] = card[i].value; // stores current card value in postion j of hand array
    //usr[user].hand[*cardInHand] = card[i].value; // stores current card value in postion j of hand array
    //++*cardInHand;
    ++usr[user].handPos;
    usr[user].score +=card[i].value;
    int l;
    if(usr[user].score > 21) { // if current score is greater than 21
        for(l=0;l<usr[user].handPos+1;l++){ //if current hand has card equal to 11 and player score is greater than 21
            if(usr[user].hand[l] == 11 && usr[user].score > 21){
                usr[user].score -= 10; // decrement total score with 10
                usr[user].hand[l] = 1; // ace in hand gets the value 1
            }

        }
    }
}
