#include <SDL2/SDL.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 650
#define WINDOW_TITLE "Projekt Casino"


struct card{
    char path[100];
    int type;
    int value;
};
typedef struct card DECK;

void card_init(DECK card[]);
void dealc(DECK card[],DECK p1[], DECK com[]);
void shuffleDeck(DECK card[]);
void playP1(DECK p1[],DECK card[],int*,int*);
void playDeal(DECK com[],DECK card[],int*,int*);

void main(){

    int next=5;
    int totp1;
    int totdeal;
    srand(time(NULL));
    DECK p1[10];
    DECK com[10];
    DECK card[60];
    card_init(card);
    shuffleDeck(card);
    dealc(card,p1,com);

    totp1=p1[0].value+p1[1].value;
    totdeal=com[0].value+com[1].value;

    playP1(p1,card,&next,&totp1);
    playDeal(com,card,&next,&totdeal);

    printf("\nPlayer score: %d \nDealer score: %d",totp1,totdeal);
}

void playDeal(DECK com [], DECK card [],int *next,int *totdeal){


    int r=2;
    printf("\nDealers turn, first the face down card will flip up.\nDealers cards: ");
    printf("%d %d",com[0].value,com[1].value);
    while(*totdeal<17){
        printf("\nDealer draws a new card.\n");
        com[r]=card[*next];
        *next++;
        printf("It's value: %d",com[r].value);
        *totdeal=*totdeal+com[r].value;
        r++;
    }
    printf("\nDealer stands at %d",*totdeal);

}

void playP1(DECK p1 [],DECK card [],int *next,int *totp1){

    int sv=0, r=2,i;
    while(sv!=2){
        if(*totp1>=21)break;
        printf("\nDo you want to 1.hit, 2.stay (pick 1 or 2)\n\n");
        scanf("%d",&sv);
        if(sv==1){
            p1[r]=card[*next];
            *next++;
            printf("Your cards: ");
            for(i=0;i<=r;i++){
                printf("%d ",p1[i].value);

            }
            *totp1=*totp1+p1[r].value;
            r++;
        }
    }
}

void dealc(DECK card [], DECK p1 [], DECK com []){
//Här delas korten ut i början.
    p1[0]=card[1];
    com[0]=card[2];
    p1[1]=card[3];
    com[1]=card[4];
    printf("Player 1:s cards values: %d and %d\n\n",p1[0].value,p1[1].value);

    printf("Dealers card value: %d and one card face down\n",com[0].value);
}

void card_init(DECK card []){
    int i,j=0;
    char tmp[5];
    for (i=0;i<53;++i){
        strcpy(card[i].path,"grafik/");
        snprintf(tmp,5,"%d",i);
        strcat(card[i].path,tmp);
        strcat(card[i].path,".bmp");
        //printf("%s\n",card[i].path);

    }
    for (i=0;i<53;++i){

        // Hearts
        if(i<10){ // 0-9 (0=back)
            card[i].value=j;
            ++j;
        }
        else if (i>9 && i<14){ //10-13
            j=1;
            card[i].value=10;
            }

        //Clubbs
        else if(i>13 && i < 23){ // 14-22
            card[i].value=j;
            ++j;
        }
        else if(i>22 && i<27){ // 23-26
            j=1;
            card[i].value=10;

            }
        //Diamonds
        else if(i>26 && i < 36){ // 27-35
            card[i].value=j;
            ++j;
            }
        else if(i>35 && i<40){ // 36-39
            j=1;
            card[i].value=10;

            }
            //Spades
        else if(i>39 && i < 49){ // 40-48
            card[i].value=j;
            ++j;
            }
        else if(i>48 && i<53){ // 49-52
            j=1;
            card[i].value=10;
            }
        printf("%d\n",card[i].value);
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


