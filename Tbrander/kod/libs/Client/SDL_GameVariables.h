#ifndef SDL_VARIABLES_H_INCLUDED
#define SDL_VARIABLES_H_INCLUDED
#include <stdbool.h>

SDL_Surface* loadSurface(char* path); //Loads individual image
extern SDL_Window* window;           //The window, initialized  in blackjack_client.C
extern SDL_Surface* screen;          // see above
extern bool running;

#endif // SDL_VARIABLES_H_INCLUDED
