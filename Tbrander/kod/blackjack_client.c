
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <assert.h>
#include <netinet/in.h>
#include <pthread.h>
#include "lib/server.h"
#include <assert.h>

#define PORTNUM 6578
#define SOCK_PATH "Casino_socket"
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "Projekt Casino"

// --------------------------------------------------------------------------------------------------
#pragma pack(1)
struct card{
    char path[100];
    int type; //(Back piece=0, Hearts=1, Clubbs=2, Diamonds=3, Spades=4)
    int game_value;
    int real_value;
    SDL_Surface* card_img;
    SDL_Rect CardPos;
};
#pragma pack(0)
typedef struct card DECK;

struct player_pos_value{
    int score, x1, y1,x2,y2,bet,tot_holding;
    int hand[11]; // Array som representerar en spelares hand, varje plats innehåller info om ett tilldelat korts värde
    int handPos; // cards in hand position
};                 // Plats [0] är första tilldelade kortet osv.
typedef struct player_pos_value PLAYER;

struct server_threads{
    DECK tdeck;  // thread_deck
    PLAYER tplayer; // thread_player_position_value
    int nthread;
    int n_users;  // the number of users currently connected
    int tconsocket[5]; // the threads own connectionsocket
};
typedef struct server_threads THREAD;


/*FUNKTIONS PROTOTYPER*/
bool loadMedia(DECK ); // Function for loading images unconverted
void SDL_initializer();
void game_running(DECK card,PLAYER [], int , struct sockaddr_in);
void quit(DECK );
void display_text(PLAYER [],int);
void connect_to_server(DECK card, PLAYER usr[]);
void loadCard(DECK );
void checkHandValue(PLAYER [], DECK , int user);
void hitAndStand();
//-------------------------------------------------

/*Global variables*/
SDL_Surface* loadSurface(char* path); //Loads individual image
SDL_Surface* table_img = NULL;        //Loaded converted table image
SDL_Surface* hit_img = NULL;          //Loaded converted hit button image
SDL_Surface* stand_img = NULL;        //Loaded converted stand button image
TTF_Font *font;                       // True type font to be loaded (*.ttf)
SDL_Window* window = NULL;            //The window
SDL_Surface* screen = NULL;           // The window surface
SDL_Surface *text;                    // Score to be printed
SDL_Event event;                      //Event- for user interaction
_Bool running = true;                 // Game loop flag
int client_socket; // global
char table[50]="grafik/casino_v3.bmp",hit_button[50]="grafik/hit.bmp",stand_button[50]="grafik/stand.bmp";
int test;

//************************************ MAIN *********************************************

int main( int argc, char* args[] ) {
    srand(time(NULL)); // Server
    DECK card;     // Klient, server
    PLAYER usr[5];     // Klient, server
    SDL_initializer(); // klient
    if (!loadMedia(card)){ // Calling function for loading 24-bit images in to the memory
        printf("Cant load img.\n");
    }
    connect_to_server(card,usr);




 return 0;
}


void display_text(PLAYER usr[],int playerNr){

    int xPos=usr[playerNr].x1,yPos=usr[playerNr].y1+120; // Every users position stored in xPos, yPos
    char scoreToDisplay[10]={0};
    snprintf(scoreToDisplay,10,"Sum: %d",usr[playerNr].score); // Translate int to string, storing it in scoreToDisplay
    TTF_Init(); // Initialize SDL_ttf
    font = TTF_OpenFont("fonts/DejaVuSans.ttf", 16); // Open true type font
    SDL_Color text_color = {255, 245, 0};
    text = TTF_RenderText_Blended(font,scoreToDisplay,text_color); // Blended = smoother edges, Solid = sharper edges

    SDL_Rect textLocation = { xPos,yPos, 0, 0 }; // Position of text, relativ to user
    SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
    // Free text
    SDL_FreeSurface(text);
    //Close the font
    TTF_CloseFont(font);
}


