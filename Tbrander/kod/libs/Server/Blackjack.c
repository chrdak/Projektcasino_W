#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SendStr.h"
#include "Cards.h"
#include "Blackjack.h"
#include "Card_Initialize.h"

void hit(PLAYER usr[],DECK card[], THREAD tdata[], int socketNumber, int* deckPosition, int message){
    int i=0,j;
    socketNumber +=1;
    if(message == 0) { //if HIT message is received
         if(*deckPosition > 51) {
             shuffleDeck(card);
            *deckPosition = 0;
        }
        cardRect(card,usr,deckPosition,socketNumber);
        checkHandValue(usr, card, socketNumber,deckPosition); // calculate client current hand
        for(i=0;i<2;i++) {
            sendDeckStruct(card, deckPosition, tdata[0].tconsocket[i]); // send current card to client
            sleep(1); // LETS CLIENT 2 Flush his socket
        }
        for(i=0;i<2;i++) {
                sendUsrStruct(usr,socketNumber, tdata[0].tconsocket[i]);
        }
    }
}

void dealerTurn(PLAYER usr[],DECK card[], THREAD tdata[], int socketNumber, int* deckPosition, int message) {
    int i;
    if(message == 1){ //if STAND message is received

        while(usr[0].score < 17) {

            if(*deckPosition>51) {
                shuffleDeck(card);
                *deckPosition=0;
            }
            cardRect(card,usr,deckPosition,0);
            checkHandValue(usr, card, 0, deckPosition);
            for(i=0;i<2;i++) {
                sendDeckStruct(card, deckPosition, tdata[0].tconsocket[i]); // send current card to client
                sendUsrStruct(usr,0, tdata[0].tconsocket[i]);
            }
            sleep(1); // Card display delay
        }
    }
}

void checkHandValue(PLAYER usr[], DECK card[], int user, int* deckPosition) { // calculates current hand value
    usr[user].hand[ usr[user].handPos ] = card[*deckPosition].game_value; // stores current card value in postion j of hand array
    ++usr[user].handPos;
    usr[user].score +=card[*deckPosition].game_value;
    int i;
    if(usr[user].score > 21) { // if current score is greater than 21
        for(i=0;i<usr[user].handPos+1;i++){ //if current hand has card equal to 11 and player score is greater than 21
            if(usr[user].hand[i] == 11 && usr[user].score > 21){
                usr[user].score -= 10; // decrement total score with 10
                usr[user].hand[i] = 1; // ace in hand gets the value 1
            }
        }
    }
}
