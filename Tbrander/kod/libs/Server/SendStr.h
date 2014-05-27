/******************************************************************
* Requirements: See comment below about required libraries        *
* Requires the Structs from: structs_Server.h                     *
*                                                                 *
* Purpose: This library focuses on the communication with the     *
* Client. It will convert the struct into strings and send it to  *
* the specified client.                                           *
*******************************************************************/

#ifndef SENDSTR_H_INCLUDED
#define SENDSTR_H_INCLUDED
#include "structs_Server.h"

extern void sendUsrStruct(PLAYER usr[],int user, int socketNumber);
extern void sendDeckStruct(DECK card[], int *deckPosition, int socketNumber);

#endif // SENDSTR_H_INCLUDED
