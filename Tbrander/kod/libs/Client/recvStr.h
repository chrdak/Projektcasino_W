/****************************************************************                                                       *
* Purpose: Function handles data sent from the Server, for every*
* client. Converts string to appropiate data type               *
* Note that the function requires the structs from struct.h.    *
*                                                               *
*                                                               *
*****************************************************************/

#ifndef RECVSTR_H_INCLUDED
#define RECVSTR_H_INCLUDED
#include "structs.h"

extern void recvStruct(DECK card[],int cardNumberOnScreen, int client_socket);
extern void recvUsrStruct(PLAYER usr[],int user, int client_socket);

#endif // RECVSTR_H_INCLUDED
