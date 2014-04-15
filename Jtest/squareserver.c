#if 0
#!/bin/sh
gcc -Wall `sdl-config --cflags` squareserver.c -o ss `sdl-config --libs` -lSDL_net 
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL_thread.h>
#include "SDL_net.h"
#include "protocol.h"

void init_thread_pointers ( SDL_Thread * client_thread[MAX_SQUARES] )
{
  int i;
  for(i=0;i<MAX_SQUARES;i++)client_thread[i]=NULL;
}

int find_free_slot(SDL_Thread * client_thread[MAX_SQUARES])
{
  int i;
  for(i=0;i<MAX_SQUARES;i++)
    if(client_thread[i]==NULL)return i;
  return -1;
}

int main(int argc, char **argv)
{
  TCPsocket sd, csd; /* Socket descriptor, Client socket descriptor */
  IPaddress ip, *remoteIP;
  char remote_ip_str[16]="";
  int quit, quit2;
  char buffer[512];

  srand(time(0));

  UDPsocket udp_sd;
  UDPpacket *p;

//  struct csm_request_entry csm_request_entry_pdu;
  struct csm_click_at csm_click_at_pdu;
  struct scm_grant_entry scm_grant_entry_pdu;
  struct scm_draw_square_at scm_draw_square_at_pdu;

  SDL_Thread * client_thread[MAX_SQUARES]; 

  init_thread_pointers(client_thread);
 
  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Resolving the host using NULL make network interface to listen */
  if (SDLNet_ResolveHost(&ip, NULL, 2000) < 0)
  {
    fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Open a connection with the IP provided (listen on the host's port) */
  if (!(sd = SDLNet_TCP_Open(&ip)))
  {
    fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  if(!(p=SDLNet_AllocPacket(512)));
  if(!(udp_sd=SDLNet_UDP_Open(0)));

  /* Wait for a connection, send data and term */
  quit = 0;
  while (!quit)
  {
    /* This check the sd if there is a pending connection.
    * If there is one, accept that, and open a new socket for communicating */
    if ((csd = SDLNet_TCP_Accept(sd)))
    {

      /* Now we can communicate with the client using csd socket
       * sd will remain opened waiting other connections */


      /* Get the remote address */
      if ((remoteIP = SDLNet_TCP_GetPeerAddress(csd)))
      {
        Uint32 remote_ip;
        unsigned char ch0, ch1, ch2, ch3;
        /* Print the address, converting in the host format */
        printf("Host connected: %x %d\n", SDLNet_Read32(&remoteIP->host), SDLNet_Read16(&remoteIP->port));
        remote_ip = SDLNet_Read32(&remoteIP->host);
        ch0 = remote_ip>>24;
        ch1 = (remote_ip & 0x00ffffff)>>16;
        ch2 = (remote_ip & 0x0000ffff)>>8;
        ch3 = remote_ip & 0x000000ff;

        sprintf(remote_ip_str,"%d.%d.%d.%d", (int)ch0, (int)ch1, (int)ch2, (int)ch3);

        printf("Remote host IP in string form: %s\n", remote_ip_str);

      }
      else
        fprintf(stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
 
      quit2 = 0;
      while (!quit2)
      {
        int allocated_index;
        
        if (SDLNet_TCP_Recv(csd, buffer, 512) > 0)
        {
        }
        else
        {
          fprintf(stderr, ">SDLNet_TCP_Recv: %s", SDLNet_GetError());
          exit(EXIT_FAILURE);
        }


        printf("Client say: %d\n", 
                   ((struct csm_request_entry*)buffer)->requested_color);
          
        allocated_index = find_free_slot(client_thread);
        if(allocated_index==-1)
        {
          scm_grant_entry_pdu.success = -1;
          if(SDLNet_TCP_Send(csd,(void*)&scm_grant_entry_pdu,
            sizeof(struct scm_grant_entry))<sizeof(struct scm_grant_entry))
          {
            fprintf(stderr,"SDLNet_TCP_Send: %s\n", SDLNet_GetError());
            exit(EXIT_FAILURE);
          }
        }
        else
        {
          scm_grant_entry_pdu.success = 0;
          if(SDLNet_TCP_Send(csd,(void*)&scm_grant_entry_pdu,
            sizeof(struct scm_grant_entry))<sizeof(struct scm_grant_entry))
          {
            fprintf(stderr,"SDLNet_TCP_Send: %s\n", SDLNet_GetError());
            exit(EXIT_FAILURE);
          }
        }


        //The stuff below should go into a client-specific thread:
        { 
          IPaddress client_address;
          if(SDLNet_ResolveHost(&client_address, remote_ip_str, 2001) == -1)
          {
            fprintf(stderr, "Couldn't resolve client... %s\n", SDLNet_GetError());
            exit(EXIT_FAILURE);
          }


          while(SDLNet_TCP_Recv(csd,&csm_click_at_pdu,
                   sizeof(struct csm_click_at))>0)
          {
            int i; int rx, ry;

            printf("%d %d\n", csm_click_at_pdu.x, csm_click_at_pdu.y);

            scm_draw_square_at_pdu.x = csm_click_at_pdu.x;
            scm_draw_square_at_pdu.y = csm_click_at_pdu.y;

            //for(i=0;i<sizeof(struct scm_draw_square_at);i++)
            //  p->data[i]=0;

            p->address.host = client_address.host;
            p->address.port = client_address.port;
            p->len=12;

            for(i=0;i<10;i++)
            {
              
              rx = rand()%10; rand()%10 + rand()%10 - 15;
              ry = rand()%10; rand()%10 + rand()%10 - 15;

              sprintf((char *)(p->data),"%d %d %d", csm_click_at_pdu.x+rx, csm_click_at_pdu.y+ry, 1);
              SDLNet_UDP_Send(udp_sd,-1,p); 
              SDL_Delay(50);
            }


          }

        }

      }
 
      /* Close the client socket */
      SDLNet_TCP_Close(csd);

    }
  }
 
  SDLNet_TCP_Close(sd);
  SDLNet_Quit();
 
  return EXIT_SUCCESS;

} 



