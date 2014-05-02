
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
    int score, xPos,yPos,bet,tot_holding;
};
typedef struct player_pos_value PLAYER;

struct server_threads{
    int nthread;
    int n_users;  // the number of users currently connected
    int tconsocket[5]; // the threads own connectionsocket
};
typedef struct server_threads THREAD;

struct nClient{
    DECK hand[15];
    PLAYER player;
    THREAD nUser;
};
typedef struct nClient NCLIENT;



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
    //login_init();
    connect_to_server(card,usr);
    SDL_initializer(); // klient

    if (!loadMedia(card)){ // Calling function for loading 24-bit images in to the memory
        printf("Cant load img.\n");
    }

    game_running(card,usr); // game loop

 return 0;
}
//***************************************************************************************

void display_text(PLAYER usr[],int playerNr){

    int x=usr[playerNr].xPos,y=usr[playerNr].yPos+120; // Every users position stored in xPos, yPos
    char scoreToDisplay[10]={0};
    snprintf(scoreToDisplay,10,"Sum: %d",usr[playerNr].score); // Translate int to string, storing it in scoreToDisplay
    TTF_Init(); // Initialize SDL_ttf
    font = TTF_OpenFont("fonts/DejaVuSans.ttf", 16); // Open true type font
    SDL_Color text_color = {255, 245, 0};
    text = TTF_RenderText_Blended(font,scoreToDisplay,text_color); // Blended = smoother edges, Solid = sharper edges

    SDL_Rect textLocation = { x,y, 0, 0 }; // Position of text, relativ to user
    SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
    // Free text
    SDL_FreeSurface(text);
    //Close the font
    TTF_CloseFont(font);
}




void game_running(DECK card, PLAYER usr[], struct sockaddr_in dest){
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

                            //HIT BUTTON
                    if(x>550 && x< 550+98 && y>530 && y<530+49 && usr[0].score < 21 && gamePlay == true) { // can only be clicked while gameplay is true
                        send(client_socket, &hit, sizeof(hit), 0); // send hit message to server
                        recv(client_socket, &card, sizeof(card), 0); // recv a card struct from server
                        recv(client_socket, &usr[0], sizeof(usr[0]), 0); // recv a usr struct from server
                        printf("Player: %d\n", usr[0].score); // print score in command
                        loadCard(card); // load card, display card, free space

                        if(usr[0].score > 21) { // if player busts show new game button
                            gamePlay = false;
                            SDL_FillRect(screen,&winLoseMessage,loseColor);
                        }
                        display_text(usr);
                    }
                        // STAND BUTTON
                    if(x>670 && x< 670+98 && y>530 && y<530+49 && usr[0].score <= 21 && gamePlay == true) { // stand button
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

                        if (!loadMedia(card)){ // Calling function for loading 24-bit images in to the memory
                            printf("Cant load img.\n");
                        }
                        send(client_socket, &newGame, sizeof(newGame), 0); // new game message to server
                        printf("New Game\n");

                            while(newGameCount < 3) { // first deal of cards
                                recv(client_socket, &card, sizeof(card), 0); // receive card from server
                                loadCard(card);
                                usr[0].score+=card.game_value;
                                ++newGameCount;
                            }

                            recv(client_socket, &usr[0], sizeof(usr[0]), 0); // receive card from server
                            recv(client_socket, &usr[1], sizeof(usr[1]), 0); // receive card from server
                            display_text(usr);
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

    test=connect(client_socket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
    if (test<0){
        perror("Can't connect to server\n");
        exit(1);
    }
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
        card[i].card_id=i;// Set card id
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



