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
void card_init(); // Initialize the card deck
void game_running();
void shuffleDeck();
void deal_init();
void* serve_client (void* parameters);    // thread function
void server();
void reset_players();
//-------------------------------------------------

/*Global variables*/
pthread_mutex_t mutex[5];             // An global array of 5 mutexes
int checksock[10] = {0};              // Busy socket check
int test;                             // assert()
//--------------------------------------
//TRÅD VARIABLER
int allBetRecv=0;                     // Keeps track of incomming bets (for sync reasons)
int first_deal=0;
int player_tot=0;
int usr_tot=0;
int turn=0;
int deckPosition=0;
bool game = false;
//-------------------------------------
DECK card[60];
PLAYER usr[5];
NCLIENT nClient[5];


//************************************ MAIN *********************************************

int main() {
    srand(time(NULL));
    card_init();
    server();
    game_running();
    return 0;
}
//***************************************************************************************

void reset_players(){
    int i,j;
    for(i=0;i<5;++i){
        nClient[i].player.bet=0;
        nClient[i].player.score=0;
        nClient[i].player.card_tot=0;
        nClient[i].player.stand=0;
        nClient[i].player.turn=0;
        nClient[i].player.winner=0;
        nClient[i].player.quit=0;
        nClient[i].player.dealerTurn=0;
    }
}

void deal_init(){ // Initialize nClient before we send the struct to every connected player.

    int i,dealNr; // dealNr = Vilken deal/varv runt bordet, cardNr = vilket kort taget fr card[]
    // The inner loop will give each users hand[] a card from the shuffled deck, card[], and also add the score.
    // The outer loop determins how many cards each player will recive, two for the initial deal.

    for(dealNr=0;dealNr<2;++dealNr){
        for(i=0;i<=player_tot;++i){
            nClient[i].hand[dealNr] = card[deckPosition]; // The dealer will recive card[0], the blue back piece
            nClient[i].player.score+=card[deckPosition].game_value;
            nClient[i].player.card_tot+=1;
            ++(deckPosition);
        }
    }
    // Update the total number of connected users.

    /*
    X, Y positions
    Dealer = 0
    Player = 1, 2, 3, 4
    */
    nClient[0].player.xPos=500;
    nClient[0].player.yPos=90;

    nClient[1].player.xPos=840;
    nClient[1].player.yPos=300;

    nClient[2].player.xPos=600;
    nClient[2].player.yPos=350;

    nClient[3].player.xPos=380;
    nClient[3].player.yPos=350;

    nClient[4].player.xPos=160;
    nClient[4].player.yPos=300;

}

void game_running(){
    shuffleDeck(); // Server
    card_init();

}

void shuffleDeck(){
    int i,j;
    DECK tmp[60];
    for (i=1; i<53;++i){
        j= rand()%52+1;
        tmp[i]=card[i];
        card[i]=card[j];
        card[j]=tmp[i];
    }
}