void game_running(DECK card, PLAYER usr[], int client_socket, struct sockaddr_in dest){
    int x,y;
    int hit = 0; // message to server for hit
    int stand = 1; // message to server for stand
    int bustOrLost = 2; // not using for the moment
    int newGame = 3; // message to server for new game
    int win = 4; // not using for the moment
    int newGameCount = 0; // keeps count on how many times to receive data when new game starts (deal cards function in server)
    bool gamePlay = false; // if true then game is running
    int closeSocketmessage = 666;
    printf("Welcome\n");

    usr[0].score = 0;
    usr[0].handPos = 0;

    SDL_Rect newGameButton;
    Uint32 newGamecolor = SDL_MapRGB(screen->format,0x65,0x33,0x32);

    newGameButton.x = 0;
    newGameButton.y = 0;
    newGameButton.w = 80;
    newGameButton.h = 40;

    SDL_Rect winLoseMessage; // displayed on win or lose
    Uint32 winColor = SDL_MapRGB(screen->format,0xFF,0xFF,0x32); // for the rect color on win
    Uint32 loseColor = SDL_MapRGB(screen->format,0xFF,0x33,0x32); // for the rect color on lose

    winLoseMessage.x = 880; // position on screen
    winLoseMessage.y = 150; // position on screen
    winLoseMessage.w = 80; // width size
    winLoseMessage.h = 40; // height size

    while(running){

        if(gamePlay == false) {
            SDL_FillRect(screen,&newGameButton,newGamecolor); // only show new game button when there is no current hand in play
        }

        SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!

        while( SDL_PollEvent( &event )) {// Check if user is closing the window --> then call quit
             switch( event.type){
                case SDL_QUIT:
                    running = false; // Gameloop flag false
                    send(client_socket, &closeSocketmessage, sizeof(closeSocketmessage), 0);
                    quit(card);
                    close(client_socket);
                    exit(0);
                    break;
                case SDL_MOUSEBUTTONDOWN:// button clicks
                    x = event.button.x; // used to know where on x-axis is currently being clicked
                    y = event.button.y; // used to know where on y-axis is currently being clicked

                    if(x>490 && x< 490+98 && y>530 && y<530+50 && usr[0].score < 21 && gamePlay == true) { // can only be clicked while gameplay is true
                        send(client_socket, &hit, sizeof(hit), 0); // send hit message to server
                        recv(client_socket, &card, sizeof(card), 0); // recv a card struct from server
                        recv(client_socket, &usr[0], sizeof(usr[0]), 0); // recv a card struct from server
                        printf("Player: %d\n", usr[0].score); // print score in command
                        loadCard(card); // load card, display card, free space

                        if(usr[0].score > 21) { // if player busts show new game button
                            gamePlay = false;
                            SDL_FillRect(screen,&winLoseMessage,loseColor);
                        }



                    }

                    if(x>610 && x< 610+98 && y>530 && y<530+50 && usr[0].score <= 21 && gamePlay == true) { // stand button
                        send(client_socket, &stand, sizeof(stand), 0); // send stand message to server
                        while(usr[1].score < 17) { // receive card while server/dealer is less than 17
                            recv(client_socket, &card, sizeof(card), 0); // receive card to be displayed on dealer part of screen
                            recv(client_socket, &usr[1].score, sizeof(usr[1].score), 0); // receive current dealer score
                            printf("Dealer: %d\n", usr[1].score); // print dealer score
                            loadCard(card); // load card, display card, free space
                        }

                        if(usr[0].score > usr[1].score && usr[0].score < 22 || usr[1].score > 21) { // if win show yellow color rect
                            SDL_FillRect(screen,&winLoseMessage,winColor);
                        }
                        if(usr[1].score > usr[0].score && usr[1].score < 22 || usr[1].score == usr[0].score || usr[0].score > 21) { // if lose show red color rect
                            SDL_FillRect(screen,&winLoseMessage,loseColor);
                        }
                        gamePlay = false; // no current game, new game button appears

                    }
                    if(x>0 && x< 0+80 && y>0 && y<0+40 && gamePlay == false) { // new game button

                       usr[0].score = 0; // player score
                       usr[1].score = 0; // dealer score
                       usr[0].handPos = 0; // variable card in hand position of player
                       //SDL_BlitSurface( table_img, NULL, screen, NULL );

                        if (!loadMedia(card)){ // Calling function for loading 24-bit images in to the memory
                            printf("Cant load img.\n");
                        }
                        send(client_socket, &newGame, sizeof(newGame), 0); // new game message to server
                        printf("New Game\n");

                            while(newGameCount < 3) { // first deal of cards

                                recv(client_socket, &card, sizeof(card), 0); // receive card from server
                                loadCard(card);
                                ++newGameCount;
                            }

                            recv(client_socket, &usr[0], sizeof(usr[0]), 0); // receive card from server
                            recv(client_socket, &usr[1], sizeof(usr[1]), 0); // receive card from server
                            newGameCount=0;
                            gamePlay = true;
                            printf("Dealer: %d\n", usr[1].score);
                            printf("Player: %d\n", usr[0].score);
                    }
                    break;
             }

        }
    }

}


