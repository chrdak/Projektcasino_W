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
void shuffleDeck();
void deal_init();
void* serve_client (void* parameters);    // thread function
void server();
void reset_players(int);
void checkHandValue(int user,int nHit);
//-------------------------------------------------

/*Global variables*/
pthread_mutex_t mutex[5];             // An global array of 5 mutexes
int checksock[10] = {0};              // Busy socket check
int test;                             // assert()
int send_flagHit,send_flagDealer,send_flagWinner;
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
DECK player0_hand[15];
DECK player1_hand[15];
DECK player2_hand[15];
DECK player3_hand[15];
DECK player4_hand[15];
PLAYER user[5];
NCLIENT nClient[5];
THREAD threads_with_player[5];

//************************************ MAIN *********************************************

int main() {
    srand(time(NULL));
    card_init();
    shuffleDeck(); // Server
    server();
    //game_running();
    return 0;
}
//***************************************************************************************

void reset_players(int nthread){

        user[nthread].bet=0;
        user[nthread].score=0;
        user[nthread].card_tot=0;
        user[nthread].stand=0;
        user[nthread].turn=0;
        user[nthread].winner=0;
        user[nthread].quit=0;
        user[nthread].dealerTurn=1; //    1 = true     0 = false

}

void deal_init(){ // Initialize nClient before we send the struct to every connected player.

    int i,cardNr; // dealNr = Vilken deal/varv runt bordet, cardNr = vilket kort taget fr card[]
    // The inner loop will give each users hand[] a card from the shuffled deck, card[], and also add the score.
    // The outer loop determins how many cards each player will recive, two for the initial deal.


        for(cardNr=0;cardNr<2;++cardNr){

            player0_hand[cardNr]=card[deckPosition];
            user[0].score+=card[deckPosition++].game_value;
            user[0].card_tot+=1;

            player1_hand[cardNr]=card[deckPosition];
            user[1].score+=card[deckPosition++].game_value;
            user[1].card_tot+=1;

            player2_hand[cardNr]=card[deckPosition];
            user[2].score+=card[deckPosition++].game_value;
            user[2].card_tot+=1;

            player3_hand[cardNr]=card[deckPosition];
            user[3].score+=card[deckPosition++].game_value;
            user[3].card_tot+=1;

            player4_hand[cardNr]=card[deckPosition];
            user[4].score+=card[deckPosition++].game_value;
            user[4].card_tot+=1;

        }
    /*
    X, Y positions
    Dealer = 0
    Player = 1, 2, 3, 4
    */
    user[0].xPos=500;
    user[0].yPos=90;

    user[1].xPos=840;
    user[1].yPos=300;

    user[2].xPos=600;
    user[2].yPos=350;

    user[3].xPos=380;
    user[3].yPos=350;

    user[4].xPos=160;
    user[4].yPos=300;

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


void checkHandValue(int which_usr,int nHit){
    int i;

    if (deckPosition>52){
        shuffleDeck();
        deckPosition=1;
        }

        // DEALER
        if(which_usr == 0) {
             player0_hand[which_usr]= card[deckPosition];
             user[0].score+=card[deckPosition].game_value;
             user[0].card_tot+=1;


             if(user[0].score > 21) { // if current score is greater than 21
                for(i=0;i<user[0].card_tot+1;i++){ //if current hand has card equal to 11 and player score is greater than 21
                    if(player0_hand[which_usr].game_value ==11 && user[0].score > 21){
                        user[0].score -= 10; // decrement total score with 10
                        player0_hand[which_usr].game_value = 1; // ace in hand gets the value 1
                    }
                }
            }
        }


        // PLAYER 1
        else if(which_usr == 1) {
            player1_hand[which_usr]= card[deckPosition];
            user[1].score+=card[deckPosition].game_value;
            user[1].card_tot+=1;

             if(user[1].score > 21) { // if current score is greater than 21
                for(i=0;i<user[1].card_tot+1;i++){ //if current hand has card equal to 11 and player score is greater than 21
                    if(player1_hand[which_usr].game_value ==11 && user[1].score > 21){
                        user[1].score -= 10; // decrement total score with 10
                        player1_hand[which_usr].game_value = 1; // ace in hand gets the value 1
                    }
                }
            }
        }


        // PLAYER 2
        else if(which_usr == 2) {
            player2_hand[which_usr]= card[deckPosition];
            user[2].score+=card[deckPosition].game_value;
            user[2].card_tot+=1;

             if(user[2].score > 21) { // if current score is greater than 21
                for(i=0;i<user[2].card_tot+1;i++){ //if current hand has card equal to 11 and player score is greater than 21
                    if(player2_hand[which_usr].game_value ==11 && user[2].score > 21){
                        user[2].score -= 10; // decrement total score with 10
                        player2_hand[which_usr].game_value = 1; // ace in hand gets the value 1
                    }
                }
            }
        }


        // PLAYER 3
        else if(which_usr == 3) {
             player3_hand[which_usr]= card[deckPosition];
             user[3].score+=card[deckPosition].game_value;
             user[3].card_tot+=1;

             if(user[3].score > 21) { // if current score is greater than 21
                for(i=0;i<user[3].card_tot+1;i++){ //if current hand has card equal to 11 and player score is greater than 21
                    if(player3_hand[which_usr].game_value ==11 && user[3].score > 21){
                        user[3].score -= 10; // decrement total score with 10
                        player3_hand[which_usr].game_value = 1; // ace in hand gets the value 1
                    }
                }
            }
        }


        // PLAYER 4
        else if(which_usr == 4) {
             player4_hand[which_usr]= card[deckPosition];
             user[4].score+=card[deckPosition].game_value;
             user[4].card_tot+=1;

             if(user[4].score > 21) { // if current score is greater than 21
                for(i=0;i<user[4].card_tot+1;i++){ //if current hand has card equal to 11 and player score is greater than 21
                    if(player4_hand[which_usr].game_value ==11 && user[4].score > 21){
                        user[4].score -= 10; // decrement total score with 10
                        player4_hand[which_usr].game_value = 1; // ace in hand gets the value 1
                    }
                }
            }
        }


    ++deckPosition;
}

/*    SERVERKOD   */
void server() {
    int usr_tot=0;                        // Number of users connected
    int server_socket,i,consocket;
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
            user[0].quit=0;
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
        user[usr_tot].quit=0;
        for (i=0;i<5;++i){
                threads_with_player[i].n_users=usr_tot;
            }

        pthread_create(&thread_id[usr_tot], NULL, &serve_client, (void *)&tdata[usr_tot]);


        //------------------------------------------------------------------------------------------------
        // Kod för att kolla om användare hoppat ur när en ny ansluter
        // om så är fallet kan den nya användaren ta den lediga platsen vid bordet
        checksock[usr_tot]=usr_tot; // Array that stores each user/threadnumber connected [0,1,2,3,4]
        usr_tot = 0;
        for(i=0;i<6;++i){ // test each space in the array
            if(checksock[i] == i)
            {
                usr_tot++;
                //printf("\n%d\n",checksock[i]);
            }
        }
        if (usr_tot==5){++usr_tot;}
    } // inner while
} // outer while

}


