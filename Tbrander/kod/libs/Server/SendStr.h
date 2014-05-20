#ifndef SENDSTR_H_INCLUDED
#define SENDSTR_H_INCLUDED
#include "structs_Server.h"

extern void sendUsrStruct(PLAYER usr[],int user, int socketNumber);
extern void sendDeckStruct(DECK card[], int *deckPosition, int socketNumber);

#endif // SENDSTR_H_INCLUDED
