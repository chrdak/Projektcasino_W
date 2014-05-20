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
};

typedef struct card DECK;
typedef struct player_pos_value PLAYER;



#endif // STRUCTS_H_INCLUDED
