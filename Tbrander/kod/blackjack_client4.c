
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
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
    int game_value;
    SDL_Surface* card_img;
    SDL_Rect CardPos;
};
typedef struct card DECK;

struct player_pos_value{
    int score,x1,y1,x2,y2,x3,y3,bet,tot_holding;
};
typedef struct player_pos_value PLAYER;


/*FUNKTIONS PROTOTYPER*/
bool loadMedia(DECK card[], int cardNumberOnScreen, PLAYER usr[],int); // Function for loading images unconverted
void SDL_initializer();
void game_running(DECK [],PLAYER [], struct sockaddr_in, int myPlayerNumber);
void quit(DECK []);
void display_score(PLAYER usr[],int userNumber);
void connect_to_server(DECK card[], PLAYER usr[]);
void login_init();
void display_message(PLAYER usr[],int userNumber, char message[]);
void playSound(char fileName[], int soundLoop);
void playSoundEffect(char fileName[],int);
void recvStruct(DECK card[],int cardNumberOnScreen, int client_socket);
void recvUsrStruct(PLAYER usr[],int user, int client_socket);
void flushSocket(int socket);
void bet_client(int myPlayerNr,PLAYER user[],int cardNumberOnScreen,DECK card[]);
void display_bet_holding(PLAYER user[],DECK card[],int,int cardNumberOnScreen);
void waiting_for_other_player(DECK card[], int cardNumberOnScreen, PLAYER usr [],int myPlayerNumber);
//-------------------------------------------------

/*Global variables*/
SDL_Surface* loadSurface(char* path); //Loads individual image
SDL_Window* window = NULL;            //The window
SDL_Surface* screen = NULL;           // The window surface
bool running = true;                 // VIKTIG GLOBAL BOOL! ANVÄNDS SOM FLAGGA FÖR VILKET SPELBORD SOM SKA PRINTAS UT I loadMedia()
int client_socket; // global
int test; // assert


int main( int argc, char* args[] ) {
    srand(time(NULL)); // Server
    PLAYER usr[5];     // Klient, server
    DECK card[60];
    TTF_Init();
    if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096)) {
            printf("Unable to open audio!\n");
            exit(1);
    }
    login_init();
    SDL_initializer();
    connect_to_server(card,usr);

 return 0;
}

void flushSocket(int socket) {
    char flush[1500] = {0};
    int byte = 1;
    while(byte>0) {
       byte= recv(socket, &flush, sizeof(flush), MSG_DONTWAIT);
       printf("\n TRASH: %d\n", byte);
    }
}

void login_init(){
    SDL_Event event;                      //Event- for user interaction
    SDL_Window* login_window=NULL;
    SDL_Surface* login_img = NULL;
    int SDL_test,x,y;
    char login[50]="grafik/login.bmp";
    SDL_test=SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); assert(SDL_test==0);

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
            x = event.button.x; // used to know where on x-axis is currently being clicked
            y = event.button.y; // used to know where on y-axis is currently being clicked

             if( event.type == SDL_QUIT ){
                 SDL_DestroyWindow(login_window);  // Dödar fönstret
                 running = false; // Gameloop flag false
             }


             if (event.type ==SDL_MOUSEBUTTONDOWN){
                if(x>520 && x< 520+100 && y>420 && y<420+49){
                    SDL_DestroyWindow(login_window);  // Dödar fönstret
                    running = false; // Gameloop flag false
                }
             }
             if (event.type ==SDL_MOUSEMOTION){
                    SDL_UpdateWindowSurface(login_window);
             }
          }
     }
     running = true;
}

void display_score(PLAYER usr[],int userNumber){

    SDL_Surface* text;                    // Score to be printed
    TTF_Font *font;                       // True type font to be loaded (*.ttf)
    char scoreToDisplay[20]={0};
    SDL_Color text_color = {255, 245, 0};
    SDL_Rect textLocation = { usr[userNumber].x2,usr[userNumber].y2, 0, 0 }; // Position of text, relative to user
    sprintf(scoreToDisplay,"Sum: %d",usr[userNumber].score); // Translate int to string, storing it in scoreToDisplay
    font = TTF_OpenFont("fonts/DejaVuSans.ttf", 16); // Open true type font
    text = TTF_RenderText_Blended(font,scoreToDisplay,text_color); // Blended = smoother edges, Solid = sharper edges
    SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
    SDL_FreeSurface(text);
    TTF_CloseFont(font);
}

