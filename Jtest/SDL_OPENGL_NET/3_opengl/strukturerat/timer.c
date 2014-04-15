#ifndef _C_TIMER
#define _C_TIMER

#include <SDL/SDL.h>

#include "timer.h"


void timer_init (struct timer_t2 * timer)
{
  timer->startTicks=0;
  timer->pausedTicks=0;
  timer->paused=0;
  timer->started=0;
};

void timer_start(struct timer_t2 * timer)
{
    //Start the timer
    timer->started = 1;
    //Unpause the timer
    timer->paused = 0;
    //Get the current clock time
    timer->startTicks = SDL_GetTicks();
};
 
void timer_stop(struct timer_t2 * timer)
{
    //Stop the timer
    timer->started = 0;
    //Unpause the timer
    timer->paused = 0;
};
 
void timer_pause(struct timer_t2 * timer)
{
    //If the timer is running and isn't already paused 
    if( ( timer->started == 1 ) && ( timer->paused == 0 ) )
    {
        //Pause the timer
        timer->paused = 1;
        //Calculate the paused ticks
        timer->pausedTicks = SDL_GetTicks() - timer->startTicks;
    }
};
 
void timer_unpause(struct timer_t2 *timer)
{
    //If the timer is paused
    if( timer->paused == 1 )
    {
        //Unpause the timer
        timer->paused = 0;
        //Reset the starting ticks
        timer->startTicks = SDL_GetTicks() - timer->pausedTicks;
        //Reset the paused ticks
        timer->pausedTicks = 0;
    }
};
 
int timer_get_ticks(struct timer_t2 *timer)
{
    //If the timer is running
    if( timer->started == 1 )
    {
        //If the timer is paused
        if( timer->paused == 1 )
        {
            //Return the number of ticks when the timer was paused
            return timer->pausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            return SDL_GetTicks() - timer->startTicks;
        }
    }
 
    //If the timer isn't running
    return 0;
 
};
 
int timer_is_started(struct timer_t2 *timer)
{
    return timer->started;
};
 
int timer_is_paused(struct timer_t2 *timer)
{
    return timer->paused;
};
#endif