void* serve_client (void* parameters) {  //thread_function
    THREAD* p = (THREAD*) parameters;
    while(game==true){/* Wait for next deal */}
    switch(p->nthread) {

        case 0: { // Dealer
            pthread_mutex_lock(&mutex[p->nthread]); // LOCK
            for(;;){

                sleep(1);
                int i, playerTurn[5]={0},nHit=0;
                while(player_tot<1){ /*Wait for player*/}
                if (deckPosition>52){
                    shuffleDeck();
                    deckPosition=1;
                }
                for(i=1;i<5;i++) { // Copy the connected users
                    playerTurn[i]=checksock[i];
                    threads_with_player[i].playerCon[i] = checksock[i];
                }
                game=true;
                deal_init(deckPosition);
                first_deal=1; // Begin the game

                for(i=1;i<5;++i){   // Turn
                    if(playerTurn[i]!=0){
                        turn=i;
                        user[i].turn=i;
                        while(turn!=0){/*Wait for case to finish*/}
                    }
                }

                while(user[0].score<17){ // Dealer draws to 17
                    checkHandValue(p->nthread,++nHit);
                    }
                    send_flagDealer=1;

                for(i=1;i<5;++i){ // Check winner
                    if(playerTurn[i]!=0){
                        if(user[i].score > user[0].score && user[i].score < 22 || user[0].score > 21) { // if win show yellow color rect
                            user[i].winner=1;

                        }
                        if(user[0].score > user[i].score && user[0].score < 22 || user[i].score > 21) { // if lose show red color rect
                            user[i].winner=0;

                        }
                    }
                }

                send_flagWinner=1;
                first_deal=0;
                printf("\ncase 0: GAME OVER Dealer stats: score %d, winner %d\n",user[0].score ,user[0].winner);
                game=false;
            } // Dealer loop
            pthread_mutex_unlock(&mutex[p->nthread]);
            break;
        }
        case 1: { // Threadfunction1/ user1
            pthread_mutex_lock(&mutex[p->nthread]);
            user[p->nthread].tot_holding=1000;
            nClient[p->nthread].nUser.nthread=p->nthread;
            send(p->tconsocket[p->nthread],&p->nthread, sizeof(int),0); // send nthread
            int stand=0,nHit=1;
            player_tot++;

            while(user[p->nthread].quit!=1){ // Game loop for case 1
                printf("\n************************** NEW GAME ****************************\n");
                reset_players(1);
                while(first_deal == 0){ sleep(1);/* STAY HERE UNTIL GAME READY */}
                send(p->tconsocket[p->nthread], &user[p->nthread].tot_holding, sizeof(user[p->nthread].tot_holding), 0); // Skickar kapital
                recv(p->tconsocket[p->nthread], &user[p->nthread].bet, sizeof(user[p->nthread].bet), 0); // Tar emot bet
                ++allBetRecv;
                printf("\nCASE 1, Bet mottaget: %d\n",user[p->nthread].bet);
                while(allBetRecv<player_tot){
                    //För att alla användare ska vara i synk väntar vi i en loop tills alla bet har kommit från samtliga spelare till servern.
                }
                allBetRecv=0;
                // send everything to the client. All players cards, bets, positions.
                send(p->tconsocket[p->nthread], &user[p->nthread], sizeof(PLAYER), 0);
                printf("\nCASE 1, väntar på tur: %d\n",user[p->nthread].turn);
                while(turn!=p->nthread){ // wait for turn and send info while waiting
                    if (send_flagHit==1){
                        send(p->tconsocket[p->nthread], &user[p->nthread], sizeof(PLAYER), 0);
                        send_flagHit=0;
                    }
                }
                printf("\nCASE 1, vår tur: %d tryck hit eller stand!\n",user[p->nthread].turn);
                if (user[p->nthread].score >=21){ user[p->nthread].stand=1;} // Om Vi fick 21 på första dealen så
                while(user[p->nthread].stand==0){
                    recv(p->tconsocket[p->nthread], &user[p->nthread], sizeof(PLAYER), 0); // send bet // måste blocka här pga stand 1 eller 0.
                    printf("\nCASE 1, stand mottaget: %d\n",user[p->nthread].stand);
                    if(user[p->nthread].stand==0){
                        checkHandValue(p->nthread, ++nHit); // nya kort delas ut här
                        if (user[p->nthread].score>=21){
                            user[p->nthread].stand=1;
                            user[p->nthread].turn=0;
                            send(p->tconsocket[p->nthread], &card[p->nthread], sizeof(DECK), 0);
                            send(p->tconsocket[p->nthread], &user[p->nthread], sizeof(PLAYER), 0);
                            send_flagHit=1;
                            printf("\nCASE 1, Hit! Tjock: poäng: %d\n",user[p->nthread].score);
                            break;
                        }
                        printf("\nCASE 1, Hit! Ej tjock, poäng: %d tryck hit eller stand!\n",user[p->nthread].score);
                        send_flagHit=1;
                    }
                    if(user[p->nthread].stand==0){
                        send(p->tconsocket[p->nthread], &card[p->nthread], sizeof(DECK), 0);
                        send(p->tconsocket[p->nthread], &user[p->nthread], sizeof(PLAYER), 0);
                        }
                }
                user[p->nthread].turn=0;
                printf("\nCASE 1, turen är över: %d)\n",user[p->nthread].turn);
                turn=0;

                while(send_flagDealer==0){ printf("\nCASE 1, väntar på dealer, din poäng: %d\n",user[p->nthread].score); sleep(1);/* FAST */ }
                    user[p->nthread].dealerTurn=0;
                    send(p->tconsocket[p->nthread], & user[p->nthread], sizeof(PLAYER), 0);
                while(send_flagWinner==0){ /* VÄNTAR */   }
                    send(p->tconsocket[p->nthread], & user[p->nthread], sizeof(PLAYER), 0);
                    printf("\nCASE 1, vinnarbeslut (1 = du vinner, 0 = dealern vinner): %d\n",user[p->nthread].winner);
                 // While not quit

                printf("\nCASE 1, GAME OVER\n");
            }
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