void display_message(PLAYER usr[],int userNumber, char message[]){

    // MESSAGE FLAGS
    /*
    79 = Message to player who's turn it is
    89 = Waiting for other player..
    99 = New game in 5..4..3..2..1.. starting..
    */

    SDL_Surface* text;                    // Score to be printed
    TTF_Font *font;                       // True type font to be loaded (*.ttf)

    if (userNumber==79){
            SDL_Color text_color = {247, 215, 16};
            SDL_Rect textLocation = { 60,540, 0, 0 };
            font = TTF_OpenFont("fonts/DejaVuSans.ttf", 36); // Open true type font
            text = TTF_RenderText_Blended(font,message,text_color); // Blended = smoother edges, Solid = sharper edges
            SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
            SDL_FreeSurface(text);
            TTF_CloseFont(font);

        } // Position of text, relative to user

    if (userNumber==89){
            SDL_Color text_color = {247, 215, 16};
            SDL_Rect textLocation = { 410,70, 0, 0 };
            font = TTF_OpenFont("fonts/DejaVuSans.ttf", 36); // Open true type font
            text = TTF_RenderText_Blended(font,message,text_color); // Blended = smoother edges, Solid = sharper edges
            SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
            SDL_FreeSurface(text);
            TTF_CloseFont(font);

        } // Position of text, relative to user
    if (userNumber==99){
        SDL_Color text_color = {247, 215, 16}; // YELLOW
        SDL_Rect textLocation = { 480,70, 0, 0 };
        font = TTF_OpenFont("fonts/DejaVuSans.ttf", 36); // Open true type font
        text = TTF_RenderText_Blended(font,message,text_color); // Blended = smoother edges, Solid = sharper edges
        SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
        SDL_FreeSurface(text);
        TTF_CloseFont(font);

    } // Position of text, relative to user
    if (userNumber==0 || userNumber==1){
        SDL_Color text_color = {155, 16, 13}; // RED
        SDL_Rect textLocation = { usr[userNumber].x3,usr[userNumber].y3, 0, 0 }; // Position of text, relativ to user
        font = TTF_OpenFont("fonts/DejaVuSans.ttf", 20); // Open true type font
        text = TTF_RenderText_Blended(font,message,text_color); // Blended = smoother edges, Solid = sharper edges
        SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
        SDL_FreeSurface(text);
        TTF_CloseFont(font);
    }
}

