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
//#include "lib/server.h"
#define PORTNUM 6578
#define SOCK_PATH "Casino_socket"

// --------------------------------------------------------------------------------------------------

struct card{
    char path[100];
    int game_value;
    SDL_Rect CardPos;
};
typedef struct card DECK;

struct player_pos_value{
    int score, x1, y1,x2,y2,x3,y3,bet,tot_holding;
    int hand[11]; // Array som representerar en spelares hand, varje plats innehåller info om tilldelade kort, färg, värden..
    int handPos;
};                 // Plats [0] är första tilldelade kortet osv.
typedef struct player_pos_value PLAYER;

struct server_threads{
    int tconsocket[5]; // the threads own connectionsocket
};
typedef struct server_threads THREAD;


/*FUNKTIONS PROTOTYPER*/

void sendUsrStruct(PLAYER usr[],int user, int socketNumber);
void sendDeckStruct(DECK card[], int *deckPosition, int socketNumber);
void card_init(DECK card[],PLAYER usr[]); // Initialize the card deck
void shuffleDeck(DECK card[]);
void deal_cards(PLAYER usr[],DECK card[], THREAD tdata[], int* deckPosition);
void server(DECK card[], PLAYER usr[], int* deckPosition);
void* serve_client (void* parameters);    // thread function
void checkHandValue(PLAYER usr[], DECK card[], int user, int* deckPosition);
void cardRect(DECK card [],PLAYER usr [], int* deckPosition, int userNumber);
void sendUserInfo(DECK card[], PLAYER usr[], THREAD tdata[],int player_deckPos[]);
void hit(PLAYER usr[],DECK card[], THREAD tdata[], int socketNumber, int* deckPosition, int message);
void dealerTurn(PLAYER usr[],DECK card[], THREAD tdata[], int socketNumber, int* deckPosition, int message);
void flushSocket(THREAD tdata[]);
//-----------------------------------------------------------------------------------------------------------------------

int main( int argc, char* args[] ) {

    srand(time(NULL));
    DECK card[54];
    PLAYER usr[5];
    int dP;
    int* deckPosition = &dP;
    *deckPosition = 0; // current card playing position in deck
    card_init(card,usr); // // Klient, server
    shuffleDeck(card); // Server
    while(1){
    server(card,usr,deckPosition);
    }
    return 0;
}

