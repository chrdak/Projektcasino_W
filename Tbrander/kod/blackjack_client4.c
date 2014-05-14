
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
#pragma pack(1)
struct card{
    char path[100];
    //int type; //(Back piece=0, Hearts=1, Clubbs=2, Diamonds=3, Spades=4)
    int game_value;
    //int real_value;
    SDL_Surface* card_img;
    SDL_Rect CardPos;
};
#pragma pack(0)
typedef struct card DECK;

struct player_pos_value{
    int score,x1,y1,x2,y2,x3,y3,bet,tot_holding;
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
bool loadMedia(DECK card[], int cardNumberOnScreen, PLAYER usr[]); // Function for loading images unconverted
void SDL_initializer();
void game_running(DECK [],PLAYER [], struct sockaddr_in, int myPlayerNumber);
void quit(DECK []);
void display_score(PLAYER usr[],int userNumber);
void connect_to_server(DECK card[], PLAYER usr[]);
void loadCard(DECK [], int cardNumberOnScreen);
void checkHandValue(PLAYER usr[], DECK card[], int user, int cardNumberOnScreen);
void login_init();
void display_message(PLAYER usr[],int userNumber, char message[]);
void playSound(char fileName[], int soundLoop);
void playSoundEffect(char fileName[]);
void recvStruct(DECK card[],int cardNumberOnScreen, int client_socket);
void recvUsrStruct(PLAYER usr[],int user, int client_socket);
void flushSocket(int socket);
int recvOtherUserInfo(DECK card[], PLAYER usr[],int user, int client_socket,int cardNumberOnScreen);
//-------------------------------------------------

/*Global variables*/
SDL_Surface* loadSurface(char* path); //Loads individual image
SDL_Surface* table_img = NULL;        //Loaded converted table image
SDL_Surface* hit_img = NULL;          //Loaded converted hit button image
SDL_Surface* bet_img= NULL;

SDL_Surface* stand_img = NULL;        //Loaded converted stand button image
TTF_Font *font;                       // True type font to be loaded (*.ttf)
SDL_Window* window = NULL;            //The window
SDL_Surface* screen = NULL;           // The window surface
SDL_Surface* text;                    // Score to be printed
Mix_Music *music = NULL;
Mix_Chunk *soundEffect = NULL;
SDL_Event event;                      //Event- for user interaction

_Bool running = true;                 // Game loop flag
int client_socket; // global
char table[50]="grafik/casino_betlight_on.bmp",hit_button[50]="grafik/hit_button.bmp",stand_button[50]="grafik/stand_button.bmp",bet_button[50]="grafik/bet_button.bmp";
int test; // assert

//************************************ MAIN *********************************************

int main( int argc, char* args[] ) {
    srand(time(NULL)); // Server
    //DECK card;     // Klient, server
    PLAYER usr[5];     // Klient, server
    DECK card[60];
    TTF_Init();
    //login_init();
    SDL_initializer(); // klient
    /*
    if (!loadMedia(card,0)){ // Calling function for loading 24-bit images in to the memory
        printf("Cant load img.\n");
    }*/
    if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096)) {
        printf("Unable to open audio!\n");
        exit(1);
    }
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


void display_score(PLAYER usr[],int userNumber){
     // Every users position stored in xPos, yPos
    char scoreToDisplay[20]={0};
    SDL_Color text_color = {255, 245, 0};
    //TTF_Init(); // Initialize SDL_ttf
    // RESET ------------------------------------------------------------------------
    SDL_Rect textLocation = { usr[userNumber].x2,usr[userNumber].y2, 0, 0 }; // Position of text, relativ to user
    //--------------------------------------------------------------------------
    sprintf(scoreToDisplay,"Sum: %d",usr[userNumber].score); // Translate int to string, storing it in scoreToDisplay
    font = TTF_OpenFont("fonts/DejaVuSans.ttf", 16); // Open true type font
    text = TTF_RenderText_Blended(font,scoreToDisplay,text_color); // Blended = smoother edges, Solid = sharper edges

    SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
    SDL_FreeSurface(text);

    // Free text
    //Close the font
    TTF_CloseFont(font);
}


void display_message(PLAYER usr[],int userNumber, char message[]){
     // Every users position stored in xPos, yPos
    SDL_Color text_color = {155, 16, 13};
    // RESET ------------------------------------------------------------------------
    SDL_Rect textLocation = { usr[userNumber].x3,usr[userNumber].y3, 0, 0 }; // Position of text, relativ to user
    //--------------------------------------------------------------------------
    // Translate int to string, storing it in scoreToDisplay
    font = TTF_OpenFont("fonts/DejaVuSans.ttf", 20); // Open true type font
    text = TTF_RenderText_Blended(font,message,text_color); // Blended = smoother edges, Solid = sharper edges

    SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
    SDL_FreeSurface(text);
    //Close the font
    TTF_CloseFont(font);
}

