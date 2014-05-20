#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SendStr.h"
#include "Cards.h"

void deal_cards(PLAYER usr[],DECK card[], THREAD tdata[], int* deckPosition){

    int i,j;
// Player score, handposition
    usr[0].score = 0;
    usr[1].score = 0;
    usr[2].score = 0;
    usr[0].handPos = 0;
    usr[1].handPos = 0;
    usr[2].handPos = 0;

//player score positions
    usr[0].x2 = 565;
    usr[1].x2 = 685;
    usr[2].x2 = 425;

    usr[0].y2 = 10;
    usr[1].y2 = 500;
    usr[2].y2 = 500;

//player card positions
    usr[0].x1 = 565;
    usr[1].x1 = 685;
    usr[2].x1 = 425;

    usr[0].y1 = 120;
    usr[1].y1 = 380;
    usr[2].y1 = 380;

//win, lose or busted message
    usr[0].x3 = 730;
    usr[0].y3 = 5;
    usr[1].x3 = 820;
    usr[1].y3 = 400;
    usr[2].x3 = 300;
    usr[2].y3 = 400;


    //*deckPosition = 0;
    for(i=0;i<5;i++){

        if(*deckPosition > 51) {
            shuffleDeck(card);
            *deckPosition = 0;
        }

        // Rectangles for positioning
        if(i==0) { //dealar first card
            cardRect(card,usr,deckPosition,0);
            checkHandValue(usr, card, 0, deckPosition);
        }

        if(i>0 && i < 3) { //player second and third card
            cardRect(card,usr,deckPosition,1);
            checkHandValue(usr, card, 1, deckPosition);
        }

        if(i>2 && i < 5) { //player second and third card
            cardRect(card,usr,deckPosition,2);
            checkHandValue(usr, card, 2, deckPosition);
        }

        for(j=0;j<2;j++) {
            sendDeckStruct(card, deckPosition, tdata[0].tconsocket[j]); // send current card to client
            printf("Deckposition: %d\n", *deckPosition);
        }
    }
    for(i=0;i<2;i++) {
        for(j=0;j<3;j++)
            sendUsrStruct(usr,j, tdata[0].tconsocket[i]);
    }
}

void shuffleDeck(DECK card[]){
    int i,j;
    DECK tmp[60];
    for (i=1; i<53;++i){
        j= rand()%52+1;
        tmp[i]=card[i];
        card[i]=card[j];
        card[j]=tmp[i];
    }
}

void cardRect(DECK card [],PLAYER usr [], int* deckPosition, int userNumber) {
    int i=0;
    *deckPosition += 1;
    card[*deckPosition].CardPos.x= usr[userNumber].x1;
    card[*deckPosition].CardPos.y= usr[userNumber].y1;
    card[*deckPosition].CardPos.w=75;
    card[*deckPosition].CardPos.h=111;
    usr[userNumber].x1 +=12;
    usr[userNumber].y1 -=12;
}
