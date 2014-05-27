/********************************************************************
* Purpose: Essential structs that the application blackjack_server.c*
* and it`s libraries is dependent upon. Contains variables required *
* to initialize the cards, their graphical representation, and their*
* specific values. Note that the last struct holds specific sockets *
* for all the clients.                                              *
*********************************************************************/


#ifndef STRUCTS_H_INCLUDED
#define STRUCTS_H_INCLUDED
#include <SDL2/SDL.h>
struct card{
    char path[100]; // String will be loaded with a specific path in which the bmp file of the card is located at.
    int game_value;
    SDL_Surface* card_img;
    SDL_Rect CardPos;
};

struct player_pos_value{
    int score,x1,y1,x2,y2,x3,y3,bet,tot_holding;
    int hand[11]; // Array that represents a players hand, Every index contains info about the specific card and it`s color, values etc.
    int handPos;

};

struct server_threads{
    int tconsocket[5]; // the threads own connectionsocket
};

typedef struct card DECK;
typedef struct player_pos_value PLAYER;
typedef struct server_threads THREAD;



#endif // STRUCTS_H_INCLUDED
