#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "SendStr.h"

void sendDeckStruct(DECK card[],int *deckPosition, int socketNumber) {
    char x[100];
    char y[100];
    char gameValue[100];

    sprintf(gameValue, "%d", card[*deckPosition].game_value);
    sprintf(x, "%d", card[*deckPosition].CardPos.x);
    sprintf(y, "%d", card[*deckPosition].CardPos.y);

    send(socketNumber, &gameValue, sizeof(gameValue), 0);
    send(socketNumber, &x, sizeof(x), 0);
    send(socketNumber, &y, sizeof(y), 0);
    send(socketNumber, &card[*deckPosition].path, sizeof(card[*deckPosition].path), 0);
}

void sendUsrStruct(PLAYER usr[],int user, int socketNumber) {
    char x1[100];
    char x2[100];
    char x3[100];
    char y1[100];
    char y2[100];
    char y3[100];
    char score[100];

    sprintf(x1, "%d", usr[user].x1);
    sprintf(x2, "%d", usr[user].x2);
    sprintf(x3, "%d", usr[user].x3);
    sprintf(y1, "%d", usr[user].y1);
    sprintf(y2, "%d", usr[user].y2);
    sprintf(y3, "%d", usr[user].y3);
    sprintf(score, "%d", usr[user].score);
    send(socketNumber, &x1, sizeof(x1), 0);
    send(socketNumber, &x2, sizeof(x2), 0);
    send(socketNumber, &x3, sizeof(x3), 0);
    send(socketNumber, &y1, sizeof(y1), 0);
    send(socketNumber, &y2, sizeof(y2), 0);
    send(socketNumber, &y3, sizeof(y3), 0);
    send(socketNumber, &score, sizeof(score), 0);

}
