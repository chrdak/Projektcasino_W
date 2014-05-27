/******************************************************************
* Requirements: See comment below about required libraries        *
* Requires the Structs from: structs_Server.h(in cards.h)         *
*                                                                 *
* Purpose: Handles all the actions involving the cards from the   *
* structs specified above. The  first deal of cards to the clients*
* is also handled in the function: deal cards                     *
*******************************************************************/

#ifndef CARDS_H_INCLUDED
#define CARDS_H_INCLUDED
#include "structs_Server.h"
#include "Card_Initialize.h"


extern void deal_cards(PLAYER usr[],DECK card[], THREAD tdata[], int* deckPosition);
extern void shuffleDeck(DECK card[]);
extern void cardRect(DECK card [],PLAYER usr [], int* deckPosition, int userNumber);

#endif // CARDS_H_INCLUDED
