#ifndef CASINO_GRAPHIC_H_INCLUDED
#define CASINO_GRAPHIC_H_INCLUDED
#include "structs.h"
#include "SDL_GameVariables.h"
#include "txt_display.h"
#include <stdbool.h>

extern bool loadMedia(DECK card[], int cardNumberOnScreen, PLAYER usr[],int);
extern void waiting_for_other_player(DECK card[], int cardNumberOnScreen, PLAYER usr [],int myPlayerNumber);

#endif // CASINO_GRAPHIC_H_INCLUDED
