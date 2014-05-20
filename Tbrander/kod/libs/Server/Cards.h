#ifndef CARDS_H_INCLUDED
#define CARDS_H_INCLUDED
#include "structs_Server.h"
#include "Card_Initialize.h"


extern void deal_cards(PLAYER usr[],DECK card[], THREAD tdata[], int* deckPosition);
extern void shuffleDeck(DECK card[]);
extern void cardRect(DECK card [],PLAYER usr [], int* deckPosition, int userNumber);

#endif // CARDS_H_INCLUDED
