
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

struct card{
    char path[100];
    int type; //(Back piece=0, Hearts=1, Clubbs=2, Diamonds=3, Spades=4)
    int game_value;
    int real_value;
    SDL_Surface* card_img;
    SDL_Rect CardPos;
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
void display_text(PLAYER usr[],DECK card[],int);
void connect_to_server();//prototyp
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
    connect_to_server();
    srand(time(NULL)); // Server
    DECK card[60];     // Klient, server
    PLAYER usr[5];     // Klient, server
    card_init(card,usr); // // Klient, server
    SDL_initializer(); // klient
    shuffleDeck(card); // Server
    if (!loadMedia(card)){ // Calling function for loading 24-bit images in to the memory
        printf("Cant load img.\n");
    }
    game_running(card,usr); // game loop

 return 0;
}
//***************************************************************************************

void display_text(PLAYER usr[],DECK card[],int playerNr){

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

void connect_to_server(){

    int mess_len;
    char buffer[100]="Not connected!";
    struct sockaddr_in dest;
    client_socket = socket(AF_INET, SOCK_STREAM, 0); assert(client_socket!=-1);

    memset(&dest, 0, sizeof(dest));                /* zero the struct */
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr("192.168.0.30"); /* set destination IP number */
    //dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* set destination IP number - localhost, 127.0.0.1*/
    dest.sin_port = htons(PORTNUM);                /* set destination port number */

    test=connect(client_socket, (struct sockaddr *)&dest, sizeof(struct sockaddr));   assert(test==0);
    mess_len = recv(client_socket, buffer, 100,0);

    /* We have to null terminate the received data ourselves */
    buffer[mess_len] = '\0';

    printf("\nReceived: %s (%d bytes).\n", buffer, mess_len);

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


bool loadMedia(DECK card[]){
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


void card_init(DECK card[], PLAYER usr[]){
    int i=0,j=1,h=10;
    char tmp[5];
    for (i=0;i<54;++i){
        strcpy(card[i].path,"grafik/cards/");
        snprintf(tmp,5,"%d",i);
        strcat(card[i].path,tmp);
        strcat(card[i].path,".bmp");
    }

    card[0].game_value=0; card[0].type=0; // Value and type for the back piece (blue)
    card[53].game_value=0; card[53].type=0; // Value and type for the back piece (red)

    for (i=1;i<53;++i){
        // Hearts
        if(i<10){ // 1-9
            card[i].game_value=j;
            ++j;
            card[i].type=1; // Hearts =1
        }
        else if (i>9 && i<14){ //10-13
            j=1;
            card[i].game_value=10;
            card[i].type=1; // Hearts =1
            }

        //Clubbs
        else if(i>13 && i < 23){ // 14-22
            card[i].game_value=j;
            ++j;
            card[i].type=2; // Clubbs =2
        }
        else if(i>22 && i<27){ // 23-26
            j=1;
            card[i].game_value=10;
            card[i].type=2; // Clubbs =2
            }
        //Diamonds
        else if(i>26 && i < 36){ // 27-35
            card[i].game_value=j;
            ++j;
            card[i].type=3; // Diamonds =3
            }
        else if(i>35 && i<40){ // 36-39
            j=1;
            card[i].game_value=10;
            card[i].type=3; // Diamonds =3
            }
            //Spades
        else if(i>39 && i < 49){ // 40-48
            card[i].game_value=j;
            ++j;
            card[i].type=4; // Spades =4
            }
        else if(i>48 && i<53){ // 49-52
            j=1;
            card[i].game_value=10;
            card[i].type=4; // Spades =4
            }
    }
    // Här ges klädda kort sina riktiga värden
    for(i=10;i<4;++i){
        card[i].real_value=h;
        ++h;
    }
    h=10;
    for(i=23;i<4;++i){
        card[i].real_value=h;
        ++h;
    }
    h=10;
    for(i=36;i<4;++i){
        card[i].real_value=h;
        ++h;
    }
    h=10;
    for(i=49;i<4;++i){
        card[i].real_value=h;
        ++h;
    }
    // ------------------------------------------


    // Black Jack-värde och riktigt värde för alla Ess
    card[1].game_value=11;card[1].real_value=14;
    card[14].game_value=11;card[14].real_value=14;
    card[27].game_value=11;card[27].real_value=14;
    card[40].game_value=11;card[40].real_value=14;

    // Initializeing card positions for each player, from the left.
    //Dealer
    usr[0].x1=500; usr[0].y1=90;
    usr[0].x2=600; usr[0].y2=90;
    // Player 1
    usr[1].x1=160; usr[1].y1=300;
    usr[1].x2=240; usr[1].y2=300;
    //Player 2
    usr[2].x1=380; usr[2].y1=350;
    usr[2].x2=460; usr[2].y2=350;
    // Player 3
    usr[3].x1=600; usr[3].y1=350;
    usr[3].x2=680; usr[3].y2=350;
    // Player 4
    usr[4].x1=840; usr[4].y1=300;
    usr[4].x2=920; usr[4].y2=300;
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



