#ifndef TXT_DISPLAY_H_INCLUDED
#define TXT_DISPLAY_H_INCLUDED
#include "structs.h"
#include "SDL_GameVariables.h"

extern void display_score(PLAYER usr[],int userNumber);
extern void display_message(PLAYER usr[],int userNumber, char message[]);
extern void display_bet_holding(PLAYER user[],DECK card[],int,int cardNumberOnScreen);

#endif // TXT_DISPLAY_H_INCLUDED
