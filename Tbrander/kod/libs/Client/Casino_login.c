/****************************************************************
* Requirements: See comment below about required libraries      *
* Graphic functions for the client. Requires the library:       *
* -SDL_GameVariables                                            *
*                                                               *
* Purpose: Function handles the startup_screen for every client.*
* Note that the function doesn`t send any data to the server.   *
*                                                               *
*                                                               *
*****************************************************************/

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

////////////////Defined_Libraries///////////////////
#include "Casino_login.h"
#include "SDL_GameVariables.h"
////////////////////////////////////////////////////

#define WINDOW_TITLE "Projekt Casino"


void login_init(){
    SDL_Event event;                      //Event- for user interaction
    SDL_Window* login_window=NULL;
    SDL_Surface* login_img = NULL;
    int SDL_test,x,y;
    char login[50]="grafik/login.bmp";
    SDL_test=SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); assert(SDL_test==0);

    if (SDL_test<0){
        printf( "SDL2 could not initialize! SDL2_Error: %s\n", SDL_GetError() );
        exit(1);
    }
    // Login screen
    login_window = SDL_CreateWindow(
    WINDOW_TITLE,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    650,
    500,
    SDL_WINDOW_SHOWN);                  assert(login_window!=NULL);

    screen = SDL_GetWindowSurface(login_window);
    login_img=loadSurface(login);
    SDL_BlitSurface(login_img, NULL, screen, NULL); // login screen
    SDL_UpdateWindowSurface(login_window);
    SDL_FreeSurface(login_img); // Frigör bild

    while(running){
        while( SDL_PollEvent( &event ) != 0 ) // Check if user is closing the window --> then call quit
          {
            x = event.button.x; // used to know where on x-axis is currently being clicked
            y = event.button.y; // used to know where on y-axis is currently being clicked

             if( event.type == SDL_QUIT ){
                 SDL_DestroyWindow(login_window);  // Dödar fönstret
                 running = false; // Gameloop flag false
             }


             if (event.type ==SDL_MOUSEBUTTONDOWN){
                if(x>520 && x< 520+100 && y>420 && y<420+49){
                    SDL_DestroyWindow(login_window);  // Dödar fönstret
                    running = false; // Gameloop flag false
                }
             }
             if (event.type ==SDL_MOUSEMOTION){
                    SDL_UpdateWindowSurface(login_window);
             }
          }
     }
     running = true;
}