void connect_to_server(DECK card, PLAYER usr[]){

    int mess_len;
    char buffer[100]="Not connected!";
    struct sockaddr_in dest;
    int len;
    int j = 0;

    client_socket = socket(AF_INET, SOCK_STREAM, 0); assert(client_socket!=-1);

    memset(&dest, 0, sizeof(dest));                /* zero the struct */
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr("10.0.2.15"); /* set destination IP number */
    dest.sin_port = htons(PORTNUM);                /* set destination port number */

    test=connect(client_socket, (struct sockaddr *)&dest, sizeof(struct sockaddr));   assert(test==0);
    game_running(card,usr,client_socket, dest);
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


SDL_Surface* loadSurface(char* path){ //Function to format the 24bit image to 32bit
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
		 /* ------ Convert the 24bit image to the optimized 32bit ------ */
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


bool loadMedia(DECK card){
    //Loading success flag
    bool success = true;
    int i=0;

    // ------------------------------ LOAD BUTTONS AND GAMEBOARD ---------------------------------

    table_img=loadSurface(table);

    hit_img=loadSurface(hit_button);
    stand_img=loadSurface(stand_button);

    SDL_Rect    hit_Rect;
    SDL_Rect    stand_Rect;

    // Positions for hit button
    hit_Rect.x=490;
    hit_Rect.y=530;
    hit_Rect.w=98;
    hit_Rect.h=50;
    // Positions for stand button
    stand_Rect.x=610;
    stand_Rect.y=530;
    stand_Rect.w=98;
    stand_Rect.h=50;



    // Draw images to screen
    SDL_BlitSurface(table_img, NULL, screen, NULL); // Gameboard
    SDL_BlitScaled(hit_img, NULL, screen, &hit_Rect); // hit button
    SDL_BlitScaled(stand_img, NULL, screen, &stand_Rect); // stand button

    // ----------------------------------------------------------------------------------------------

    if( table_img == NULL ){
        printf( "Failed to load image!\n" );
        success = false;
    }

    SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!
    return success;
}


void quit(DECK card){
    int i;
    //Quit SDL_ttf
    TTF_Quit();
    // Frigör bilder för spelkorten.

    SDL_FreeSurface(table_img); // Frigör bild för Black Jack bordet.
    SDL_FreeSurface(hit_img); // Frigör bild för hit-knapp bordet.
    SDL_FreeSurface(stand_img); // Frigör bild för stand-knapp bordet.
    SDL_DestroyWindow(window);  // Dödar fönstret
    SDL_Quit();
}



void loadCard(DECK card) {
     card.card_img = loadSurface(card.path);
     SDL_BlitScaled(card.card_img, NULL, screen, &card.CardPos);
     SDL_FreeSurface(card.card_img);

}


void checkHandValue(PLAYER usr[], DECK card, int user) { // calculates current hand value
    usr[user].hand[ usr[user].handPos ] = card.game_value; // stores current card value in postion j of hand array
    ++usr[user].handPos;
    usr[user].score +=card.game_value;
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
