/****************************************************************
* Requirements: See comment below about required libraries      *
* Requires the Structs from: structs_Server.h                   *
*                                                               *
* Purpose: Function handles the blackjack logic and dealer AI.  *
*****************************************************************/

#ifndef BLACKJACK_H_INCLUDED
#define BLACKJACK_H_INCLUDED

#include "structs_Server.h"

extern void checkHandValue(PLAYER usr[], DECK card[], int user, int* deckPosition);
extern void hit(PLAYER usr[],DECK card[], THREAD tdata[], int socketNumber, int* deckPosition, int message);
extern void dealerTurn(PLAYER usr[],DECK card[], THREAD tdata[], int socketNumber, int* deckPosition, int message);


#endif // BLACKJACK_H_INCLUDED