void game_running(DECK card [], PLAYER usr[], struct sockaddr_in dest, int myPlayerNumber){
    int x,y,i,j, count = 0; // x,y kordinater, i,j räknare
    int hit = 0; // message to server for hit
    int stand = 1; // message to server for stand
    int newGameCount = 0; // keeps count on how many times to receive data when new game starts (deal cards function in server)
    bool gamePlay = false; // if true then game is running
    bool myTurn = false;
    bool dealerTurn=true;
    int receive_flag = 0, send_flag_klient2_done=6;
    int closeSocketmessage = 666;
    int cardNumberOnScreen = 0;
    int test=0;
    SDL_Event event;                      //Event- for user interaction
    printf("Welcome player %d\n", myPlayerNumber+1);
    usr[0].score = 0;
    usr[1].bet = 0;
    usr[2].bet = 0;

    Uint32 start;
    const int FPS = 20;
    playSound("sound/david-luong_perto-de-voce-close-to-you.wav", -1);

    playSoundEffect("sound/casino_backround_sound.wav",2);
    while(running){
        start = SDL_GetTicks();
        if(gamePlay == false) {
            cardNumberOnScreen=0;
            count=0;
            printf("New Game\n");
            bet_client(myPlayerNumber+1,usr,cardNumberOnScreen,card);
            send(client_socket, &usr[myPlayerNumber+1].bet, sizeof(usr[myPlayerNumber+1].bet), 0);

            display_message(usr,89, "Waiting for other player..");
            SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!
            while(newGameCount < 5) { // first deal of cards
                recvStruct(card,cardNumberOnScreen,client_socket);
                waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
                ++newGameCount;
                ++cardNumberOnScreen;
            }
            // FIRST DEAL
            for(i=0;i<3;i++) {
                recvUsrStruct(usr,i,client_socket);
            }
            waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
            if (myPlayerNumber+1==1){display_message(usr,79, "You are player 1!");}
            if (myPlayerNumber+1==2){display_message(usr,79, "You are player 2!");}
            SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!
            sleep(2); // För att fördröja medelandet om vilken spelare man är så man hinner läsa
            newGameCount=0;
            gamePlay = true;
        }


//-------------------- KLIENT 2 VÄNTAR PÅ SIN TUR  ---------------------------

        while(gamePlay == true && myTurn == false) {
            waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
            display_message(usr,79, "Player 1:s turn.");
            SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!
            test=recv(client_socket, &receive_flag, sizeof(receive_flag), 0);
            if(test == ENOTCONN){
                perror("\nServer closed connection\n");

            }
            // Tar emot kort och poäng
            if (receive_flag==5){
                flushSocket(client_socket);
                ++cardNumberOnScreen;
                recvStruct(card,cardNumberOnScreen,client_socket);
                recvUsrStruct(usr,1,client_socket);
                display_message(usr,79, "Player 1:s turn.");
                SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!
                continue;
            }
            if(receive_flag == myPlayerNumber) {
                 myTurn = true;
            }
            loadMedia(card,cardNumberOnScreen,usr,myPlayerNumber+1);
            display_message(usr,79, "Your turn.");
            SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!
        }

// ------------------- KLIENT 2 HAR VÄNTAT KLART ------------------------------


        while(SDL_PollEvent(&event)) { /* POLL EVERYTHING FROM EVENTSTACK */ }

        SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!

        while(myTurn == true){ // GAME LOOP
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
                        if(x>490 && x< 490+98 && y>530 && y<530+49 && usr[myPlayerNumber+1].score < 21 && gamePlay == true && myTurn == true) { // can only be clicked while gameplay is true
                            send(client_socket, &hit, sizeof(hit), 0); // send hit message to server
                            ++cardNumberOnScreen;
                            recvStruct(card,cardNumberOnScreen,client_socket); // recv a card struct from server
                            if(count == 0) {
                                i = myPlayerNumber+1;
                                count ++;
                            }
                            recvUsrStruct(usr,i,client_socket);
                            playSoundEffect("sound/cardSlide6.wav",-1);
                            if (!loadMedia(card,cardNumberOnScreen,usr,myPlayerNumber+1)){ // Calling function for loading 24-bit images in to the memory
                                printf("Cant load img.\n");
                            }
                            flushSocket(client_socket);
                            if(usr[myPlayerNumber+1].score > 21) { // if player bust
                                myTurn = false;
                                display_message(usr,79, "Busted!");
                                SDL_UpdateWindowSurface(window);
                                sleep(1); //Fördröj medelandet "Busted!"
                                send(client_socket, &stand, sizeof(stand), 0); // send stand message to server
                                usr[myPlayerNumber+1].bet=0;
                            }
                            if(usr[myPlayerNumber+1].score == 21) { // if player bust
                                myTurn = false;
                                display_message(usr,79, "Blackjack!");
                                SDL_UpdateWindowSurface(window);
                                sleep(1); //Fördröj medelandet "Blackjack!"
                                send(client_socket, &stand, sizeof(stand), 0); // send stand message to server
                            }

                        }
                        // STAND BUTTON
                        if(x>610 && x< 610+98 && y>530 && y<530+49 && usr[myPlayerNumber+1].score <= 21 && gamePlay == true) { // stand button
                            send(client_socket, &stand, sizeof(stand), 0); // send stand message to server
                            myTurn = false;
                        }
                        break;
                 }
            } // EVENT WHILE
            SDL_UpdateWindowSurface(window);
    } // OUTER WHILE

    if (myPlayerNumber+1==1){
        while(1) {
            waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
            display_message(usr,79, "Player 2:s turn.");
            SDL_UpdateWindowSurface(window);
            test=recv(client_socket, &receive_flag, sizeof(receive_flag), 0);
            if(test == ENOTCONN){
                perror("\nServer closed connection\n");
            }
            // Tar emot kort och poäng
            if (receive_flag==5){
                ++cardNumberOnScreen;
                recvStruct(card,cardNumberOnScreen,client_socket);
                recvUsrStruct(usr,2,client_socket);
            }
            if(receive_flag == 6) {
                break;
            }
        } // while client 2 i playing, recv cards if he hits, else break
    }// if user 0

    waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
    display_message(usr,79, "Dealers turn.");
    SDL_UpdateWindowSurface(window);
    while(usr[0].score < 17) { // receive card while server/dealer is less than 17
        ++cardNumberOnScreen;
        recvStruct(card,cardNumberOnScreen,client_socket); // receive card to be displayed on dealer part of screen
        recvUsrStruct(usr,0,client_socket); // receive current dealer score
        waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
        display_message(usr,79, "Dealers turn.");
        SDL_UpdateWindowSurface(window);
    }
    waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
    flushSocket(client_socket);

    if(usr[myPlayerNumber+1].score > usr[0].score && usr[myPlayerNumber+1].score < 22 || usr[0].score > 21 && usr[myPlayerNumber+1].score < 22) { // if win show yellow color rect
        display_message(usr,79, "You Win!");
        usr[myPlayerNumber+1].tot_holding+=usr[myPlayerNumber+1].bet*2;
        playSoundEffect("sound/winner.wav",-1);
        usr[myPlayerNumber+1].bet=0;
    }
    if(usr[0].score > usr[myPlayerNumber+1].score && usr[0].score < 22 || usr[0].score == usr[myPlayerNumber+1].score || usr[myPlayerNumber+1].score > 21) { // if lose show red color rect
        display_message(usr,79, "You Lose..");
        playSoundEffect("sound/loser.wav",-1);
        usr[myPlayerNumber+1].bet=0;
    }
    SDL_UpdateWindowSurface(window);

    flushSocket(client_socket);
    gamePlay=false;

