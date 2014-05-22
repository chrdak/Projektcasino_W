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
#include "libs/Server/SendStr.h"
#include "libs/Server/Cards.h"
#include "libs/Server/Card_Initialize.h"
#include "libs/Server/Blackjack.h"
#define PORTNUM 6578
#define SOCK_PATH "Casino_socket"


/*FUNKTIONS PROTOTYPER*/

void server(DECK card[], PLAYER usr[], int* deckPosition);

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
    server(card,usr,deckPosition);
    return 0;
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

    int pid;
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
            // FORK WHEN SECOND PLAYER CONNECTS
           /* pid=fork();
            if (pid==0){ return;}*/
            startGame = true;
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
            sleep(5);
            dealCards = true;
            hitting = true;
            hitMe = true;

        } // START GAME

    } // ACCEPT LOOP

}
