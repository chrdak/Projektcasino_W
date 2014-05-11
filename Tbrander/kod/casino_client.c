
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
    int score, xPos, yPos,bet,tot_holding,stand,card_tot,winner,turn,quit,dealerTurn;
    // tot_holding = totalt kapital
};
typedef struct player_pos_value PLAYER;

struct server_threads{
    int nthread;
    int n_users;  // the number of users currently connected
    int tconsocket[5]; // the threads own connectionsocket
    int playerCon[5];
};
typedef struct server_threads THREAD;

struct nClient{
    DECK hand[15];
    PLAYER player;
    THREAD nUser;
};
typedef struct nClient NCLIENT;



/*FUNKTIONS PROTOTYPER*/
bool loadMedia(NCLIENT nClient[]); // Function for loading images unconverted
void SDL_initializer();
void game_running(NCLIENT nClient[],int,PLAYER user[],DECK card[]);
void quit(NCLIENT nClient[]);
void display_text(NCLIENT nClient[]);
int connect_to_server(NCLIENT nClient[], PLAYER user[]);//prototyp
void login_init();
void bet_client(NCLIENT nClient[],int nthread,PLAYER user[]);
void hit_stand(NCLIENT nClient[],int nthread,PLAYER user[]);
void draw(PLAYER user[],int nthread,int betSig);
void sendCient(NCLIENT nClient[],int nthread);
void recvClient(NCLIENT nClient[],int nthread);
//-------------------------------------------------

/*Global variables*/
SDL_Surface* loadSurface(char* path); //Loads individual image
SDL_Window* window = NULL;            //The window
SDL_Surface* screen = NULL;           // The window surface
SDL_Event event;                      //Event- for user interaction
int client_socket=0;
char table[50]="grafik/casino_betlight_off.bmp",hit_button[50]="grafik/hit_button.bmp",table_lights[50]="grafik/casino_betlight_on.bmp";
char stand_button[50]="grafik/stand_button.bmp",bet_button[50]="grafik/bet_button.bmp";


//************************************ MAIN *********************************************

int main( int argc, char* args[] ) {
    srand(time(NULL));
    int nthread;
    SDL_initializer();
    NCLIENT nClient[5];
    PLAYER user[5];
    DECK card[60];
    //login_init();

    if (!loadMedia(nClient)){ // Calling function for loading 24-bit images in to the memory
        printf("Cant load img.\n");
    }
    nthread=connect_to_server(nClient,user);

    game_running(nClient,nthread, user,card); // Game loop
    //quit(NCLIENT nClient[]);
 return 0;
}
//***************************************************************************************


/*

void display_text(NCLIENT nClient[]){

    char scoreToDisplay[10]={0};
    SDL_Surface *text;                    // Score to be printed
    TTF_Font *font;                       // True type font to be loaded (*.ttf)
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

*/


void draw(PLAYER user[],int nthread,int betSig){
    int i,j;

    // ------------------------- DRAW GAMEBOARD, BUTTONS -------------------------------------------
    SDL_Surface* table_img = NULL;        //Loaded converted table image
    SDL_Surface* table_lights_img=NULL;
    SDL_Surface* hit_img = NULL;          //Loaded converted hit button image
    SDL_Surface* stand_img = NULL;        //Loaded converted stand button image
    SDL_Surface* bet_img= NULL;

    table_img=loadSurface(table);
    table_lights_img=loadSurface(table_lights);
    hit_img=loadSurface(hit_button);
    stand_img=loadSurface(stand_button);
    bet_img=loadSurface(bet_button);

    SDL_Rect    hit_Rect;
    SDL_Rect    stand_Rect;
    SDL_Rect    bet_Rect;
    SDL_Rect    vit_Rect;
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
    // Positions for bet button
    bet_Rect.x=430;
    bet_Rect.y=530;
    bet_Rect.w=98;
    bet_Rect.h=49;

    if (betSig==1){
        SDL_BlitSurface(table_lights_img, NULL, screen, NULL);         // Gameboard
    }
    else{
        SDL_BlitSurface(table_img, NULL, screen, NULL);         // Gameboard
    }

    SDL_BlitScaled(hit_img, NULL, screen, &hit_Rect);       // hit button
    SDL_BlitScaled(stand_img, NULL, screen, &stand_Rect);   // stand button
    SDL_BlitSurface(bet_img, NULL, screen, &bet_Rect);      // bet button
    SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!
// -------------------------------------------------------------------------------

/*
    for(i=0;i<5;++i){ // test each space in the array
        if(nClient[1].nUser.playerCon[i] == i){


            for(j=0;j<nClient[1].player.card_tot;++j){
                nClient[i].hand[j].card_img=loadSurface(nClient[1].hand[j].path);
                SDL_Rect    hand_Rect;

                hand_Rect.x=nClient[1].player.xPos;
                hit_Rect.y=nClient[1].player.yPos;
                hit_Rect.w=98;
                hit_Rect.h=49;

                SDL_BlitScaled(nClient[1].hand[j].card_img, NULL, screen, &hand_Rect);   // stand button
            }

        }
    }
*/
    SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!
}

