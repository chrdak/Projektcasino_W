/****************************************************************
* Requirements: See comment below about required libraries      *
* Bet function for the client. Requires the functions:          *
* -Loadmedia (casino graphics.h), and the structs located in  *
* (structs.h), PlaySoundEffect(sound.h)                         *
*                                                               *
* Purpose: Function handles betting for every client. Note that *
* the function doesn`t send any data to the server.             *
*                                                               *
*****************************************************************/


#ifndef BET_H_INCLUDED
#define BET_H_INCLUDED

/////////Defined Libraries Required///////
#include "structs.h"
#include "SDL_GameVariables.h"
#include "Casino_graphic.h"
#include "sound.h"
#include "txt_display.h"
////////////////////////////////////////////

extern void bet_client(int myPlayerNr,PLAYER user[],int cardNumberOnScreen,DECK card[]);

#endif // BET_H_INCLUDED
