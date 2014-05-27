/****************************************************************
* Purpose: Function handles the sound for every client          *
*                                                               *
*                                                               *
*****************************************************************/

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>


void playSound(char fileName[], int soundLoop) {
    Mix_Music *music = NULL;
    Mix_Chunk *soundEffect = NULL;
    if(music == NULL) {
        music = Mix_LoadMUS(fileName);
        Mix_PlayMusic(music, soundLoop);
    }
    music = NULL; // set pointer to null
    Mix_FreeMusic(music);

}

void playSoundEffect(char fileName[],int channel) {
     Mix_Music *music = NULL;
     Mix_Chunk *soundEffect = NULL;
     Mix_Volume(2,50);
     soundEffect = Mix_LoadWAV(fileName);
     Mix_PlayChannel( channel,soundEffect,0);
     soundEffect = NULL;
     Mix_FreeChunk(soundEffect);
}
