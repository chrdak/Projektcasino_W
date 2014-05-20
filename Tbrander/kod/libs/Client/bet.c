#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "bet.h"
#include "Casino_graphic.h"
#include "sound.h"
#include "txt_display.h"

void bet_client(int myPlayerNr,PLAYER user[],int cardNumberOnScreen,DECK card[]){
        int x,y,bet=0,stand=0,betSig=1;
        SDL_Event event;
        running=false;
        loadMedia(card,-1,user,myPlayerNr);
        while(betSig==1){
            while( SDL_PollEvent( &event )) {// Check if user is closing the window --> then call quit
                 switch( event.type){

                    case SDL_QUIT:
                        exit(0);

                    case SDL_MOUSEBUTTONDOWN:   {// button clicks

                            x = event.button.x; // used to know where on x-axis is currently being clicked
                            y = event.button.y; // used to know where on y-axis is currently being clicked

                            if (event.button.button == (SDL_BUTTON_LEFT)){

                            // + 1
                                    if(x>70 && x< 70+55 && y>86 && y<86+55 && user[myPlayerNr].tot_holding>=1) { // can only be clicked while gameplay is true
                                        bet+=1;
                                        user[myPlayerNr].tot_holding-=1;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // + 10
                                    if(x>70 && x< 70+55 && y>150 && y<150+55 && user[myPlayerNr].tot_holding>=10) { // can only be clicked while gameplay is true

                                        bet+=10;
                                        user[myPlayerNr].tot_holding-=10;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // + 50
                                    if(x>70 && x< 70+55 && y>215 && y<215+55 && user[myPlayerNr].tot_holding>=50) { // can only be clicked while gameplay is true

                                        bet+=50;
                                        user[myPlayerNr].tot_holding-=50;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // + 100
                                    if(x>70 && x< 70+55 && y>277 && y<277+55 && user[myPlayerNr].tot_holding>=100) { // can only be clicked while gameplay is true

                                        bet+=100;
                                        user[myPlayerNr].tot_holding-=100;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // BET BUTTON
                                    if(x>550 && x< 550+98 && y>530 && y<530+49) {
                                        if (user[myPlayerNr].bet <=0){
                                            display_message(user,79, "Place your bet please!");
                                            SDL_UpdateWindowSurface(window);
                                            sleep(1); // delay the message
                                        }
                                        else{
                                        user[myPlayerNr].bet=bet;
                                        betSig=0;
                                        playSoundEffect("sound/chipsStack4.wav",-1);
                                        break;
                                        }
                                    }
                                playSoundEffect("sound/chipsStack4.wav",-1);

                            }// LEFT MOUSE

            // ---------------------------------------------------------------------------------------------------------

                            // RIGHT MOUSE

                            // - 1
                            if (event.button.button == (SDL_BUTTON_RIGHT)){
                                    if(x>70 && x< 70+55 && y>86 && y<86+55 && user[myPlayerNr].bet >= 1) { // can only be clicked while gameplay is true

                                            bet-=1;
                                            user[myPlayerNr].tot_holding+=1;
                                            user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // - 10
                                    if(x>70 && x< 70+55 && y>150 && y<150+55 && user[myPlayerNr].bet >= 10) { // can only be clicked while gameplay is true

                                        bet-=10;
                                        user[myPlayerNr].tot_holding+=10;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // - 50
                                    if(x>70 && x< 70+55 && y>215 && y<215+55 && user[myPlayerNr].bet >= 50) { // can only be clicked while gameplay is true

                                        bet-=50;
                                        user[myPlayerNr].tot_holding+=50;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);
                                    }

                            // - 100
                                    if(x>70 && x< 70+55 && y>277 && y<277+55 && user[myPlayerNr].bet >= 100) { // can only be clicked while gameplay is true

                                        bet-=100;
                                        user[myPlayerNr].tot_holding+=100;
                                        user[myPlayerNr].bet=bet;
                                        printf("\nNew bet: %d\n",user[myPlayerNr].bet);
                                        printf("\nHoldings: %d\n",user[myPlayerNr].tot_holding);

                                    }
                                    playSoundEffect("sound/chipsStack4.wav",-1);
                        }// RIGHT MOUSE
                        loadMedia(card,-1,user,myPlayerNr);
                        display_bet_holding(user,card,myPlayerNr,cardNumberOnScreen);
                        break;
                    } // case Mousedown
                case SDL_MOUSEMOTION: {
                    SDL_UpdateWindowSurface(window);
                }
                }// SWITCH
            }// While inner
    } // While outer
    running=true;
}