// ------------------------ NEW GAME MSG ---------------------------------
        sleep(2);
        display_message(usr,99, "New game in 5..");
        SDL_UpdateWindowSurface(window);
        sleep(1);
        waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
        display_message(usr,99, "New game in 4..");
        SDL_UpdateWindowSurface(window);
        sleep(1);
        waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
        display_message(usr,99, "New game in 3..");
        SDL_UpdateWindowSurface(window);
        sleep(1);
        waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
        display_message(usr,99, "New game in 2..");
        SDL_UpdateWindowSurface(window);
        sleep(1);
        waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
        display_message(usr,99, "New game in 1..");
        SDL_UpdateWindowSurface(window);
        sleep(1);
        for(i=0;i<3;++i){ // nollställ allas poäng
            usr[i].score=0;
        }
        cardNumberOnScreen = -1;
        waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
        display_message(usr,99, "Starting..");
        SDL_UpdateWindowSurface(window);
        sleep(1);
        waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
// ------------------------------------------------------------------------

    if(1000/FPS>SDL_GetTicks()-start) {
            SDL_Delay(1000/FPS-(SDL_GetTicks()-start));
    }
    while(SDL_PollEvent(&event)) { /* POLL EVERYTHING FROM EVENTSTACK */ }
    } // GAME LOOP
}