void game_running(NCLIENT nClient[],int nthread, PLAYER user[],DECK card[]){
    bool game=true;
    int betSig=1,i,bytes=0;
    while (game==true){
 printf("\n************************** NEW GAME ****************************\n");
// -------------------------- BET  REQUEST FROM SERVER -------------------------------------
        betSig=1;
        //draw(user,nthread,betSig);
        bet_client(nClient,nthread,user);
        send(nClient[nthread].nUser.tconsocket[nthread], &user[nthread].bet, sizeof(user[nthread].bet), 0); // send bet
        //draw(user,nthread,betSig);
        betSig=0;

// -------------------------------- BET OVER -----------------------------------------------

// ---------------------- WAITING TO RECIVE OTHER BETS -------------------------------------
        recv(nClient[nthread].nUser.tconsocket[nthread], &user[nthread], sizeof(user[nthread]), MSG_DONTWAIT); // Tar emot bet från alla
        draw(user,nthread,betSig);
        // DIN TUR ?
// ----------------------- WAITING FOR YOUR TURN -------------------------------------------

        while(user[nthread].turn!=nthread){
            recv(nClient[nthread].nUser.tconsocket[nthread], &user[nthread], sizeof(user[nthread]), 0); // TAR EMOT TURORDNING
            draw(user,nthread,betSig);
            printf("\nYour turn: %d\n",user[nthread].turn);
        }
// -------------------------- WAITING IS OVER ----------------------------------------------

// ---------------------------- YOUR TURN --------------------------------------------------

        while(user[nthread].stand==0){
            printf("\nYour score: %d\n",user[nthread].score);
            hit_stand(nClient,nthread,user);
            send(nClient[nthread].nUser.tconsocket[nthread], &user[nthread], sizeof(user[nthread]), 0); // send stand or hit
            if (user[nthread].stand ==1){ break;}
            recv(nClient[nthread].nUser.tconsocket[nthread], &card[nthread], sizeof(card[nthread]), 0); // Tar emot kort, score osv.
            recv(nClient[nthread].nUser.tconsocket[nthread], &user[nthread], sizeof(user[nthread]), 0); // Tar emot kort, score osv.
            if (user[nthread].score>=21){break;}
            draw(user,nthread,betSig);
        }
        user[nthread].turn = 0; // nollställ turn när turen är över

// ----------------------- YOUR TURN IS OVER -----------------------------------------------

// ---------------------- WAITING FOR DEALER -----------------------------------------------

        while(user[nthread].dealerTurn== 1) {
            recv(nClient[nthread].nUser.tconsocket[nthread], &user[nthread], sizeof(user[nthread]), 0);
            draw(user,nthread,betSig);
        }
        printf("\nScore: %d, Winner: %d\n",user[nthread].score,user[nthread].winner);
        user[nthread].bet=0;

        for(i=0;i<5;++i){
            bytes=recv(nClient[nthread].nUser.tconsocket[nthread], &user[nthread], sizeof(user[nthread]), MSG_DONTWAIT); // Tar emot bet från alla
            printf("\nGAME OVER, varv %d skräpbyte recv %d:\n",i,bytes);
        }

    } //game loop
}


