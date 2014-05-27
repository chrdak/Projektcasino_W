/****************************************************************
* Requirements: See comment below about required libraries      *
* Requires the Structs from: structs_Server.h                   *
*                                                               *
* Purpose: Function handles the initialization of all the cards *
* from the specified library above.                             *
*****************************************************************/

#ifndef CARD_INITIALIZE_H_INCLUDED
#define CARD_INITIALIZE_H_INCLUDED
#include "structs_Server.h"

extern void card_init(DECK card [], PLAYER usr[]);

#endif // CARD_INITIALIZE_H_INCLUDED