void connect_to_server(DECK card[], PLAYER usr[]){
    int myPlayerNumber;
    struct sockaddr_in dest;
    client_socket = socket(AF_INET, SOCK_STREAM, 0); assert(client_socket!=-1);
    memset(&dest, 0, sizeof(dest));                /* zero the struct */
    dest.sin_family = AF_INET;
    //dest.sin_addr.s_addr = inet_addr("130.237.84.123"); /* set destination IP number */
    dest.sin_addr.s_addr = inet_addr("127.0.0.1"); /* set destination IP number */
    dest.sin_port = htons(PORTNUM);                /* set destination port number */
    test=connect(client_socket, (struct sockaddr *)&dest, sizeof(struct sockaddr));   assert(test==0);
    recv(client_socket, &myPlayerNumber, sizeof(myPlayerNumber), 0);
    recv(client_socket, &usr[myPlayerNumber+1].tot_holding, sizeof(usr[myPlayerNumber+1].tot_holding), 0);
    printf("cash: %d\n",usr[myPlayerNumber+1].tot_holding);
    game_running(card,usr, dest, myPlayerNumber);
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

bool loadMedia(DECK card[], int cardNumberOnScreen, PLAYER usr [],int myPlayerNumber){
    //Loading success flag
    bool success = true;
    int i=0;
    // ------------------------------ LOAD BUTTONS AND GAMEBOARD ---------------------------------
    if (running==true){
        char game_table[50]="grafik/casino_betlight_off.bmp", hit_button[50]="grafik/hit_button.bmp",stand_button[50]="grafik/stand_button.bmp";
        SDL_Surface* table_lightsOff_img = NULL;        //Loaded converted table image
        SDL_Surface* hit_img = NULL;          //Loaded converted hit button image
        SDL_Surface* stand_img = NULL;        //Loaded converted stand button image
        table_lightsOff_img=loadSurface(game_table);
        hit_img=loadSurface(hit_button);
        stand_img=loadSurface(stand_button);
        SDL_Rect    hit_Rect;
        SDL_Rect    stand_Rect;
        hit_Rect.x=490;
        hit_Rect.y=530;
        hit_Rect.w=98;
        hit_Rect.h=49;
        stand_Rect.x=610;
        stand_Rect.y=530;
        stand_Rect.w=98;
        stand_Rect.h=49;
        SDL_BlitSurface(table_lightsOff_img, NULL, screen, NULL);
        SDL_BlitScaled(hit_img, NULL, screen, &hit_Rect);       // hit button
        SDL_BlitScaled(stand_img, NULL, screen, &stand_Rect);   // stand button
        SDL_FreeSurface(hit_img); // Frigör bild för hit-knapp bordet.
        SDL_FreeSurface(stand_img); // Frigör bild för stand-knapp bordet.
        SDL_FreeSurface(table_lightsOff_img);
        }
    if (running==false){
        char bet_table[50]="grafik/casino_betlight_on.bmp",bet_button[50]="grafik/bet_button.bmp";
        SDL_Surface* table_lightsOn_img = NULL;        //Loaded converted table image
        SDL_Surface* bet_img= NULL;
        table_lightsOn_img=loadSurface(bet_table);
        bet_img=loadSurface(bet_button);
        SDL_Rect    bet_Rect;
        bet_Rect.x=550;
        bet_Rect.y=530;
        bet_Rect.w=98;
        bet_Rect.h=49;
        SDL_BlitSurface(table_lightsOn_img, NULL, screen, NULL);
        SDL_BlitSurface(bet_img, NULL, screen, &bet_Rect);
        SDL_FreeSurface(table_lightsOn_img);
        SDL_FreeSurface(bet_img);
    }
    // ----------------------------------------------------------------------------------------------
    if (running==true){
        for(i=0;i<3;i++) {
            display_score(usr,i); // display user and dealer score
        }
    }
    for(i=0;i<cardNumberOnScreen+1;i++) {
        card[i].card_img = SDL_LoadBMP(card[i].path);
        SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);
        SDL_FreeSurface(card[i].card_img);
    }

    SDL_UpdateWindowSurface(window);
    display_bet_holding(usr,card,myPlayerNumber,cardNumberOnScreen);
    return success;
}

void quit(DECK card[]){
    int i;
    //Quit SDL_ttf
    TTF_Quit();
    // Frigör bilder för spelkorten.

    //SDL_FreeSurface(table_img); // Frigör bild för Black Jack bordet.
    //SDL_FreeSurface(hit_img); // Frigör bild för hit-knapp bordet.
    //SDL_FreeSurface(stand_img); // Frigör bild för stand-knapp bordet.
    SDL_DestroyWindow(window);  // Dödar fönstret
    SDL_Quit();
}

void playSound(char fileName[], int soundLoop) {
    Mix_Music *music = NULL;
    Mix_Chunk *soundEffect = NULL;
    if(music == NULL) {
        music = Mix_LoadMUS(fileName);
        Mix_PlayMusic(music, soundLoop);
    }
    music = NULL; // set pointer to null
    Mix_FreeMusic(music);

}

void playSoundEffect(char fileName[],int channel) {
     Mix_Music *music = NULL;
     Mix_Chunk *soundEffect = NULL;
     Mix_Volume(2,50);
     soundEffect = Mix_LoadWAV(fileName);
     Mix_PlayChannel( channel,soundEffect,0);
     soundEffect = NULL;
     Mix_FreeChunk(soundEffect);
}

