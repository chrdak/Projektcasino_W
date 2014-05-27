/****************************************************************
* Requirements: See comment below about required libraries      *
* Requires the Structs from: structs_Server.h                   *
*                                                               *
* Purpose: Function handles the initialization of all the cards *
* from the specified library above.                             *
*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SendStr.h"
#include "Card_Initialize.h"

void card_init(DECK card [], PLAYER usr[]){
    int i;
    int gameValue = 1;

    for (i=0;i<54;++i){
        sprintf(card[i].path,"grafik/cards/%d.bmp",i);
        if(i > 1 && i < 10 || i > 14 && i < 23 || i > 27 && i < 36 || i > 40 && i < 49){ // all cards between 2 and 9
            card[i].game_value=gameValue;
        }

        if (i>9 && i<14 || i>22 && i<27 || i>35 && i<40 || i>48 && i<53){ // All tens, jacks, queens and kings equal 10
            card[i].game_value = 10;
        }

        if (i==1 || i==14 || i==27 || i==40){ // ACE:s
            card[i].game_value=11;
            gameValue = 1;
        }
        ++gameValue;
        card[53].game_value=0;

    }
}



