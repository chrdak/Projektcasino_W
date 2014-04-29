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
#pragma pack(1)
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
    int hand[11]; // Array som representerar en spelares hand, varje plats innehåller info om tilldelade kort, färg, värden..
    int handPos;
};                 // Plats [0] är första tilldelade kortet osv.
typedef struct player_pos_value PLAYER;

struct server_threads{
#pragma pack(0)
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
void deal_cards(PLAYER usr[],DECK card[], THREAD tdata[], int socketNumber, int* dP, int* cXp, int* cYp, int* dCxP, int* dCyP);
int server(DECK card[], PLAYER usr[], int* deckPosition, int* cXp, int* cYp, int* dCxP, int* dCyP);
void* serve_client (void* parameters);    // thread function
void checkAceValue(PLAYER usr[], DECK card[], int user, int dP);
void buttonListeninig(DECK card[], PLAYER usr[], THREAD tdata[], int message, int* deckPosition, int socketNumber, int* cXp, int* cYp, int* dCxP, int* dCyP);
void cardRect(DECK card [],int* dP, int* x,int* y);
//-------------------------------------------------

/*Global variables*/
pthread_mutex_t mutex[5];             // An global array of 5 mutexes
int consocket;
int test;
//************************************ MAIN *********************************************

int main( int argc, char* args[] ) {

    srand(time(NULL));
    DECK card[54];
    PLAYER usr[5];
    int dP;
    int cardXposition;
    int cardYposition;
    int dealerCardXposition;
    int dealerCardYposition;
    int* deckPosition = &dP;

    int* cXp = &cardXposition;
    int* cYp = &cardYposition;

    int* dCxP = &dealerCardXposition;
    int* dCyP = & dealerCardYposition;
    *deckPosition = 0; // current card playing position in deck
    *cXp = 0; // card x position on screen for player
    *cYp = 0;   // card y position on screen for player
    *dCxP = 0;  // card x position on screen for dealer
    *dCyP = 0;  // card y position on screen for dealer
    card_init(card,usr); // // Klient, server
    shuffleDeck(card); // Server
    server(card,usr,deckPosition,cXp,cYp,dCxP,dCyP);

    return 0;
}
//***************************************************************************************

void deal_cards(PLAYER usr[],DECK card[], THREAD tdata[], int socketNumber, int* dP, int* cXp, int* cYp, int* dCxP, int* dCyP){

    int i;
    usr[0].score = 0;
    usr[1].score = 0;
    usr[0].handPos = 0;
    usr[1].handPos = 0;
    *cXp = 880;
    *cYp = 290;
    *dCxP = 565;
    *dCyP = 150;
    *dP = 0;
    for(i=1;i<4;i++){

        // Rectangles for positioning
        if(i==1) { //dealar first card
            cardRect(card,dP,dCxP,dCyP);
            checkAceValue(usr, card, 0, i);
        }

        if(i>1 && i < 4) { //player second and third card
            cardRect(card,dP,cXp,cYp);
            checkAceValue(usr, card, 1, i);
        }
        send(tdata[0].tconsocket[socketNumber], &card[i], sizeof(DECK), 0); // send current card to client
    }
    send(tdata[0].tconsocket[socketNumber], &usr[1], sizeof(usr[1]), 0); // send current card to client
    send(tdata[0].tconsocket[socketNumber], &usr[0], sizeof(usr[0]), 0); // send current card to client
    *dP = i; // stores current card position

}
void game_running(DECK card[],PLAYER usr[]){
    char str[1000];
    while(1){
        recv(consocket,str,100,0);
        printf("%s\n",str);
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
int server(DECK card[], PLAYER usr[], int* deckPosition, int* cXp, int* cYp, int* dCxP, int* dCyP) {
    int server_socket,i=0, j;
    int listen_socket; // socket used to listen for incoming connections
    struct sockaddr_in serv, dest;
    char msg[] = "Connected with server.\n";
    int message;
    bool running = false;

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


    //Main-Accept loop
    while(1) {
        consocket = accept(listen_socket, (struct sockaddr *)&dest, &socksize);
        if(consocket == -1) {  // if accept fails to initialize connection -> return value == -1
            continue;
        }
        printf("Incoming connection\n");

        //Dealer initilization: The dealer will have a separate thread without the need of an client
        if(i==0){
            tdata[0].tconsocket[0] = consocket;
            tdata[0].nthread = 0;
            pthread_create(&thread_id[0], NULL, &serve_client, (void *)&tdata[0]);
            i++;
        }

        // Each individual client will be servered by a thread.     // Problem: Dealer och användare1 delar på samma socket
        tdata[0].tconsocket[i] = consocket;
        tdata[0].n_users = i;  // the number of users currently connected must be known to thread[0]/Dealer
        tdata[i].nthread = i;
        printf("%d\n", i);
        running = true;
        pthread_create(&thread_id[i], NULL, &serve_client,(void *)&tdata[i]);
      //  pthread_create(&filosof[i], NULL, &philosophize, (void *)&filosof_info[i]); // Skapar nya trådar och ger dem rätt nr! NYTT
        while(running == true) {

            recv(tdata[0].tconsocket[i], &message, sizeof(message), 0); // receives hit, stand, newgame messages from client

            if(message == 666){
                close(tdata[0].tconsocket[i]);
                --i;
                printf("%d\n", i);
                running = false;
            }
            buttonListeninig(card,usr,tdata,message,deckPosition,i,cXp,cYp,dCxP,dCyP);

        }

        //++i;
    }

}

void buttonListeninig(DECK card[], PLAYER usr[], THREAD tdata[], int message, int* deckPosition, int socketNumber, int* cXp, int* cYp, int* dCxP, int* dCyP) {

    int tempDeckPos = 0;
    if(message == 0) { //if HIT message is received
         cardRect(card,deckPosition,cXp,cYp);
         if(*deckPosition > 52) {
             shuffleDeck(card);
            *deckPosition = 1;
            cardRect(card,deckPosition,cXp,cYp);
        }

        tempDeckPos = *deckPosition;
        checkAceValue(usr, card, 1, tempDeckPos); // calculate client current hand
        send(tdata[0].tconsocket[socketNumber], (void*)&card[*deckPosition], sizeof(DECK), 0); // send card to client
        send(tdata[0].tconsocket[socketNumber], (void*)&usr[1], sizeof(usr[1]), 0); //send current hand information to client
        printf("Player: %d\n", usr[1].score);
    }

    if(message == 1){ //if STAND message is received

        printf("Player score: %d\n", usr[1].score);
        while(usr[0].score < 17) {

            cardRect(card,deckPosition,dCxP,dCyP);

            if(*deckPosition>52) {
                shuffleDeck(card);
                *deckPosition=1;
                cardRect(card,deckPosition,dCxP,dCyP);
            }

            tempDeckPos = *deckPosition;
            checkAceValue(usr, card, 0, tempDeckPos);
            send(tdata[0].tconsocket[socketNumber], (void*)&card[*deckPosition], sizeof(DECK), 0);
            send(tdata[0].tconsocket[socketNumber], (void*)&usr[0].score, sizeof(usr[0].score), 0);
            printf("Dealer: %d\n",usr[0].score );
        }
    }

    if(message == 3) { //if New game message is received from client
        shuffleDeck(card);
        deal_cards(usr,card,tdata, socketNumber, deckPosition,cXp,cYp,dCxP,dCyP);
        printf("New Game\n");
        printf("Dealer: %d\n", usr[0].score);
        printf("Player: %d\n", usr[1].score);
    }
}

void cardRect(DECK card [],int* dP, int* x,int* y) {
    *dP += 1;
    card[*dP].CardPos.x= *x;
    card[*dP].CardPos.y= *y;
    card[*dP].CardPos.w=75;
    card[*dP].CardPos.h=111;
    *x +=12;
    *y -=12;
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
            pthread_mutex_unlock(&mutex[p->nthread]);
            break;
        }
        case 1: { // Threadfunction1/ user1

            pthread_mutex_lock(&mutex[p->nthread]);
            pthread_mutex_unlock(&mutex[p->nthread]);
            // Stay in a loop and wait for the dealer to make contact.
            // Based  on the information ->(waiting for turn): only make changes to your surroundings and loop, your turn: calculate game vaules,and send to dealer and loop
            // (user has exited): Reset all your values, notify the server that the slot is empty(setting i == p->nthread) and unlock your mutex -> Return to main
            break;
        }
        case 2: { // Threadfunction2/ user2

            pthread_mutex_lock(&mutex[p->nthread]);
            pthread_mutex_unlock(&mutex[p->nthread]);
            break;
        }
        case 3: { // Threadfunction3/ user3

            pthread_mutex_lock(&mutex[p->nthread]);
            pthread_mutex_unlock(&mutex[p->nthread]);
            break;
        }
        case 4: { // Threadfunction4/ user4
            pthread_mutex_lock(&mutex[p->nthread]);
            pthread_mutex_unlock(&mutex[p->nthread]);
            break;
        }
        default: { // Maximum number of users has been reached!

        }

    }

}

void checkAceValue(PLAYER usr[], DECK card[], int user, int dP) { // calculates current hand value
    usr[user].hand[ usr[user].handPos ] = card[dP].game_value; // stores current card value in postion j of hand array
    ++usr[user].handPos;
    usr[user].score +=card[dP].game_value;
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
