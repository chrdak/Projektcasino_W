/****************************************************************
* Requirements: See comment below about required libraries      *
* Graphic functions for the client. Requires the library:       *
* -SDL_GameVariables and structs from the struct.h file         *
*                                                               *
* Purpose: Function handles text in the Gui for every client.   *
* Note that the function doesn`t send any data to the server.   *
*                                                               *
*                                                               *
*****************************************************************/

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_ttf.h>

#include "txt_display.h"


void display_score(PLAYER usr[],int userNumber){

    SDL_Surface* text;                    // Score to be printed
    TTF_Font *font;                       // True type font to be loaded (*.ttf)
    char scoreToDisplay[20]={0};
    SDL_Color text_color = {255, 245, 0};
    SDL_Rect textLocation = { usr[userNumber].x2,usr[userNumber].y2, 0, 0 }; // Position of text, relative to user
    sprintf(scoreToDisplay,"Sum: %d",usr[userNumber].score); // Translate int to string, storing it in scoreToDisplay
    font = TTF_OpenFont("fonts/DejaVuSans.ttf", 16); // Open true type font
    text = TTF_RenderText_Blended(font,scoreToDisplay,text_color); // Blended = smoother edges, Solid = sharper edges
    SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
    SDL_FreeSurface(text);
    TTF_CloseFont(font);
}

void display_message(PLAYER usr[],int userNumber, char message[]){

    // MESSAGE FLAGS
    /*    79 = Message to player who's turn it is
    89 = Waiting for other player..
    99 = New game in 5..4..3..2..1.. starting../    */

    SDL_Surface* text;                    // Score to be printed
    TTF_Font *font;                       // True type font to be loaded (*.ttf)

    if (userNumber==79){
            SDL_Color text_color = {247, 215, 16};            SDL_Rect textLocation = { 60,540, 0, 0 };
            font = TTF_OpenFont("fonts/DejaVuSans.ttf", 36); // Open true type font
            text = TTF_RenderText_Blended(font,message,text_color); // Blended = smoother edges, Solid = sharper edges
            SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
            SDL_FreeSurface(text);
            TTF_CloseFont(font);

        } // Position of text, relative to user

    if (userNumber==89){
            SDL_Color text_color = {247, 215, 16};
            SDL_Rect textLocation = { 410,70, 0, 0 };
            font = TTF_OpenFont("fonts/DejaVuSans.ttf", 36); // Open true type font
            text = TTF_RenderText_Blended(font,message,text_color); // Blended = smoother edges, Solid = sharper edges
            SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
            SDL_FreeSurface(text);
            TTF_CloseFont(font);

        } // Position of text, relative to user
    if (userNumber==99){
        SDL_Color text_color = {247, 215, 16}; // YELLOW
        SDL_Rect textLocation = { 480,70, 0, 0 };
        font = TTF_OpenFont("fonts/DejaVuSans.ttf", 36); // Open true type font
        text = TTF_RenderText_Blended(font,message,text_color); // Blended = smoother edges, Solid = sharper edges
        SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
        SDL_FreeSurface(text);
        TTF_CloseFont(font);

    } // Position of text, relative to user
    if (userNumber==0 || userNumber==1){
        SDL_Color text_color = {155, 16, 13}; // RED
        SDL_Rect textLocation = { usr[userNumber].x3,usr[userNumber].y3, 0, 0 }; // Position of text, relative to user
        font = TTF_OpenFont("fonts/DejaVuSans.ttf", 20); // Open true type font/        text = TTF_RenderText_Blended(font,message,text_color); // Blended = smoother edges, Solid = sharper edges
        SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
        SDL_FreeSurface(text);
        TTF_CloseFont(font);
    }
}

void display_bet_holding(PLAYER user[],DECK card[],int myPlayerNr,int cardNumberOnScreen){

        SDL_Surface* text;                    // Score to be printed
        TTF_Font *font;                       // True type font to be loaded (*.ttf)
        char player_bet[20]="";
        char player_holdings[20]="";
        if(user[myPlayerNr].tot_holding>0){
            sprintf(player_bet, "Bet: %d", user[myPlayerNr].bet);
            sprintf(player_holdings, "Holding: %d", user[myPlayerNr].tot_holding);
        }
        if(user[myPlayerNr].tot_holding==0){
            sprintf(player_bet, "Bet: %d", user[myPlayerNr].bet);
            sprintf(player_holdings, "Insufficient funds");
        }
        SDL_Color text_color = {251, 218, 15}; // casinoyellow
        SDL_Rect textLocation = { 60,360, 0, 0 };
        font = TTF_OpenFont("fonts/DejaVuSans.ttf", 20); // Open true type font
        text = TTF_RenderText_Blended(font,player_bet,text_color); // Blended = smoother edges, Solid = sharper edges
        SDL_BlitSurface(text, NULL, screen, &textLocation); // Draw to screen
        SDL_FreeSurface(text);
        TTF_CloseFont(font);

        SDL_Rect textLocation_holdings = { 60,390, 0, 0 }; // Position of text, relativ to user
        font = TTF_OpenFont("fonts/DejaVuSans.ttf", 20); // Open true type font
        text = TTF_RenderText_Blended(font,player_holdings,text_color); // Blended = smoother edges, Solid = sharper edges
        SDL_BlitSurface(text, NULL, screen, &textLocation_holdings); // Draw to screen
        SDL_FreeSurface(text);
        TTF_CloseFont(font);
        SDL_UpdateWindowSurface(window);
}

