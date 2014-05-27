/****************************************************************                                                       *
* Purpose: Function handles data sent from the Server, for every*
* client. Converts string to appropiate data type               *
* Note that the function requires the structs from struct.h.    *
*                                                               *
*                                                               *
*****************************************************************/

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "recvStr.h"

void recvStruct(DECK card[],int cardNumberOnScreen, int client_socket) {
    char x[100];
    char y[100];
    char gameValue[100];
    int test = 0;

    test = recv(client_socket, &gameValue, sizeof(gameValue), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &x, sizeof(x), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &y, sizeof(y), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &card[cardNumberOnScreen].path, sizeof(card[cardNumberOnScreen].path), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }

    sscanf(gameValue, "%d", &card[cardNumberOnScreen].game_value);
    sscanf(x, "%d", &card[cardNumberOnScreen].CardPos.x);
    sscanf(y, "%d", &card[cardNumberOnScreen].CardPos.y);
    card[cardNumberOnScreen].CardPos.w=75;
    card[cardNumberOnScreen].CardPos.h=111;

}

void recvUsrStruct(PLAYER usr[],int user, int client_socket) {

    char x1[100];
    char x2[100];
    char x3[100];
    char y1[100];
    char y2[100];
    char y3[100];
    char score[100];
    int test=0;

    test = recv(client_socket, &x1, sizeof(x1), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &x2, sizeof(x2), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &x3, sizeof(x3), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &y1, sizeof(y1), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &y2, sizeof(y2), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &y3, sizeof(y3), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }
    test = recv(client_socket, &score, sizeof(score), 0);
    if(test == ENOTCONN){
        perror("\nServer closed connection\n");
    }

    sscanf(x1, "%d", &usr[user].x1);
    sscanf(x2, "%d", &usr[user].x2);
    sscanf(x3, "%d", &usr[user].x3);

    sscanf(y1, "%d", &usr[user].y1);
    sscanf(y2, "%d", &usr[user].y2);
    sscanf(y3, "%d", &usr[user].y3);
    sscanf(score, "%d", &usr[user].score);
}
