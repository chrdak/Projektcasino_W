
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
#include <assert.h>
#include "libs/Client/recvStr.h"
#include "libs/Client/structs.h"
#include "libs/Client/sound.h"
#include "libs/Client/SDL_GameVariables.h"
#include "libs/Client/SDL_init.h"
#include "libs/Client/txt_display.h"
#include "libs/Client/Casino_graphic.h"
#include "libs/Client/bet.h"
#include "libs/Client/Casino_login.h"

#define PORTNUM 6578
#define SOCK_PATH "Casino_socket"
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "Projekt Casino"


/*FUNKTIONS PROTOTYPER*/

void game_running(DECK [],PLAYER [], struct sockaddr_in, int myPlayerNumber,int client_socket);
void quit(DECK []);
void connect_to_server(DECK card[], PLAYER usr[]);
void flushSocket(int socket);
//-------------------------------------------------

/*Global variables*/
bool running = true;                 // VIKTIG GLOBAL BOOL! ANVÄNDS SOM FLAGGA FÖR VILKET SPELBORD SOM SKA PRINTAS UT I loadMedia()
int test; // assert


int main( int argc, char* args[] ) {
    srand(time(NULL));
    PLAYER usr[5];
    DECK card[60];
    TTF_Init();
    if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096)) {
            printf("Unable to open audio!\n");
            exit(1);
    }
    SDL_Cursor* cursor;
    cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
    SDL_SetCursor(cursor);
    SDL_ShowCursor(1);

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

void game_running(DECK card [], PLAYER usr[], struct sockaddr_in dest, int myPlayerNumber, int client_socket){
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
    SDL_Event event;       //Event- for user interaction
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

    if (usr[myPlayerNumber+1].tot_holding==0){
        waiting_for_other_player(card,cardNumberOnScreen,usr,myPlayerNumber+1);
        display_message(usr,79, "You lost everything. Reseting funds..");
        SDL_UpdateWindowSurface(window);
        sleep(2);
        usr[myPlayerNumber+1].tot_holding=1000;} //Reset
    } // GAME LOOP
}

void connect_to_server(DECK card[], PLAYER usr[]){
    int myPlayerNumber,client_socket=0;
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
    game_running(card,usr, dest, myPlayerNumber,client_socket);
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