void game_running(DECK card [], PLAYER usr[], struct sockaddr_in dest, int myPlayerNumber){
    int x,y,i,j, count = 0;
    int hit = 0; // message to server for hit
    int stand = 1; // message to server for stand
    int bustOrLost = 2; // not using for the moment
    int newGame = 3; // message to server for new game
    int win = 4; // not using for the moment
    int newGameCount = 0; // keeps count on how many times to receive data when new game starts (deal cards function in server)
    bool gamePlay = false; // if true then game is running
    bool myTurn = false;
    bool dealerTurn=true;
    int receiveTurn = 0;
    int closeSocketmessage = 666;
    int userCount = 0;
    int dealCount = 0;
    int cardNumberOnScreen = 0;

    printf("Welcome player %d\n", myPlayerNumber+1);

    usr[0].score = 0;
    usr[0].handPos = 0;

    SDL_Rect newGameButton;
    Uint32 newGamecolor = SDL_MapRGB(screen->format,0x65,0x33,0x32);

    newGameButton.x = 0;
    newGameButton.y = 0;
    newGameButton.w = 80;
    newGameButton.h = 40;

    Uint32 start;
    const int FPS = 20;
    playSound("sound/david-luong_perto-de-voce-close-to-you.wav", -1);
    while(running){
        start = SDL_GetTicks();
        if(gamePlay == false) {
            count=0;
            //SDL_FillRect(screen,&newGameButton,newGamecolor); // only show new game button when there is no current hand in play
            cardNumberOnScreen = 0;
            /*
            if (!loadMedia(card,cardNumberOnScreen,usr)){ // Calling function for loading 24-bit images in to the memory
                printf("Cant load img.\n");
            }*/
            //send(client_socket, &newGame, sizeof(newGame), 0); // new game message to server
            //recv(client_socket, &userCount, sizeof(userCount), 0); // receives the amount of players connected
            dealCount = (userCount*2) + 1; // deal card to right amount of players including dealer
            printf("New Game\n");

            while(newGameCount < 5) { // first deal of cards


                recvStruct(card,cardNumberOnScreen,client_socket);

                if (!loadMedia(card,cardNumberOnScreen,usr)){ // Calling function for loading 24-bit images in to the memory
                    printf("Cant load img.\n");
                }
                //loadCard(card, cardNumberOnScreen);

                ++newGameCount;
                ++cardNumberOnScreen;

                }
                for(i=0;i<3;i++) {
                    recvUsrStruct(usr,i,client_socket);
                }
                if (!loadMedia(card,cardNumberOnScreen,usr)){ // Calling function for loading 24-bit images in to the memory
                    printf("Cant load img.\n");
                }

            printf("Dealer: %d\n", usr[0].score);
            printf("Player1: %d\n", usr[1].score);
            printf("Player2: %d\n", usr[2].score);
            newGameCount=0;
            gamePlay = true;
        }


//-------------------- KLIENT 2 VÄNTAR PÅ SIN TUR  ---------------------------

        while(gamePlay == true && myTurn == false) {
            if (!loadMedia(card,cardNumberOnScreen,usr)){ // Calling function for loading 24-bit images in to the memory
                printf("Cant load img.\n");
            }

            recv(client_socket, &receiveTurn, sizeof(receiveTurn), 0);
            printf("reciveturn: %d\n", receiveTurn);

            // loop som tar emot kort, poäng osv
            if(receiveTurn == myPlayerNumber) {
                 printf("Number inne: %d\n", receiveTurn);
                 myTurn = true;
            }
        }

// ------------------- KLIENT 2 HAR VÄNTAT KLART ------------------------------

        SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!

        while(myTurn == true){
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

                        //hit(card,usr,dest, gamePlay,loseColor,winLoseMessage, x, y);
                                //HIT BUTTON
                        if(x>550 && x< 550+98 && y>530 && y<530+49 && usr[myPlayerNumber+1].score < 21 && gamePlay == true && myTurn == true) { // can only be clicked while gameplay is true
                            send(client_socket, &hit, sizeof(hit), 0); // send hit message to server
                            ++cardNumberOnScreen;
                            recvStruct(card,cardNumberOnScreen,client_socket); // recv a card struct from server
    //                      for(i=0;i<3;i++) {
                            if(count == 0) {
                                i = myPlayerNumber+1;
                                count ++;
                            }
                            recvUsrStruct(usr,i,client_socket);
                            printf("card score: %d\n", usr[i].score);
    //                      }
                            printf("card value: %d\n", card[cardNumberOnScreen].game_value);
                            printf("card path: %s\n", card[cardNumberOnScreen].path);
                            playSoundEffect("sound/cardSlide6.wav");
                            if (!loadMedia(card,cardNumberOnScreen,usr)){ // Calling function for loading 24-bit images in to the memory
                                printf("Cant load img.\n");
                            }

                            flushSocket(client_socket);

                            if(usr[myPlayerNumber+1].score > 21) { // if player busts show new game button
                                myTurn = false;
                                display_message(usr,myPlayerNumber+1, "BUST");
                                send(client_socket, &stand, sizeof(stand), 0); // send stand message to server
                            }

                        }
                            // STAND BUTTON
                        if(x>670 && x< 670+98 && y>530 && y<530+49 && usr[myPlayerNumber+1].score <= 21 && gamePlay == true) { // stand button
                            printf("\nSTAND INNAN: %d\n",stand);
                            send(client_socket, &stand, sizeof(stand), 0); // send stand message to server
                            printf("\nSTAND EFTER: %d\n",stand);
                            myTurn = false;

                        }
                        break;
                 }
            } // EVENT WHILE
            sleep(1);
            SDL_UpdateWindowSurface(window); // DENNA KOD RAD GÖR ATT VI FÅR BILD!!
    } // OUTER WHILE

    printf("\nSpelare %d klar! Väntar på dealer...\n",myPlayerNumber+1);

    // Detta ska ej bort!!