void bet_client(NCLIENT nClient[],int nthread,PLAYER user[]){
        int x,y,bet=0,stand=0,betSig=1;
        while(betSig==1){
            draw(user,nthread,betSig);
            while( SDL_PollEvent( &event )) {// Check if user is closing the window --> then call quit

                 switch( event.type){
/*
                    case SDL_QUIT:
                        nClient[nthread].player.quit=1;
                        nClient[nthread].player.stand=1;
                        send(nClient[nthread].nUser.tconsocket[nthread], &user[nthread], sizeof(user[nthread]), 0); // send stand or hit
                        quit(nClient);
                        close(nClient[nthread].nUser.tconsocket[nthread]);
                        exit(0);
*/
                    case SDL_MOUSEBUTTONDOWN:   {// button clicks

                            x = event.button.x; // used to know where on x-axis is currently being clicked
                            y = event.button.y; // used to know where on y-axis is currently being clicked

                            if (event.button.button == (SDL_BUTTON_LEFT)){

                            // + 1
                                    if(x>70 && x< 70+55 && y>86 && y<86+55 && user[nthread].tot_holding>bet+1) { // can only be clicked while gameplay is true

                                        bet+=1;
                                        user[nthread].tot_holding-=1;
                                        user[nthread].bet=bet;
                                        printf("\nNew bet: %d\n",user[nthread].bet);
                                        printf("\nHoldings: %d\n",user[nthread].tot_holding);
                                    }

                            // + 10
                                    if(x>70 && x< 70+55 && y>150 && y<150+55 && user[nthread].tot_holding>bet+10) { // can only be clicked while gameplay is true

                                        bet+=10;
                                        user[nthread].tot_holding-=10;
                                        user[nthread].bet=bet;
                                        printf("\nNew bet: %d\n",user[nthread].bet);
                                        printf("\nHoldings: %d\n",user[nthread].tot_holding);
                                    }

                            // + 50
                                    if(x>70 && x< 70+55 && y>215 && y<215+55 && user[nthread].tot_holding>bet+50) { // can only be clicked while gameplay is true

                                        bet+=50;
                                        user[nthread].tot_holding-=50;
                                        user[nthread].bet=bet;
                                        printf("\nNew bet: %d\n",user[nthread].bet);
                                        printf("\nHoldings: %d\n",user[nthread].tot_holding);
                                    }

                            // + 100
                                    if(x>70 && x< 70+55 && y>277 && y<277+55 && user[nthread].tot_holding>bet+100) { // can only be clicked while gameplay is true

                                        bet+=100;
                                        user[nthread].tot_holding-=100;
                                        user[nthread].bet=bet;
                                        printf("\nNew bet: %d\n",user[nthread].bet);
                                        printf("\nHoldings: %d\n",user[nthread].tot_holding);
                                    }

                            // BET BUTTON
                                    if(x>430 && x< 430+98 && y>530 && y<530+49) {
                                        user[nthread].bet=bet;
                                        betSig=0;
                                        break;
                                    }

                            }// LEFT MOUSE

            // ---------------------------------------------------------------------------------------------------------

                            // RIGHT MOUSE

                            // - 1
                            if (event.button.button == (SDL_BUTTON_RIGHT)){
                                    if(x>70 && x< 70+55 && y>86 && y<86+55 && user[nthread].bet >= 1) { // can only be clicked while gameplay is true

                                            bet-=1;
                                            user[nthread].tot_holding+=1;
                                            user[nthread].bet=bet;
                                        printf("\nNew bet: %d\n",user[nthread].bet);
                                        printf("\nHoldings: %d\n",user[nthread].tot_holding);
                                    }

                            // - 10
                                    if(x>70 && x< 70+55 && y>150 && y<150+55 && user[nthread].bet >= 10) { // can only be clicked while gameplay is true

                                        bet-=10;
                                        user[nthread].tot_holding+=10;
                                        user[nthread].bet=bet;
                                        printf("\nNew bet: %d\n",user[nthread].bet);
                                        printf("\nHoldings: %d\n",user[nthread].tot_holding);
                                    }

                            // - 50
                                    if(x>70 && x< 70+55 && y>215 && y<215+55 && user[nthread].bet >= 50) { // can only be clicked while gameplay is true

                                        bet-=50;
                                        user[nthread].tot_holding+=50;
                                        user[nthread].bet=bet;
                                        printf("\nNew bet: %d\n",user[nthread].bet);
                                        printf("\nHoldings: %d\n",user[nthread].tot_holding);
                                    }

                            // - 100
                                    if(x>70 && x< 70+55 && y>277 && y<277+55 && user[nthread].bet >= 100) { // can only be clicked while gameplay is true

                                        bet-=100;
                                        user[nthread].tot_holding+=100;
                                        user[nthread].bet=bet;
                                        printf("\nNew bet: %d\n",user[nthread].bet);
                                        printf("\nHoldings: %d\n",user[nthread].tot_holding);
                                    }
                        }// RIGHT MOUSE
                        break;
                    } // case Mousedown
                }// SWITCH
            }// While inner
    } // While outer
}

