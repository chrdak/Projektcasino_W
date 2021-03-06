/****************************************************************
* Purpose: Function handles the initialization of SDL           *
*                                                               *
*                                                               *
*****************************************************************/

#ifndef SDL_INIT_H_INCLUDED
#define SDL_INIT_H_INCLUDED

/////////////////Defined_Libraries//////////////
#include "SDL_GameVariables.h"
////////////////////////////////////////////////

extern void SDL_initializer();
extern SDL_Surface* loadSurface(char* path);

#endif // SDL_INIT_H_INCLUDED