//    int UserNr=1;
//    if (myPlayerNumber==0){UserNr=2; sleep(10);}
//    cardNumberOnScreen=recvOtherUserInfo(card,usr,UserNr,client_socket,cardNumberOnScreen);
//    printf("\ncardnr %d\n",cardNumberOnScreen);
//    if (!loadMedia(card,cardNumberOnScreen,usr)){ // Calling function for loading 24-bit images in to the memory
//            printf("Cant load img.\n");
//    }



    while(usr[0].score < 17) { // receive card while server/dealer is less than 17
        ++cardNumberOnScreen;
        recvStruct(card,cardNumberOnScreen,client_socket); // receive card to be displayed on dealer part of screen
        recvUsrStruct(usr,0,client_socket); // receive current dealer score
        if (!loadMedia(card,cardNumberOnScreen,usr)){ // Calling function for loading 24-bit images in to the memory
            printf("Cant load img.\n");
        }
        printf("\nDealer score: %d\n",usr[0].score);
    }
    flushSocket(client_socket);
    SDL_UpdateWindowSurface(window);


    if(usr[myPlayerNumber+1].score > usr[0].score && usr[myPlayerNumber+1].score < 22 || usr[0].score > 21) { // if win show yellow color rect

        display_message(usr,myPlayerNumber+1, "You Win");
    }
    if(usr[0].score > usr[myPlayerNumber+1].score && usr[0].score < 22 || usr[0].score == usr[myPlayerNumber+1].score || usr[myPlayerNumber+1].score > 21) { // if lose show red color rect

        display_message(usr,myPlayerNumber+1, "You Lose");
    }

    flushSocket(client_socket);
    SDL_UpdateWindowSurface(window);
    gamePlay=false;



    if(1000/FPS>SDL_GetTicks()-start) {
            SDL_Delay(1000/FPS-(SDL_GetTicks()-start));
    }

    printf("\nSpelare %d, Game Over!\n",myPlayerNumber+1);

    } // GAME LOOP

}

