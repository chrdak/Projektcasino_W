#ifndef STRUCTS_H_INCLUDED
#define STRUCTS_H_INCLUDED
#include <SDL2/SDL.h>
struct card{
    char path[100];
    int game_value;
    SDL_Surface* card_img;
    SDL_Rect CardPos;
};

struct player_pos_value{
    int score,x1,y1,x2,y2,x3,y3,bet,tot_holding;
    int hand[11]; // Array som representerar en spelares hand, varje plats innehåller info om tilldelade kort, färg, värden..
    int handPos;

};

struct server_threads{
    int tconsocket[5]; // the threads own connectionsocket
};

typedef struct card DECK;
typedef struct player_pos_value PLAYER;
typedef struct server_threads THREAD;



#endif // STRUCTS_H_INCLUDED
