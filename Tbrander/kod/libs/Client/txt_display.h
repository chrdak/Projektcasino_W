/****************************************************************
* Requirements: See comment below about required libraries      *
* Graphic functions for the client. Requires the library:       *
* -SDL_GameVariables and structs from the struct.h file         *
*                                                               *
* Purpose: Function handles text in the Gui for every client.   *
* Note that the function doesn`t send any data to the server.   *
*                                                               *
*                                                               *
*****************************************************************/

#ifndef TXT_DISPLAY_H_INCLUDED
#define TXT_DISPLAY_H_INCLUDED

/////////////Defined_Libraries//////////////////
#include "structs.h"
#include "SDL_GameVariables.h"
////////////////////////////////////////////////

extern void display_score(PLAYER usr[],int userNumber);
extern void display_message(PLAYER usr[],int userNumber, char message[]);
extern void display_bet_holding(PLAYER user[],DECK card[],int,int cardNumberOnScreen);

#endif // TXT_DISPLAY_H_INCLUDED