void hit_stand(NCLIENT nClient[],int nthread,PLAYER user[]){

    int x,y,bet=0,buttonSig=0;
    while(buttonSig==0){
    while( SDL_PollEvent( &event )) {// Check if user is closing the window --> then call quit
         switch( event.type){
/*
            case SDL_QUIT:

                nClient[nthread].player.quit=1;
                nClient[nthread].player.stand=1;
                send(nClient[nthread].nUser.tconsocket[nthread], &user[nthread], sizeof(user[nthread]), 0); // send stand or hit
                quit(nClient);
                close(nClient[nthread].nUser.tconsocket[nthread]);
                exit(0);
*/

            case SDL_MOUSEBUTTONDOWN: {// button clicks

                x = event.button.x; // used to know where on x-axis is currently being clicked
                y = event.button.y; // used to know where on y-axis is currently being clicked
        //HIT BUTTON
                if (event.button.button == (SDL_BUTTON_LEFT)){
                    if(x>550 && x< 550+98 && y>530 && y<530+49) { // can only be clicked while gameplay is true

                        user[nthread].stand=0;
                        buttonSig=1;
                        break;
                    }

        // STAND BUTTON
                    if(x>670 && x< 670+98 && y>530 && y<530+49) { // stand button

                        user[nthread].stand=1;
                        buttonSig=1;
                        break;
                    }
                }// LEFT MOUSE

                break;
            }
        }// switch

      }// inner while
      draw(user,nthread,0);
    }// outer while
}


int connect_to_server(NCLIENT nClient[], PLAYER user[]){

    int nthread=0, test=0;
    char buffer[100]="Not connected!";
    struct sockaddr_in dest;
    client_socket = socket(AF_INET, SOCK_STREAM, 0); assert(client_socket!=-1);
    memset(&dest, 0, sizeof(dest));                /* zero the struct */
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr("127.0.0.1"); /* set destination IP number */
    dest.sin_port = htons(PORTNUM);                /* set destination port number */
    test=connect(client_socket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
    if (test<0){
        perror("Can't connect to server\n");
        exit(1);
    }
// ---------------------------------------- CONNECTED -------------------------------------------------
    recv(client_socket,&nthread,sizeof(int),0); // Recive user id
    nClient[nthread].nUser.nthread=nthread;
    nClient[nthread].nUser.tconsocket[nthread]=client_socket; // place the socket in the struct
    recv(nClient[nthread].nUser.tconsocket[nthread], &user[nthread].tot_holding, sizeof(user[nthread].tot_holding),0);
    return nthread;
}


void login_init(){
    bool running=true;
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
	SDL_Surface* loadedSurface =SDL_LoadBMP(path);// IMG_Load(path); // IMG_Load för SDL_image, *.PNG-format
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


bool loadMedia(NCLIENT nClient[]){

    SDL_Surface* table_img = NULL;        //Loaded converted table image
    SDL_Surface* hit_img = NULL;          //Loaded converted hit button image
    SDL_Surface* stand_img = NULL;        //Loaded converted stand button image
    SDL_Surface* bet_img= NULL;
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
    // Positions for bet button
    bet_Rect.x=430;
    bet_Rect.y=530;
    bet_Rect.w=98;
    bet_Rect.h=49;

    SDL_BlitSurface(table_img, NULL, screen, NULL);         // Gameboard
    SDL_BlitScaled(hit_img, NULL, screen, &hit_Rect);       // hit button
    SDL_BlitScaled(stand_img, NULL, screen, &stand_Rect);   // stand button
    SDL_BlitSurface(bet_img, NULL, screen, &bet_Rect);      // bet button
    // ----------------------------------------------------------------------------------------------


/*
    if( table_img == NULL ){
        printf( "Failed to load image!\n" );
        success = false;
    }
*/
    SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!

    return success;
}


/*
void quit(NCLIENT nClient[]){
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

*/