void recvStruct(DECK card[],int cardNumberOnScreen, int client_socket) {
    char x[100];
    char y[100];
    char gameValue[100];
    int test = 0;

    test = recv(client_socket, &gameValue, sizeof(gameValue), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &x, sizeof(x), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &y, sizeof(y), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &card[cardNumberOnScreen].path, sizeof(card[cardNumberOnScreen].path), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }

    sscanf(gameValue, "%d", &card[cardNumberOnScreen].game_value);
    sscanf(x, "%d", &card[cardNumberOnScreen].CardPos.x);
    sscanf(y, "%d", &card[cardNumberOnScreen].CardPos.y);
    card[cardNumberOnScreen].CardPos.w=75;
    card[cardNumberOnScreen].CardPos.h=111;

}

void recvUsrStruct(PLAYER usr[],int user, int client_socket) {

    char x1[100];
    char x2[100];
    char x3[100];
    char y1[100];
    char y2[100];
    char y3[100];
    char score[100];
    int test=0;

    test = recv(client_socket, &x1, sizeof(x1), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &x2, sizeof(x2), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &x3, sizeof(x3), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &y1, sizeof(y1), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &y2, sizeof(y2), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &y3, sizeof(y3), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &score, sizeof(score), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }

    sscanf(x1, "%d", &usr[user].x1);
    sscanf(x2, "%d", &usr[user].x2);
    sscanf(x3, "%d", &usr[user].x3);

    sscanf(y1, "%d", &usr[user].y1);
    sscanf(y2, "%d", &usr[user].y2);
    sscanf(y3, "%d", &usr[user].y3);
    sscanf(score, "%d", &usr[user].score);
}