void deal_cards(PLAYER usr[],DECK card[], THREAD tdata[], int* deckPosition){

    int i,j;
// Player score, handposition
    usr[0].score = 0;
    usr[1].score = 0;
    usr[2].score = 0;
    usr[0].handPos = 0;
    usr[1].handPos = 0;
    usr[2].handPos = 0;

//player score positions
    usr[0].x2 = 565;
    usr[1].x2 = 685;
    usr[2].x2 = 425;

    usr[0].y2 = 10;
    usr[1].y2 = 500;
    usr[2].y2 = 500;

//player card positions
    usr[0].x1 = 565;
    usr[1].x1 = 685;
    usr[2].x1 = 425;

    usr[0].y1 = 150;
    usr[1].y1 = 380;
    usr[2].y1 = 380;

//win, lose or busted message
    usr[0].x3 = 730;
    usr[0].y3 = 5;
    usr[1].x3 = 820;
    usr[1].y3 = 400;
    usr[2].x3 = 300;
    usr[2].y3 = 400;


    //*deckPosition = 0;
    for(i=0;i<5;i++){

        if(*deckPosition > 51) {
            shuffleDeck(card);
            *deckPosition = 0;
        }

        // Rectangles for positioning
        if(i==0) { //dealar first card
            cardRect(card,usr,deckPosition,0);
            checkHandValue(usr, card, 0, deckPosition);
        }

        if(i>0 && i < 3) { //player second and third card
            cardRect(card,usr,deckPosition,1);
            checkHandValue(usr, card, 1, deckPosition);
        }

        if(i>2 && i < 5) { //player second and third card
            cardRect(card,usr,deckPosition,2);
            checkHandValue(usr, card, 2, deckPosition);
        }

        for(j=0;j<2;j++) {
            sendDeckStruct(card, deckPosition, tdata[0].tconsocket[j]); // send current card to client
            printf("Deckposition: %d\n", *deckPosition);
        }
    }
    for(i=0;i<2;i++) {
        for(j=0;j<3;j++)
            sendUsrStruct(usr,j, tdata[0].tconsocket[i]);
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

void card_init(DECK card [], PLAYER usr[]){
    int i;
    int gameValue = 1;

    for (i=0;i<54;++i){
        sprintf(card[i].path,"grafik/cards/%d.bmp",i);
        if(i > 1 && i < 10 || i > 14 && i < 23 || i > 27 && i < 36 || i > 40 && i < 49){ // all cards between 2 and 9
            card[i].game_value=gameValue;
        }

        if (i>9 && i<14 || i>22 && i<27 || i>35 && i<40 || i>48 && i<53){ // All tens, jacks, queens and kings equal 10
            card[i].game_value = 10;
        }

        if (i==1 || i==14 || i==27 || i==40){ // ACE:s
            card[i].game_value=11;
            gameValue = 1;
        }
        ++gameValue;
        card[53].game_value=0;

    }
}

void server(DECK card[], PLAYER usr[], int* deckPosition) {
    int server_socket,i=0, j;
    int listen_socket; // socket used to listen for incoming connections
    struct sockaddr_in serv, dest;
    char msg[] = "Connected with server.\n";
    int message;
    int consocket;
    int player_deckPos[22]={0};
    int send_flag_if_hit=5; // send flag to waiting client so he can recv the card the playing client recived
    bool dealCards = true;
    bool hitting = true;
    bool hitMe = true;
    bool startGame = false;
    bool quit = false;
    int test = 0;

    // thread and mutex initialization
    THREAD tdata[5]; // this is the threads individual data(struct server_threads)
    i = 0; // reseting the variable for future use

    int pid=getpid();
    socklen_t socksize = sizeof(struct sockaddr_in);
    memset(&serv, 0, sizeof(serv));           // zero the struct before filling the fields
    serv.sin_family = AF_INET;                // set the type of connection to TCP/IP
    serv.sin_addr.s_addr = htonl(INADDR_ANY); // set our address to any interface
    serv.sin_port = htons(PORTNUM);
    listen_socket = socket(AF_INET, SOCK_STREAM, 0); assert(listen_socket!=-1);
    test=bind(listen_socket, (struct sockaddr *)&serv, sizeof(struct sockaddr)); assert(test==0);
    test=listen(listen_socket, 4); assert(test==0); // a maximum of 4 connections simultaniously can be  made


    //Main-Accept loop
    while(quit==false) {
        consocket = accept(listen_socket, (struct sockaddr *)&dest, &socksize);
        if(consocket == -1) {  // if accept fails to initialize connection -> return value == -1
            continue;
        }
        printf("Incoming connection\n");
        usr[i+1].tot_holding=1000;
        tdata[0].tconsocket[i] = consocket;
        send(tdata[0].tconsocket[i], &i, sizeof(i), 0);
        send(tdata[0].tconsocket[i], &usr[i+1].tot_holding, sizeof(usr[i+1].tot_holding), 0);
        ++i;
        if(i==2) {
            startGame = true;
            //fork() här kanske?
        }
        printf("Player number %d is connected\n", i);

        while(startGame == true) {

            for(i=0;i<2;++i){
                recv(tdata[0].tconsocket[i], &usr[i+1].bet, sizeof(usr[i+1].bet), 0);
            }
            printf("bet: %d\n",usr[i+1].bet);
            send_flag_if_hit=5;
            while(dealCards == true) {
                deal_cards(usr,card,tdata, deckPosition);
                dealCards = false;
            }
            while(dealCards == false && hitMe == true) {
                for(i=0;i<2;i++){
                    send(tdata[0].tconsocket[i], &i, sizeof(i), 0); // VILKEN SPELARE
                    hitting = true;
                    while(hitting == true){
                        test = recv(tdata[0].tconsocket[i], &message, sizeof(message), 0); // 0= hit 1=STAND
                        if(test == ENOTCONN){
                            send(tdata[0].tconsocket[1], &test, sizeof(test), 0);
                            send(tdata[0].tconsocket[0], &test, sizeof(test), 0);
                            return;
                        }
                        if (message==666){quit=true; message=1;}
                        if(i==0 && message==0){send(tdata[0].tconsocket[1], &send_flag_if_hit, sizeof(send_flag_if_hit), 0);}
                        if(i==1 && message==0){send(tdata[0].tconsocket[0], &send_flag_if_hit, sizeof(send_flag_if_hit), 0);}
                        printf("\n MESSAGE: %d", message);
                        hit(usr,card,tdata,i,deckPosition,message);

                        if(message == 1){
                            hitting = false;
                        }
                    }
                    if(i == 1){
                        hitMe = false;
                    }
                }
            } // GAMELOOP för klientspel

            send_flag_if_hit=6;
            send(tdata[0].tconsocket[0], &send_flag_if_hit, sizeof(send_flag_if_hit), 0);
            dealerTurn(usr,card,tdata,0,deckPosition,1);
            if(quit==true){
                startGame=false;
                close(tdata[0].tconsocket[0]);
                close(tdata[0].tconsocket[1]);
                return;
            }
            flushSocket(tdata);
            sleep(5);
            dealCards = true;
            hitting = true;
            hitMe = true;

        } // START GAME

    } // ACCEPT LOOP

}

void hit(PLAYER usr[],DECK card[], THREAD tdata[], int socketNumber, int* deckPosition, int message){
    int i=0,j;
    socketNumber +=1;
    if(message == 0) { //if HIT message is received
         if(*deckPosition > 51) {
             shuffleDeck(card);
            *deckPosition = 0;
        }
        cardRect(card,usr,deckPosition,socketNumber);
        checkHandValue(usr, card, socketNumber,deckPosition); // calculate client current hand
        for(i=0;i<2;i++) {
            sendDeckStruct(card, deckPosition, tdata[0].tconsocket[i]); // send current card to client
            sleep(1); // LETS CLIENT 2 Flush his socket
        }
        for(i=0;i<2;i++) {
                sendUsrStruct(usr,socketNumber, tdata[0].tconsocket[i]);
        }
    }
}

void dealerTurn(PLAYER usr[],DECK card[], THREAD tdata[], int socketNumber, int* deckPosition, int message) {
    int i;
    if(message == 1){ //if STAND message is received

        while(usr[0].score < 17) {

            if(*deckPosition>51) {
                shuffleDeck(card);
                *deckPosition=0;
            }
            cardRect(card,usr,deckPosition,0);
            checkHandValue(usr, card, 0, deckPosition);
            for(i=0;i<2;i++) {
                sendDeckStruct(card, deckPosition, tdata[0].tconsocket[i]); // send current card to client
                sendUsrStruct(usr,0, tdata[0].tconsocket[i]);
            }
            sleep(1); // Card display delay
        }
    }
}

void cardRect(DECK card [],PLAYER usr [], int* deckPosition, int userNumber) {
    int i=0;
    *deckPosition += 1;
    card[*deckPosition].CardPos.x= usr[userNumber].x1;
    card[*deckPosition].CardPos.y= usr[userNumber].y1;
    card[*deckPosition].CardPos.w=75;
    card[*deckPosition].CardPos.h=111;
    usr[userNumber].x1 +=12;
    usr[userNumber].y1 -=12;
}

void checkHandValue(PLAYER usr[], DECK card[], int user, int* deckPosition) { // calculates current hand value
    usr[user].hand[ usr[user].handPos ] = card[*deckPosition].game_value; // stores current card value in postion j of hand array
    ++usr[user].handPos;
    usr[user].score +=card[*deckPosition].game_value;
    int i;
    if(usr[user].score > 21) { // if current score is greater than 21
        for(i=0;i<usr[user].handPos+1;i++){ //if current hand has card equal to 11 and player score is greater than 21
            if(usr[user].hand[i] == 11 && usr[user].score > 21){
                usr[user].score -= 10; // decrement total score with 10
                usr[user].hand[i] = 1; // ace in hand gets the value 1
            }
        }
    }
}

void sendDeckStruct(DECK card[],int *deckPosition, int socketNumber) {
    char x[100];
    char y[100];
    char gameValue[100];

    sprintf(gameValue, "%d", card[*deckPosition].game_value);
    sprintf(x, "%d", card[*deckPosition].CardPos.x);
    sprintf(y, "%d", card[*deckPosition].CardPos.y);

    send(socketNumber, &gameValue, sizeof(gameValue), 0);
    send(socketNumber, &x, sizeof(x), 0);
    send(socketNumber, &y, sizeof(y), 0);
    send(socketNumber, &card[*deckPosition].path, sizeof(card[*deckPosition].path), 0);
}

void sendUsrStruct(PLAYER usr[],int user, int socketNumber) {
    char x1[100];
    char x2[100];
    char x3[100];
    char y1[100];
    char y2[100];
    char y3[100];
    char score[100];

    sprintf(x1, "%d", usr[user].x1);
    sprintf(x2, "%d", usr[user].x2);
    sprintf(x3, "%d", usr[user].x3);
    sprintf(y1, "%d", usr[user].y1);
    sprintf(y2, "%d", usr[user].y2);
    sprintf(y3, "%d", usr[user].y3);
    sprintf(score, "%d", usr[user].score);
    send(socketNumber, &x1, sizeof(x1), 0);
    send(socketNumber, &x2, sizeof(x2), 0);
    send(socketNumber, &x3, sizeof(x3), 0);
    send(socketNumber, &y1, sizeof(y1), 0);
    send(socketNumber, &y2, sizeof(y2), 0);
    send(socketNumber, &y3, sizeof(y3), 0);
    send(socketNumber, &score, sizeof(score), 0);

}

void flushSocket(THREAD tdata[]) {
    char flush[1500] = {0};
    int byte = 1,i;
    while(byte>0) {
        for(i=1;i<3;++i){
           byte= recv(tdata[0].tconsocket[i], &flush, sizeof(flush), MSG_DONTWAIT);
           printf("\n TRASH: %d\n", byte);
        }
    }
}


