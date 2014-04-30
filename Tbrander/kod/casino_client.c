
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
//#include <SDL2/SDL_image.h>
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

struct card{
    char path[100];
    int type; //(Back piece=0, Hearts=1, Clubbs=2, Diamonds=3, Spades=4)
    int game_value;
    int real_value;
    SDL_Surface* card_img;
    SDL_Rect CardPos;
    int card_id;
};
typedef struct card DECK;

struct player_pos_value{
    int score, x1, y1,x2,y2,bet,tot_holding;
    DECK hand[11]; // Array som representerar en spelares hand, varje plats innehåller info om tilldelade kort, färg, värden..
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
bool loadMedia(DECK card[]); // Function for loading images unconverted
void card_init(DECK card[],PLAYER usr[]); // Initialize the card deck
void SDL_initializer();
void game_running(DECK card[],PLAYER usr[]);
void shuffleDeck(DECK card[]);
void quit(DECK card[]);
void display_text(PLAYER usr[],int);
void connect_to_server(DECK card[],PLAYER usr[]);//prototyp
void deal_cards(PLAYER usr[],DECK card[],int id,int playerNr);
void login_init();
//-------------------------------------------------

/*Global variables*/
SDL_Surface* loadSurface(char* path); //Loads individual image
SDL_Surface* table_img = NULL;        //Loaded converted table image
SDL_Surface* hit_img = NULL;          //Loaded converted hit button image
SDL_Surface* stand_img = NULL;        //Loaded converted stand button image
SDL_Surface* bet_img= NULL;

TTF_Font *font;                       // True type font to be loaded (*.ttf)
SDL_Window* window = NULL;            //The window
SDL_Surface* screen = NULL;           // The window surface
SDL_Surface *text;                    // Score to be printed
SDL_Event event;                      //Event- for user interaction
_Bool running = true;                 // Game loop flag
int client_socket; // global
char table[50]="grafik/casino_v3.bmp",hit_button[50]="grafik/hit_button.bmp",stand_button[50]="grafik/stand_button.bmp",bet_button[50]="grafik/bet_button.bmp";
int test; // assert

//************************************ MAIN *********************************************

int main( int argc, char* args[] ) {
    srand(time(NULL)); // Server
    DECK card[60];     // Klient, server
    PLAYER usr[5];     // Klient, server
    card_init(card,usr); // // Klient, server
    login_init();
    SDL_initializer(); // klient
    //shuffleDeck(card); // Server
    if (!loadMedia(card)){ // Calling function for loading 24-bit images in to the memory
        printf("Cant load img.\n");
    }
    //connect_to_server(card,usr);
    game_running(card,usr); // game loop

 return 0;
}
//***************************************************************************************

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

void game_running(DECK card[],PLAYER usr[]){

    while(running){

        while( SDL_PollEvent( &event ) != 0 ) // Check if user is closing the window --> then call quit
          {
             if( event.type == SDL_QUIT ){
                running = false; // Gameloop flag false
                quit(card);
                exit(0);
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

void connect_to_server(DECK card[],PLAYER usr[]){

    int mess_len;
    char buffer[100]="Not connected!";
    struct sockaddr_in dest;
    client_socket = socket(AF_INET, SOCK_STREAM, 0); assert(client_socket!=-1);
    memset(&dest, 0, sizeof(dest));                /* zero the struct */
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr("127.0.0.1"); /* set destination IP number */
    //dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* set destination IP number - localhost, 127.0.0.1*/
    dest.sin_port = htons(PORTNUM);                /* set destination port number */

    test=connect(client_socket, (struct sockaddr *)&dest, sizeof(struct sockaddr));   //assert(test==0);
    /*if (test<0){
        perror("Can't connect to server\n");
        exit(1);
    }*/
    memset(&buffer[0], 0, sizeof(buffer));
    mess_len = recv(client_socket, buffer,100,0);
    printf("%s",buffer);

    /* We have to null terminate the received data ourselves */
    buffer[mess_len] = '\0';

    printf("\nReceived: %s (%d bytes).\n", buffer, mess_len);

}

void login_init(){

    SDL_Window* login_window=NULL;
    SDL_Surface* login_img = NULL;
    int SDL_test;
    char login[50]="grafik/login.bmp";
    SDL_test=SDL_Init(SDL_INIT_VIDEO |SDL_INIT_AUDIO); assert(SDL_test==0);

    if (SDL_test<0){
        printf( "SDL2 could not initialize! SDL2_Error: %s\n", SDL_GetError() );
        exit(1);
    }
    // Login screen
    login_window = SDL_CreateWindow(
    WINDOW_TITLE,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    650,
    500,
    SDL_WINDOW_SHOWN);                  assert(login_window!=NULL);

    screen = SDL_GetWindowSurface(login_window);
    login_img=loadSurface(login);
    SDL_BlitSurface(login_img, NULL, screen, NULL); // login screen
    SDL_UpdateWindowSurface(login_window);
    SDL_FreeSurface(login_img); // Frigör bild

    while(running){

        while( SDL_PollEvent( &event ) != 0 ) // Check if user is closing the window --> then call quit
          {
             if( event.type == SDL_QUIT ){
                 SDL_DestroyWindow(login_window);  // Dödar fönstret
                 running = false; // Gameloop flag false
             }
          }
     }
     running = true;
}


void SDL_initializer(){

    // Gameboard window created
    window = SDL_CreateWindow(
    WINDOW_TITLE,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    SDL_WINDOW_SHOWN);
    screen = SDL_GetWindowSurface(window);

}

SDL_Surface* loadSurface(char* path){ //Function to format the 24bit image to 32bit
	//The final optimized image
	SDL_Surface* optimizedSurface = NULL; // Här lagras den optimerade 32bitars bilden

	//Load image at specified path
	SDL_Surface* loadedSurface = /*IMG_Load(path); */SDL_LoadBMP(path); // IMG_Load för SDL_image, *.PNG-format
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


bool loadMedia(DECK card[]){
    //Loading success flag
    bool success = true;
    int i=0;

    // ------------------------------ LOAD BUTTONS AND GAMEBOARD ---------------------------------

/*
    int imgFlags = IMG_INIT_PNG;

    if( !( IMG_Init( imgFlags ) & imgFlags ) ) {
        printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
        success = false;
        exit(1);
        }
*/
    table_img=loadSurface(table);
    hit_img=loadSurface(hit_button);
    stand_img=loadSurface(stand_button);
    bet_img=loadSurface(bet_button);

    SDL_Rect    hit_Rect;
    SDL_Rect    stand_Rect;
    SDL_Rect    bet_Rect;
    // Positions for hit button
    hit_Rect.x=550;
    hit_Rect.y=530;
    hit_Rect.w=98;
    hit_Rect.h=49;
    // Positions for stand button
    stand_Rect.x=670;
    stand_Rect.y=530;
    stand_Rect.w=98;
    stand_Rect.h=49;

    bet_Rect.x=430;
    bet_Rect.y=530;
    bet_Rect.w=98;
    bet_Rect.h=49;
    // Draw images to screen


    SDL_BlitSurface(table_img, NULL, screen, NULL);         // Gameboard
    SDL_BlitScaled(hit_img, NULL, screen, &hit_Rect);       // hit button
    SDL_BlitScaled(stand_img, NULL, screen, &stand_Rect);   // stand button
    SDL_BlitSurface(bet_img, NULL, screen, &bet_Rect);      // bet button
    // ----------------------------------------------------------------------------------------------

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
    SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!
    return success;
}


void card_init(DECK card [], PLAYER usr[]){
    int i;
    int gameValue = 1;
    int realValue = 1;

    for (i=0;i<54;++i){
        sprintf(card[i].path,"grafik/cards/%d.bmp",i);
        if(i > 1 && i < 10 || i > 14 && i < 23 || i > 27 && i < 36 || i > 40 && i < 49){ // all cards between 2 and 9
            card[i].game_value=gameValue;
            card[i].real_value=realValue;
            card[i].type=i/13 +1;
            card[i].card_id=i;
        }

        if (i>9 && i<14 || i>22 && i<27 || i>35 && i<40 || i>48 && i<53){ // All tens, jacks, queens and kings equal 10
            card[i].game_value = 10;
            card[i].real_value = realValue;
            card[i].type=i/14 +1;
        }

        if (i==1 || i==14 || i==27 || i==40){ // ACE:s
            card[i].game_value=11;
            card[i].type=i/13 +1;
            gameValue = 1;
            realValue = 1;
            card[i].real_value=1;
        }

        ++realValue;
        ++gameValue;
        card[53].real_value=0;card[53].type=0;card[53].game_value=0;
        //printf(" Real Value = %d    Type = %d    Value = %d      Path = %s \n ",card[i].real_value,card[i].type,card[i].game_value, card[i].path);
    }
}



void deal_cards(PLAYER usr[],DECK card[],int playerNr,int id){

    card[id].CardPos.x=usr[playerNr].x1;
    card[id].CardPos.y=usr[playerNr].y1;
    card[id].CardPos.w=70;
    card[id].CardPos.h=106;
    usr[playerNr].score+=card[id].game_value;
    SDL_BlitScaled(card[id].card_img, NULL, screen, &card[id].CardPos);// Draw card image to screen and scale
    display_text(usr,playerNr);// Call display_score
}



void quit(DECK card[]){
    int i;
    //Quit SDL_ttf
    TTF_Quit();
    // Frigör bilder för spelkorten.
    for (i=0;i<53;++i){
        SDL_FreeSurface(card[i].card_img);
    }

    SDL_FreeSurface(table_img); // Frigör bild för Black Jack bordet.
    SDL_FreeSurface(hit_img); // Frigör bild för hit-knapp bordet.
    SDL_FreeSurface(stand_img); // Frigör bild för stand-knapp bordet.
    SDL_DestroyWindow(window);  // Dödar fönstret
    SDL_Quit();
}