void bet_client(int myPlayerNr,PLAYER user[],int cardNumberOnScreen,DECK card[]){
        int x,y,bet=0,stand=0,betSig=1;
        SDL_Event event;
        running=false;
        loadMedia(card,-1,user,myPlayerNr);
        while(betSig==1){
            while( SDL_PollEvent( &event )) {// Check if user is closing the window --> then call quit
                 switch( event.type){

                    case SDL_QUIT:
                        exit(0);

                    case SDL_MOUSEBUTTONDOWN:   {// button clicks

                            x = event.button.x; // used to know where on x-axis is currently being clicked
                            y = event.button.y; // used to know where on y-axis is currently being clicked

                            if (event.button.button == (SDL_BUTTON_LEFT)){

                            // + 1
                                    if(x>70 && x< 70+55 && y>86 && y<86+55 && user[myPlayerNr].tot_holding>=1) { // can only be clicked while gameplay is true
                                        bet+=1;
                                        user[myPlayerNr].tot_holding-=1;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // + 10
                                    if(x>70 && x< 70+55 && y>150 && y<150+55 && user[myPlayerNr].tot_holding>=10) { // can only be clicked while gameplay is true

                                        bet+=10;
                                        user[myPlayerNr].tot_holding-=10;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // + 50
                                    if(x>70 && x< 70+55 && y>215 && y<215+55 && user[myPlayerNr].tot_holding>=50) { // can only be clicked while gameplay is true

                                        bet+=50;
                                        user[myPlayerNr].tot_holding-=50;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // + 100
                                    if(x>70 && x< 70+55 && y>277 && y<277+55 && user[myPlayerNr].tot_holding>=100) { // can only be clicked while gameplay is true

                                        bet+=100;
                                        user[myPlayerNr].tot_holding-=100;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // BET BUTTON
                                    if(x>550 && x< 550+98 && y>530 && y<530+49) {
                                        if (user[myPlayerNr].bet <=0){
                                            display_message(user,79, "Place your bet please!");
                                            SDL_UpdateWindowSurface(window);
                                            sleep(1); // delay the message
                                        }
                                        else{
                                        user[myPlayerNr].bet=bet;
                                        betSig=0;
                                        playSoundEffect("sound/chipsStack4.wav",-1);
                                        break;
                                        }
                                    }
                                playSoundEffect("sound/chipsStack4.wav",-1);

                            }// LEFT MOUSE

            // ---------------------------------------------------------------------------------------------------------

                            // RIGHT MOUSE

                            // - 1
                            if (event.button.button == (SDL_BUTTON_RIGHT)){
                                    if(x>70 && x< 70+55 && y>86 && y<86+55 && user[myPlayerNr].bet >= 1) { // can only be clicked while gameplay is true

                                            bet-=1;
                                            user[myPlayerNr].tot_holding+=1;
                                            user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // - 10
                                    if(x>70 && x< 70+55 && y>150 && y<150+55 && user[myPlayerNr].bet >= 10) { // can only be clicked while gameplay is true

                                        bet-=10;
                                        user[myPlayerNr].tot_holding+=10;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // - 50
                                    if(x>70 && x< 70+55 && y>215 && y<215+55 && user[myPlayerNr].bet >= 50) { // can only be clicked while gameplay is true

                                        bet-=50;
                                        user[myPlayerNr].tot_holding+=50;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // - 100
                                    if(x>70 && x< 70+55 && y>277 && y<277+55 && user[myPlayerNr].bet >= 100) { // can only be clicked while gameplay is true

                                        bet-=100;
                                        user[myPlayerNr].tot_holding+=100;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);

                                    }
                                    playSoundEffect("sound/chipsStack4.wav",-1);
                        }// RIGHT MOUSE
                        loadMedia(card,-1,user,myPlayerNr);
                        display_bet_holding(user,card,myPlayerNr,cardNumberOnScreen);
                        break;
                    } // case Mousedown
                case SDL_MOUSEMOTION: {
                    SDL_UpdateWindowSurface(window);
                }
                }// SWITCH
            }// While inner
    } // While outer
    running=true;
}

void display_bet_holding(PLAYER user[],DECK card[],int myPlayerNr,int cardNumberOnScreen){

        SDL_Surface* text;                    // Score to be printed
        TTF_Font *font;                       // True type font to be loaded (*.ttf)
        char player_bet[20]="";
        char player_holdings[20]="";
        if(user[myPlayerNr].tot_holding>0){
            sprintf(player_bet, "Bet: %d", user[myPlayerNr].bet);
            sprintf(player_holdings, "Holding: %d", user[myPlayerNr].tot_holding);
        }
        if(user[myPlayerNr].tot_holding==0){
            sprintf(player_bet, "Bet: %d", user[myPlayerNr].bet);
            sprintf(player_holdings, "Insufficient funds");
        }
        SDL_Color text_color = {251, 218, 15}; // casinoyellow
        SDL_Rect textLocation = { 60,360, 0, 0 };
        font = TTF_OpenFont("fonts/DejaVuSans.ttf", 20); // Open true type font
        text = TTF_RenderText_Blended(font,player_bet,text_color); // Blended = smoother edges, Solid = sharper edges
        SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
        SDL_FreeSurface(text);
        TTF_CloseFont(font);

        SDL_Rect textLocation_holdings = { 60,390, 0, 0 }; // Position of text, relativ to user
        font = TTF_OpenFont("fonts/DejaVuSans.ttf", 20); // Open true type font
        text = TTF_RenderText_Blended(font,player_holdings,text_color); // Blended = smoother edges, Solid = sharper edges
        SDL_BlitSurface(text, NULL, screen, &textLocation_holdings); // Draw to screen
        SDL_FreeSurface(text);
        TTF_CloseFont(font);
        SDL_UpdateWindowSurface(window);
}

void waiting_for_other_player(DECK card[], int cardNumberOnScreen, PLAYER usr [],int myPlayerNumber){
        int i;
        char game_table[50]="grafik/casino_betlight_off.bmp";
        SDL_Surface* table_lightsOff_img = NULL;  //Loaded converted table image
        table_lightsOff_img=loadSurface(game_table);
        SDL_BlitSurface(table_lightsOff_img, NULL, screen, NULL);
        SDL_FreeSurface(table_lightsOff_img);
        for(i=0;i<3;i++) {
            display_score(usr,i); // display user and dealer score
        }
        for(i=0;i<cardNumberOnScreen+1;i++) {
            card[i].card_img = SDL_LoadBMP(card[i].path);
            SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);
            SDL_FreeSurface(card[i].card_img);
        }
        display_bet_holding(usr,card,myPlayerNumber,cardNumberOnScreen);
}



