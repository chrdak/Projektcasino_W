/****************************************************************
* Requirements: See comment below about required libraries      *
* Graphic functions for the client. Requires the functions:     *
* -display_bet osv (txt_display.h), and the structs located in  *
* (structs.h)                                                   *
*                                                               *
* Purpose: Function handles the Gui for every client. Note that *
* the function doesn`t send any data to the server.             *
*                                                               *
*****************************************************************/

#ifndef CASINO_GRAPHIC_H_INCLUDED
#define CASINO_GRAPHIC_H_INCLUDED

////////////////Defined_Libraries///////////////////
#include "structs.h"
#include "SDL_GameVariables.h"
#include "txt_display.h"
#include <stdbool.h>
////////////////////////////////////////////////////

extern bool loadMedia(DECK card[], int cardNumberOnScreen, PLAYER usr[],int);
extern void waiting_for_other_player(DECK card[], int cardNumberOnScreen, PLAYER usr [],int myPlayerNumber);

#endif // CASINO_GRAPHIC_H_INCLUDED
