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
void card_init(DECK card[],PLAYER usr[]); // Initialize the card deck
void game_running(DECK card[],PLAYER usr[]);
void shuffleDeck(DECK card[]);
void deal_cards(PLAYER usr[],DECK card[]);
void* serve_client (void* parameters);    // thread function
void server(DECK card[], PLAYER usr[]);
//-------------------------------------------------

/*Global variables*/
pthread_mutex_t mutex[5];             // An global array of 5 mutexes
int checksock[4] = {0};               // Busy socket check
int test;
int usr_tot=0;              // Number of users connected
DECK card[60];     // Klient, server
PLAYER usr[5];     // Klient, server

//************************************ MAIN *********************************************

int main( int argc, char* args[] ) {

    srand(time(NULL)); // Server

    card_init(card,usr); // // Klient, server
    //shuffleDeck(card); // Server
    //deal_cards(usr,card);
    server(card,usr);
    game_running(card,usr); // game loop
 return 0;
}
//***************************************************************************************

void deal_cards(PLAYER usr[],DECK card[]){
    int i=0,playerNr=0;
    for(i=0;i<5;++i){ // 5 = Number of users inc dealer
        // Rectangles for positioning (deal 1)
        card[i].CardPos.x=usr[playerNr].x1;
        card[i].CardPos.y=usr[playerNr].y1;
        card[i].CardPos.w=70;
        card[i].CardPos.h=106;
        usr[playerNr].score+=card[i].game_value;
        ++playerNr;
    }
    playerNr=0;
        for(i=5;i<10;++i){// 5 = Number of users inc dealer
        // Rectangles for positioning (deal 2)
        card[i].CardPos.x=usr[playerNr].x2;
        card[i].CardPos.y=usr[playerNr].y2;
        card[i].CardPos.w=75;
        card[i].CardPos.h=111;
        usr[playerNr].score+=card[i].game_value;
        ++playerNr;
   }
}

void game_running(DECK card[],PLAYER usr[]){

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

/*    SERVERKOD   */
void server(DECK card[], PLAYER usr[]) {
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
       if(usr_tot >4) {
           fprintf(stderr, "MER ÄN 4!\n");
       }
    //Accept loop
    while(usr_tot<5) {
        consocket = accept(listen_socket, (struct sockaddr *)&dest, &socksize);
        if(consocket == -1) {  // if accept fails to initialize connection -> return value == -1
            continue;
        }
        //printf("Incoming connection from %s - sending welcome\n", inet_ntoa(dest.sin_addr)); //This code snippet will gen. an error. But it works anyway!
        //send(consocket, msg, strlen(msg), 0);

        //Dealer initilization: The dealer will have a separate thread without the need of an client
        if(usr_tot==0){
            tdata[0].tconsocket[0] = consocket;
            tdata[0].nthread = 0;
            pthread_create(&thread_id[0], NULL, &serve_client, (void *)&tdata[0]);
            usr_tot++;
        }
        // Each individual client will be servered by a thread.     // Problem: Dealer och användare1 delar på samma socket
        tdata[0].tconsocket[usr_tot] = consocket;
        tdata[usr_tot].tconsocket[usr_tot] = consocket;
        tdata[0].n_users = usr_tot;  // the number of users currently connected must be known to thread[0]/Dealer
        tdata[usr_tot].nthread = usr_tot;
        pthread_create(&thread_id[usr_tot], NULL, &serve_client,(void *)&tdata[usr_tot]);
        checksock[usr_tot]=usr_tot;
        usr_tot = 1;
        for(i=0;i<5;++i){
            if(checksock[i] == i)
            {
                usr_tot++;
            }
        }
    } // inner while
} // outer while

}


void* serve_client (void* parameters) {  //thread_function

    THREAD* p = (THREAD*) parameters;
    switch(p->nthread) {
        case 0: { // Dealer
            pthread_mutex_lock(&mutex[p->nthread]);
            // The number of users playing must be known to the dealer (n_users), if there are no users -> return to main(?)
            // which user is going to play against the dealer?, other users  must wait and will need to be notified by the dealer
            // Send information to the client -> forward the answer to the specific thread and wait for the threads answer
            // Make Calculations based on the answer from the thread. Compare it with your own values.
            //  Send the necessary values back to the client/clients
            //  Loop
            for(;;);
            pthread_mutex_unlock(&mutex[p->nthread]);
            break;
        }
        case 1: { // Threadfunction1/ user1

            char buffer[100]="case 1";
            pthread_mutex_lock(&mutex[p->nthread]);

            send(p->tconsocket[p->nthread],buffer,strlen(buffer),0);
            memset(&buffer[0], 0, sizeof(buffer));

            for(;;);


            usr_tot=p->nthread;
            pthread_mutex_unlock(&mutex[p->nthread]);
            // Stay in a loop and wait for the dealer to make contact.
            // Based  on the information ->(waiting for turn): only make changes to your surroundings and loop, your turn: calculate game vaules,and send to dealer and loop
            // (user has exited): Reset all your values, notify the server that the slot is empty(setting i == p->nthread) and unlock your mutex -> Return to main
            break;
        }
        case 2: { // Threadfunction2/ user2

            char buffer[100]="case 2";
            pthread_mutex_lock(&mutex[p->nthread]);

            send(p->tconsocket[p->nthread],buffer,strlen(buffer),0);
            memset(&buffer[0], 0, sizeof(buffer));

            for(;;);

            usr_tot=p->nthread;
            pthread_mutex_unlock(&mutex[p->nthread]);
            break;
        }
        case 3: { // Threadfunction3/ user3

            char buffer[100]="case 3";
            pthread_mutex_lock(&mutex[p->nthread]);

            send(p->tconsocket[p->nthread],buffer,strlen(buffer),0);
            memset(&buffer[0], 0, sizeof(buffer));

            for(;;);

            usr_tot=p->nthread;
            pthread_mutex_unlock(&mutex[p->nthread]);
            break;
        }
        case 4: { // Threadfunction4/ user4
            char buffer[100]="case 4";
            pthread_mutex_lock(&mutex[p->nthread]);

            send(p->tconsocket[p->nthread],buffer,strlen(buffer),0);
            memset(&buffer[0], 0, sizeof(buffer));

            for(;;);

            usr_tot=p->nthread;
            pthread_mutex_unlock(&mutex[p->nthread]);
            break;
        }
        default: {

        fprintf(stderr, "ERROR"); // Maximum number of users has been reached!
                    break;
        }

    }

}