void connect_to_server(DECK card[], PLAYER usr[]){
    int myPlayerNumber;
    struct sockaddr_in dest;
    client_socket = socket(AF_INET, SOCK_STREAM, 0); assert(client_socket!=-1);
    memset(&dest, 0, sizeof(dest));                /* zero the struct */
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr("10.0.2.15"); /* set destination IP number */
    dest.sin_port = htons(PORTNUM);                /* set destination port number */
    test=connect(client_socket, (struct sockaddr *)&dest, sizeof(struct sockaddr));   assert(test==0);
    recv(client_socket, &myPlayerNumber, sizeof(myPlayerNumber), 0);
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

bool loadMedia(DECK card[], int cardNumberOnScreen, PLAYER usr []){
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
    for(i=0;i<3;i++) {
        display_score(usr,i); // display users and dealer score
    }
    for(i=0;i<cardNumberOnScreen+1;i++) {
            card[i].card_img = SDL_LoadBMP(card[i].path);
            SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);
            SDL_FreeSurface(card[i].card_img);
    }

    if( table_img == NULL ){
        printf( "Failed to load image!\n" );
        success = false;
    }
    SDL_FreeSurface(table_img); // Frigör bild för Black Jack bordet.
    SDL_FreeSurface(hit_img); // Frigör bild för hit-knapp bordet.
    SDL_FreeSurface(stand_img); // Frigör bild för stand-knapp bordet.
    SDL_UpdateWindowSurface(window);

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

void loadCard(DECK card [], int cardNumberOnScreen) {
     card[cardNumberOnScreen].card_img = loadSurface(card[cardNumberOnScreen].path);
     SDL_BlitScaled(card[cardNumberOnScreen].card_img, NULL, screen, &card[cardNumberOnScreen].CardPos);
     SDL_FreeSurface(card[cardNumberOnScreen].card_img);
}

void checkHandValue(PLAYER usr[], DECK card[], int user, int cardNumberOnScreen) { // calculates current hand value
    usr[user].hand[ usr[user].handPos ] = card[cardNumberOnScreen].game_value; // stores current card value in postion j of hand array
    ++usr[user].handPos;
    usr[user].score +=card[cardNumberOnScreen].game_value;
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

void playSound(char fileName[], int soundLoop) {

    if(music == NULL) {
        music = Mix_LoadMUS(fileName);
        Mix_PlayMusic(music, soundLoop);
    }
    music = NULL; // set pointer to null
    Mix_FreeMusic(music);

}

void playSoundEffect(char fileName[]) {
     soundEffect = Mix_LoadWAV(fileName);
     Mix_PlayChannel( -1,soundEffect,0);
     soundEffect = NULL;
     Mix_FreeChunk(soundEffect);
}

void recvStruct(DECK card[],int cardNumberOnScreen, int client_socket) {
    char x[100];
    char y[100];
    char gameValue[100];

    recv(client_socket, &gameValue, sizeof(gameValue), 0);
    recv(client_socket, &x, sizeof(x), 0);
    recv(client_socket, &y, sizeof(y), 0);
    recv(client_socket, &card[cardNumberOnScreen].path, sizeof(card[cardNumberOnScreen].path), 0);

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


    recv(client_socket, &x1, sizeof(x1), 0);
    recv(client_socket, &x2, sizeof(x2), 0);
    recv(client_socket, &x3, sizeof(x3), 0);
    recv(client_socket, &y1, sizeof(y1), 0);
    recv(client_socket, &y2, sizeof(y2), 0);
    recv(client_socket, &y3, sizeof(y3), 0);
    recv(client_socket, &score, sizeof(score), 0);

    sscanf(x1, "%d", &usr[user].x1);
    sscanf(x2, "%d", &usr[user].x2);
    sscanf(x3, "%d", &usr[user].x3);

    sscanf(y1, "%d", &usr[user].y1);
    sscanf(y2, "%d", &usr[user].y2);
    sscanf(y3, "%d", &usr[user].y3);

    sscanf(score, "%d", &usr[user].score);
}


    // Detta ska ej bort!!
//int recvOtherUserInfo(DECK card[], PLAYER usr[],int user, int client_socket,int cardNumberOnScreen){
//
//    int bytes=1;
//    char x[50];
//    char y[50];
//    char gameValue[50];
//    char x1[50];
//    char x2[50];
//    char x3[50];
//    char y1[50];
//    char y2[50];
//    char y3[50];
//    char score[50];
//    flushSocket(client_socket);
////    while (bytes>0){
//        bytes=recv(client_socket, &x1, sizeof(x1), 0);
//        bytes=recv(client_socket, &x2, sizeof(x2), 0);
//        bytes=recv(client_socket, &x3, sizeof(x3), 0);
//        bytes=recv(client_socket, &y1, sizeof(y1), 0);
//        bytes=recv(client_socket, &y2, sizeof(y2), 0);
//        bytes=recv(client_socket, &y3, sizeof(y3), 0);
//        bytes=recv(client_socket, &score, sizeof(score), 0);
//        bytes=recv(client_socket, &gameValue, sizeof(gameValue), 0);
//        bytes=recv(client_socket, &x, sizeof(x), 0);
//        bytes=recv(client_socket, &y, sizeof(y), 0);
//        bytes=recv(client_socket, &card[cardNumberOnScreen].path, sizeof(card[cardNumberOnScreen].path), 0);
//
//        puts(card[cardNumberOnScreen].path);
//
//        sscanf(x1, "%d", &usr[user].x1);
//        sscanf(x2, "%d", &usr[user].x2);
//        sscanf(x3, "%d", &usr[user].x3);
//        sscanf(y1, "%d", &usr[user].y1);
//        sscanf(y2, "%d", &usr[user].y2);
//        sscanf(y3, "%d", &usr[user].y3);
//
//        sscanf(score, "%d", &usr[user].score);
//        sscanf(gameValue, "%d", &card[cardNumberOnScreen].game_value);
//        sscanf(x, "%d", &card[cardNumberOnScreen].CardPos.x);
//        sscanf(y, "%d", &card[cardNumberOnScreen].CardPos.y);
//        card[cardNumberOnScreen].CardPos.w=75;
//        card[cardNumberOnScreen].CardPos.h=111;
//        cardNumberOnScreen++;
////    }
//
//    return cardNumberOnScreen;
//
//}
