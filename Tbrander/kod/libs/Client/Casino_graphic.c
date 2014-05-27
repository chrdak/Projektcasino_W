/****************************************************************
* Requirements: See comment below about required libraries      *
* Graphic functions for the client. Requires the functions:     *
* -display_bet osv (txt_display.h), and the structs located in  *
* (structs.h)                                                   *
*                                                               *
* Purpose: Function handles the Gui for every client. Note that *
* the function doesn`t send any data to the server.             *
*                                                               *
*****************************************************************/

#include <SDL2/SDL.h>
#include <stdbool.h>

////////////////Defined_Libraries///////////////////
#include "Casino_graphic.h"
#include "txt_display.h"
#include "structs.h"
////////////////////////////////////////////////////

bool loadMedia(DECK card[], int cardNumberOnScreen, PLAYER usr [],int myPlayerNumber){
    //Loading success flag
    bool success = true;
    int i=0;
    // ------------------------------ LOAD BUTTONS AND GAMEBOARD ---------------------------------
    if (running==true){
        char game_table[50]="grafik/casino_betlight_off.bmp", hit_button[50]="grafik/hit_button.bmp",stand_button[50]="grafik/stand_button.bmp";
        SDL_Surface* table_lightsOff_img = NULL;        //Loaded converted table image
        SDL_Surface* hit_img = NULL;          //Loaded converted hit button image
        SDL_Surface* stand_img = NULL;        //Loaded converted stand button image
        table_lightsOff_img=loadSurface(game_table);
        hit_img=loadSurface(hit_button);
        stand_img=loadSurface(stand_button);
        SDL_Rect    hit_Rect;
        SDL_Rect    stand_Rect;
        hit_Rect.x=490;
        hit_Rect.y=530;
        hit_Rect.w=98;
        hit_Rect.h=49;
        stand_Rect.x=610;
        stand_Rect.y=530;
        stand_Rect.w=98;
        stand_Rect.h=49;
        SDL_BlitSurface(table_lightsOff_img, NULL, screen, NULL);
        SDL_BlitScaled(hit_img, NULL, screen, &hit_Rect);       // hit button
        SDL_BlitScaled(stand_img, NULL, screen, &stand_Rect);   // stand button
        SDL_FreeSurface(hit_img); // Frigör bild för hit-knapp bordet.
        SDL_FreeSurface(stand_img); // Frigör bild för stand-knapp bordet.
        SDL_FreeSurface(table_lightsOff_img);
        }
    if (running==false){
        char bet_table[50]="grafik/casino_betlight_on.bmp",bet_button[50]="grafik/bet_button.bmp";
        SDL_Surface* table_lightsOn_img = NULL;        //Loaded converted table image
        SDL_Surface* bet_img= NULL;
        table_lightsOn_img=loadSurface(bet_table);
        bet_img=loadSurface(bet_button);
        SDL_Rect    bet_Rect;
        bet_Rect.x=550;
        bet_Rect.y=530;
        bet_Rect.w=98;
        bet_Rect.h=49;
        SDL_BlitSurface(table_lightsOn_img, NULL, screen, NULL);
        SDL_BlitSurface(bet_img, NULL, screen, &bet_Rect);
        SDL_FreeSurface(table_lightsOn_img);
        SDL_FreeSurface(bet_img);
    }
    // ----------------------------------------------------------------------------------------------
    if (running==true){
        for(i=0;i<3;i++) {
            display_score(usr,i); // display user and dealer score
        }
    }
    for(i=0;i<cardNumberOnScreen+1;i++) {
        card[i].card_img = SDL_LoadBMP(card[i].path);
        SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);
        SDL_FreeSurface(card[i].card_img);
    }

    SDL_UpdateWindowSurface(window);
    display_bet_holding(usr,card,myPlayerNumber,cardNumberOnScreen);
    return success;
}


void waiting_for_other_player(DECK card[], int cardNumberOnScreen, PLAYER usr [],int myPlayerNumber){
        int i;
        char game_table[50]="grafik/casino_betlight_off.bmp";
        SDL_Surface* table_lightsOff_img = NULL;  //Loaded converted table image
        table_lightsOff_img=loadSurface(game_table);
        SDL_BlitSurface(table_lightsOff_img, NULL, screen, NULL);
        SDL_FreeSurface(table_lightsOff_img);
        for(i=0;i<3;i++) {
            display_score(usr,i); // display user and dealer score
        }
        for(i=0;i<cardNumberOnScreen+1;i++) {
            card[i].card_img = SDL_LoadBMP(card[i].path);
            SDL_BlitScaled(card[i].card_img, NULL, screen, &card[i].CardPos);
            SDL_FreeSurface(card[i].card_img);
        }
        display_bet_holding(usr,card,myPlayerNumber,cardNumberOnScreen);
}