void card_init(){
    int i;
    int gameValue = 1;
    int realValue = 1;

    for (i=0;i<54;++i){
        sprintf(card[i].path,"grafik/cards/%d.bmp",i);
        if(i > 1 && i < 10 || i > 14 && i < 23 || i > 27 && i < 36 || i > 40 && i < 49){ // all cards between 2 and 9
            card[i].game_value=gameValue;
            card[i].real_value=realValue;
            card[i].type=i/13 +1;

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


void checkHandValue(int usr,int nHit){
    int i;
    if (deckPosition>52){
        shuffleDeck();
        deckPosition=1;
        }
    nClient[usr].hand[nHit] = card[deckPosition]; // The dealer will recive card[0], the blue back piece
    nClient[usr].player.score+=card[deckPosition].game_value;
    ++deckPosition;
    if(nClient[usr].player.score > 21) { // if current score is greater than 21
        for(i=0;i<nClient[usr].player.card_tot+1;i++){ //if current hand has card equal to 11 and player score is greater than 21
            if(nClient[usr].hand[i].game_value ==11 && nClient[usr].player.score > 21){
                nClient[usr].player.score -= 10; // decrement total score with 10
                nClient[usr].hand[i].game_value = 1; // ace in hand gets the value 1
            }
        }
    }

}

/*    SERVERKOD   */
void server() {
    int usr_tot=0;                        // Number of users connected
    int server_socket,i,consocket;
    game=true;
    int listen_socket; // socket used to listen for incoming connections
    struct sockaddr_in serv, dest;
    char msg[] = "Connected with server.\n";
    char buffer[100]={0};
    // thread and mutex initialization
    THREAD tdata[5]; // this is the threads individual data(struct server_threads)
    pthread_t thread_id[5];  // thread ID given to 5 elements in the array
    for(i = 0; i < 5; i++)
    {
        pthread_mutex_init (&mutex[i], NULL);// initialize mutex i where i: 0..4
    }
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


   while(1){
    //Accept loop
    if (usr_tot>5){
        printf("Server queue full\n");
        sleep(1);
    }
    while(usr_tot<5) {

        consocket = accept(listen_socket, (struct sockaddr *)&dest, &socksize);

        if(consocket == -1) {  // if accept fails to initialize connection -> return value == -1
            continue;
        }
        //Dealer initilization: The dealer will have a separate thread without the need of an client
        if(usr_tot==0){
            nClient[0].player.quit=0;
            tdata[0].tconsocket[0] = consocket;
            tdata[0].nthread = 0;
            usr_tot++;
            tdata[0].n_users = usr_tot;
            pthread_create(&thread_id[0], NULL, &serve_client, (void *)&tdata[0]);
            checksock[0]=0; // Array that stores each user/threadnumber connected [0,1,2,3,4]
        }
        // Each individual client will be servered by a thread.     // Problem: Dealer och användare1 delar på samma socket
        tdata[0].tconsocket[usr_tot] = consocket;
        tdata[usr_tot].tconsocket[usr_tot] = consocket;
        tdata[0].n_users = usr_tot;  // the number of users currently connected must be known to thread[0]/Dealer
        tdata[usr_tot].nthread = usr_tot;
        nClient[usr_tot].player.quit=0;
        for (i=0;i<5;++i){
                nClient[i].nUser.n_users=usr_tot;
            }

        pthread_create(&thread_id[usr_tot], NULL, &serve_client,(void *)&tdata[usr_tot]);

        //------------------------------------------------------------------------------------------------
        // Kod för att kolla om användare hoppat ur när en ny ansluter
        // om så är fallet kan den nya användaren ta den lediga platsen vid bordet
        checksock[usr_tot]=usr_tot; // Array that stores each user/threadnumber connected [0,1,2,3,4]
        usr_tot = 0;
        for(i=0;i<6;++i){ // test each space in the array
            if(checksock[i] == i)
            {
                usr_tot++;
            }
        }
        if (usr_tot==5){++usr_tot;}
    } // inner while
} // outer while

}


void* serve_client (void* parameters) {  //thread_function
    THREAD* p = (THREAD*) parameters;
    while(game==true){/* Wait for next deal */}
    int send_flagHit=0,send_flagDealer=0,send_flagWinner=0;
    switch(p->nthread) {
        case 0: { // Dealer
            for(;;){
                pthread_mutex_lock(&mutex[p->nthread]); // LOCK

                int i, playerTurn[5]={0},nHit=0;
                while(player_tot<1){ /*Wait for player*/}
                if (deckPosition>52){
                    shuffleDeck();
                    deckPosition=1;
                }
                for(i=1;i<5;i++) { // Copy the connected users
                    playerTurn[i]=checksock[i];
                    nClient[i].nUser.playerCon[i] = checksock[i];
                }
                game=true;

                reset_players();
                deal_init(deckPosition);
                first_deal=1; // Begin the game

                for(i=1;i<5;++i){   // Turn
                    if(playerTurn[i]!=0){
                        turn=i;
                        nClient[i].player.turn=i;
                        while(turn!=0){/*Wait for case to finish*/}
                    }
                }

                while(nClient[0].player.score<17){ // Dealer draws to 17
                    checkHandValue(p->nthread,++nHit);
                    send_flagDealer=1;
                    }

                for(i=1;i<5;++i){ // Check winner
                    if(playerTurn[i]!=0){
                        if(nClient[i].player.score > nClient[0].player.score && nClient[i].player.score < 22 || nClient[0].player.score > 21) { // if win show yellow color rect
                            nClient[i].player.winner=1;
                        }
                        if(nClient[0].player.score > nClient[i].player.score && nClient[0].player.score < 22 || nClient[i].player.score > 21) { // if lose show red color rect
                            nClient[i].player.winner=0;
                        }
                    }
                }

                send_flagWinner=1;
                first_deal=0;
                game=false;
            } // Dealer loop
            pthread_mutex_unlock(&mutex[p->nthread]);
            break;
        }
        case 1: { // Threadfunction1/ user1
            pthread_mutex_lock(&mutex[p->nthread]);
            nClient[p->nthread].nUser.nthread=p->nthread;
            send(p->tconsocket[p->nthread], &p->nthread, sizeof(int), 0); // send nthread
            int stand=0,nHit=1;
            player_tot++;
            while(nClient[p->nthread].player.quit!=1){ // Game loop for the thread
                while(first_deal == 0){}
                recv(p->tconsocket[p->nthread],&nClient[p->nthread].player.bet,sizeof(nClient[p->nthread].player.bet),0); // Bet förfrågan
                ++allBetRecv;
                while(allBetRecv<player_tot){
                    //För att alla användare ska vara i synk väntar vi i en loop tills alla bet har kommit från samtliga spelare till servern.
                }
                // send everything to the client. All players cards, bets, positions.
                send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);

                while(turn!=p->nthread){ // wait for turn and send info while waiting
                    if (send_flagHit==1){
                        send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);
                        send_flagHit=0;
                    }
                }
                while(nClient[p->nthread].player.stand==0){
                    send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0); // send the turn to client
                    // recive hit, stand
                    recv(p->tconsocket[p->nthread],&nClient[p->nthread].player.stand,sizeof(nClient[p->nthread].player.stand),0);
                    if(nClient[p->nthread].player.stand==0){
                        checkHandValue(p->nthread, ++nHit);
                        if (nClient[p->nthread].player.score>=21){
                            nClient[p->nthread].player.stand=1;
                            send_flagHit=1;
                            break;
                        }
                        send_flagHit=1;
                    }
                }
                turn=0;
                while(send_flagDealer==0){ /* wait for turn and send info while waiting*/}
                    nClient[p->nthread].player.dealerTurn=1;
                    send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);

                while(send_flagWinner==0){ /* wait for winner decision and send info while waiting*/}
                    nClient[p->nthread].player.dealerTurn=0;
                    send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);

                } // While not quit

            usr_tot=p->nthread; // When quiting
            checksock[p->nthread] = 0;
            pthread_mutex_unlock(&mutex[p->nthread]);
            // Stay in a loop and wait for the dealer to make contact.
            // Based  on the information ->(waiting for turn): only make changes to your surroundings and loop, your turn: calculate game vaules,and send to dealer and loop
            // (user has exited): Reset all your values, notify the server that the slot is empty(setting i == p->nthread) and unlock your mutex -> Return to main
            break;
        }
        case 2: { // Threadfunction2/ user2
            pthread_mutex_lock(&mutex[p->nthread]);
            nClient[p->nthread].nUser.nthread=p->nthread;
            send(p->tconsocket[p->nthread], &p->nthread, sizeof(int), 0); // send nthread
            int stand=0,nHit=1;
            player_tot++;
            while(nClient[p->nthread].player.quit!=1){ // Game loop for the thread
                while(first_deal == 0){}
                recv(p->tconsocket[p->nthread],&nClient[p->nthread].player.bet,sizeof(nClient[p->nthread].player.bet),0); // Bet förfrågan
                ++allBetRecv;
                while(allBetRecv<player_tot){
                    //För att alla användare ska vara i synk väntar vi i en loop tills alla bet har kommit från samtliga spelare till servern.
                }
                // send everything to the client. All players cards, bets, positions.
                send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);

                while(turn!=p->nthread){ // wait for turn and send info while waiting
                    if (send_flagHit==1){
                        send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);
                        send_flagHit=0;
                    }
                }
                while(nClient[p->nthread].player.stand==0){
                    send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0); // send the turn to client
                    // recive hit, stand
                    recv(p->tconsocket[p->nthread],&nClient[p->nthread].player.stand,sizeof(nClient[p->nthread].player.stand),0);
                    if(nClient[p->nthread].player.stand==0){
                        checkHandValue(p->nthread, ++nHit);
                        if (nClient[p->nthread].player.score>=21){
                            nClient[p->nthread].player.stand=1;
                            send_flagHit=1;
                            break;
                        }
                        send_flagHit=1;
                    }
                }
                turn=0;
                while(send_flagDealer==0){ /* wait for turn and send info while waiting*/}
                    nClient[p->nthread].player.dealerTurn=1;
                    send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);

                while(send_flagWinner==0){ /* wait for winner decision and send info while waiting*/}
                    nClient[p->nthread].player.dealerTurn=0;
                    send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);

                } // While not quit

            usr_tot=p->nthread; // When quiting
            checksock[p->nthread] = 0;
            pthread_mutex_unlock(&mutex[p->nthread]);
            // Stay in a loop and wait for the dealer to make contact.
            // Based  on the information ->(waiting for turn): only make changes to your surroundings and loop, your turn: calculate game vaules,and send to dealer and loop
            // (user has exited): Reset all your values, notify the server that the slot is empty(setting i == p->nthread) and unlock your mutex -> Return to main
            break;
        }
        case 3: { // Threadfunction3/ user3
            pthread_mutex_lock(&mutex[p->nthread]);
            nClient[p->nthread].nUser.nthread=p->nthread;
            send(p->tconsocket[p->nthread], &p->nthread, sizeof(int), 0); // send nthread
            int stand=0,nHit=1;
            player_tot++;
            while(nClient[p->nthread].player.quit!=1){ // Game loop for the thread
                while(first_deal == 0){}
                recv(p->tconsocket[p->nthread],&nClient[p->nthread].player.bet,sizeof(nClient[p->nthread].player.bet),0); // Bet förfrågan
                ++allBetRecv;
                while(allBetRecv<player_tot){
                    //För att alla användare ska vara i synk väntar vi i en loop tills alla bet har kommit från samtliga spelare till servern.
                }
                // send everything to the client. All players cards, bets, positions.
                send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);

                while(turn!=p->nthread){ // wait for turn and send info while waiting
                    if (send_flagHit==1){
                        send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);
                        send_flagHit=0;
                    }
                }
                while(nClient[p->nthread].player.stand==0){
                    send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0); // send the turn to client
                    // recive hit, stand
                    recv(p->tconsocket[p->nthread],&nClient[p->nthread].player.stand,sizeof(nClient[p->nthread].player.stand),0);
                    if(nClient[p->nthread].player.stand==0){
                        checkHandValue(p->nthread, ++nHit);
                        if (nClient[p->nthread].player.score>=21){
                            nClient[p->nthread].player.stand=1;
                            send_flagHit=1;
                            break;
                        }
                        send_flagHit=1;
                    }
                }
                turn=0;
                while(send_flagDealer==0){ /* wait for turn and send info while waiting*/}
                    nClient[p->nthread].player.dealerTurn=1;
                    send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);

                while(send_flagWinner==0){ /* wait for winner decision and send info while waiting*/}
                    nClient[p->nthread].player.dealerTurn=0;
                    send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);

                } // While not quit

            usr_tot=p->nthread; // When quiting
            checksock[p->nthread] = 0;
            pthread_mutex_unlock(&mutex[p->nthread]);
            // Stay in a loop and wait for the dealer to make contact.
            // Based  on the information ->(waiting for turn): only make changes to your surroundings and loop, your turn: calculate game vaules,and send to dealer and loop
            // (user has exited): Reset all your values, notify the server that the slot is empty(setting i == p->nthread) and unlock your mutex -> Return to main
            break;
        }
        case 4: { // Threadfunction4/ user4
            pthread_mutex_lock(&mutex[p->nthread]);
            nClient[p->nthread].nUser.nthread=p->nthread;
            send(p->tconsocket[p->nthread], &p->nthread, sizeof(int), 0); // send nthread
            int stand=0,nHit=1;
            player_tot++;
            while(nClient[p->nthread].player.quit!=1){ // Game loop for the thread
                while(first_deal == 0){}
                recv(p->tconsocket[p->nthread],&nClient[p->nthread].player.bet,sizeof(nClient[p->nthread].player.bet),0); // Bet förfrågan
                ++allBetRecv;
                while(allBetRecv<player_tot){
                    //För att alla användare ska vara i synk väntar vi i en loop tills alla bet har kommit från samtliga spelare till servern.
                }
                // send everything to the client. All players cards, bets, positions.
                send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);

                while(turn!=p->nthread){ // wait for turn and send info while waiting
                    if (send_flagHit==1){
                        send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);
                        send_flagHit=0;
                    }
                }
                while(nClient[p->nthread].player.stand==0){
                    send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0); // send the turn to client
                    // recive hit, stand
                    recv(p->tconsocket[p->nthread],&nClient[p->nthread].player.stand,sizeof(nClient[p->nthread].player.stand),0);
                    if(nClient[p->nthread].player.stand==0){
                        checkHandValue(p->nthread, ++nHit);
                        if (nClient[p->nthread].player.score>=21){
                            nClient[p->nthread].player.stand=1;
                            send_flagHit=1;
                            break;
                        }
                        send_flagHit=1;
                    }
                }
                turn=0;
                while(send_flagDealer==0){ /* wait for turn and send info while waiting*/}
                    nClient[p->nthread].player.dealerTurn=1;
                    send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);

                while(send_flagWinner==0){ /* wait for winner decision and send info while waiting*/}
                    nClient[p->nthread].player.dealerTurn=0;
                    send(p->tconsocket[p->nthread], &nClient, sizeof(NCLIENT), 0);

                } // While not quit

            usr_tot=p->nthread; // When quiting
            checksock[p->nthread] = 0;
            pthread_mutex_unlock(&mutex[p->nthread]);
            // Stay in a loop and wait for the dealer to make contact.
            // Based  on the information ->(waiting for turn): only make changes to your surroundings and loop, your turn: calculate game vaules,and send to dealer and loop
            // (user has exited): Reset all your values, notify the server that the slot is empty(setting i == p->nthread) and unlock your mutex -> Return to main
            break;
        }


        default: {

        fprintf(stderr, "ERROR"); // Maximum number of users has been reached!
                    break;
        }

    }

}

