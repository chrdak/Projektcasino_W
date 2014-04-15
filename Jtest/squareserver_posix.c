#if 0
#!/bin/sh
gcc -Wall squareserver_posix.c -o ss_px -lpthread -lm
exit
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <math.h>
#include "protocol.h"

#define BACKLOG 10

struct udp_info {
  int udpsockfd;
  struct addrinfo hints, *servinfo, *p;
} very_ugly_array[MAX_SQUARES];

void init_the_very_ugly_array (struct udp_info * the_hideous_array, int n)
{
  int i;
  for(i=0;i<n;i++)
    the_hideous_array[i].udpsockfd = -1;
}

struct square_t
{
  pthread_t thread_id;        // Thread ID of controlling thread
  int square_color;           // Color of the square
  float square_x;             // x-coordinate
  float square_y;             // y-coordinate
  float speed_x;              // sx and sy are for having the
  float speed_y;              // sqaure move in a direction
  float dest_x;               // The destination coordinates
  float dest_y;               // of the square
  int csd;                    // Client descriptor
  char remote_ip_str[16];     // Client IP
  char server_port[10];       // The port the server runs on
  int free;                   // Flag to indicate if this slot is free
  int myindex;                // Index to tell who I am
};

void db_print(int n, struct square_t square)
{
  printf("======DEBUG PRINTOUT========\nRecord number: %d.\n", n);
  switch(square.square_color) { case RED: printf("Color: RED.\n"); break;
    case BLUE: printf("Color: BLUE.\n"); break; case YELLOW: printf("Color: YELLOW.\n"); break;
    default: printf("Color: None.\n");}
  printf("Client descr: %d.\n", square.csd );
  printf("Remote IP: %s.\n", square.remote_ip_str);
  printf("Server port: %s.\n", square.server_port);
  printf("Free flag: %d.\n=====================\n", square.free);
}

/*
  Inits the array of squares
*/
void init_squares ( struct square_t square[], int n )
{
  int i;
  for(i=0;i<n;i++)
  {
    square[i].thread_id = 0;
    square[i].square_color=-1;
    square[i].square_x = 0.0; square[i].square_y = 0.0;
    square[i].speed_x = 0.0; square[i].speed_y = 0.0;
    square[i].dest_x = 0.0; square[i].dest_y = 0.0;
    square[i].csd = 0;
    square[i].remote_ip_str[0]=0;
    square[i].server_port[0]=0;
    square[i].free = 1;
    square[i].myindex = -1;
  }

}

/*
  Goes through the array of structs describing squares
  and returns the index of an unused position.
  Returns -1 if none is available.
*/
int find_free_slot(struct square_t square[], int n)
{
  int i;
  for(i=0;i<n;i++)
    if(square[i].free==1)return i;
  return -1;
}

void prepare_udp_stuff_in_very_ugly_array(int index, char * remote_ip, char * port)
{
  int rv;

  memset(&(very_ugly_array[index].hints), 0, sizeof very_ugly_array[index].hints);
  very_ugly_array[index].hints.ai_family = AF_UNSPEC;
  very_ugly_array[index].hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo(  remote_ip , port, &(very_ugly_array[index].hints), &(very_ugly_array[index].servinfo))) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(EXIT_FAILURE);
  }

  // loop through all the results and make a socket
  for(very_ugly_array[index].p = very_ugly_array[index].servinfo;
      very_ugly_array[index].p != NULL;
      very_ugly_array[index].p = very_ugly_array[index].p->ai_next) {
    if ((very_ugly_array[index].udpsockfd = socket(very_ugly_array[index].p->ai_family, very_ugly_array[index].p->ai_socktype,
    very_ugly_array[index].p->ai_protocol)) == -1) {
      perror("Preparing UDP: socket");
      continue;
    }
    break;
  }

  if (very_ugly_array[index].p == NULL) {
    fprintf(stderr, "Preparing UDP: failed to bind socket\n");
    exit(EXIT_FAILURE);
  }

}


void* move_square (void *parameters)
{
  char buffer[512];
//  int udpsockfd;
//  struct addrinfo hints, *servinfo, *p;
//  int rv;

  int i;

  struct scm_draw_square_at scm_draw_square_at_pdu;

  // Obtain parameters
  struct square_t * square = (struct square_t*)parameters;

  // Prepare UDP-stuff in very_ugle_array:

  prepare_udp_stuff_in_very_ugly_array( square->myindex, square->remote_ip_str, square->server_port);

/*  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo(  square->remote_ip_str , square->server_port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(EXIT_FAILURE);
  }

  // loop through all the results and make a socket
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((udpsockfd = socket(p->ai_family, p->ai_socktype,
    p->ai_protocol)) == -1) {
      perror("Preparing UDP: socket");
      continue;
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "Preparing UDP: failed to bind socket\n");
    exit(EXIT_FAILURE);
  }

  //Set up udpsocket so others can use it.
  very_ugly_array[square->myindex] = udpsockfd;

  //UDP prepared

*/


  scm_draw_square_at_pdu.x = square->square_x;
  scm_draw_square_at_pdu.y = square->square_y;
  scm_draw_square_at_pdu.prevx = 0.0;
  scm_draw_square_at_pdu.prevy = 0.0;
  scm_draw_square_at_pdu.square_color = square->square_color;

  sprintf((char *)(buffer),"%f %f %f %f %d",
    scm_draw_square_at_pdu.x, scm_draw_square_at_pdu.y,
    scm_draw_square_at_pdu.prevx, scm_draw_square_at_pdu.prevy, scm_draw_square_at_pdu.square_color);

  // Draw this square so it can be seen on all screens:
  for(i=0;i<MAX_SQUARES;i++) if( (very_ugly_array[i].udpsockfd != -1) /*&& (i != square->myindex)*/ )
    if (sendto(very_ugly_array[i].udpsockfd, buffer, strlen(buffer)+1, 0,
               very_ugly_array[i].p->ai_addr, very_ugly_array[i].p->ai_addrlen) == -1) {
      perror("talker: sendto");
      exit(1);
    }

/*  if (sendto(udpsockfd, buffer, strlen(buffer)+1, 0, p->ai_addr, p->ai_addrlen) == -1) {
    perror("talker: sendto");
    exit(1);
  }
*/


  for(;;) //Do an infinite loop and move the square.
  {
    float vx, vy, lenv;
    int i;

    if ( (square->square_x-square->dest_x)*(square->square_x-square->dest_x)+
         (square->square_y-square->dest_y)*(square->square_y-square->dest_y)>
          5.0*5.0*(square->speed_x*square->speed_x + square->speed_y*square->speed_y)   )
    {
      vx = square->dest_x - square->square_x;
      vy = square->dest_y - square->square_y;
      lenv = sqrt(vx*vx+vy*vy);

      square->speed_x = 5.0*vx / lenv;
      square->speed_y = 5.0*vy / lenv;

      scm_draw_square_at_pdu.prevx = square->square_x;
      scm_draw_square_at_pdu.prevy = square->square_y;

      square->square_x += square->speed_x;
      square->square_y += square->speed_y;

      scm_draw_square_at_pdu.x = square->square_x;
      scm_draw_square_at_pdu.y = square->square_y;

      scm_draw_square_at_pdu.square_color = square->square_color;

      sprintf((char *)(buffer),"%f %f %f %f %d",
        scm_draw_square_at_pdu.x, scm_draw_square_at_pdu.y,
        scm_draw_square_at_pdu.prevx, scm_draw_square_at_pdu.prevy,
        scm_draw_square_at_pdu.square_color);

      //if (sendto(udpsockfd, buffer, strlen(buffer)+1, 0, p->ai_addr, p->ai_addrlen) == -1) {
      //  perror("talker: sendto");
      //  exit(1);
      //}

      // Send commands to all other clients too:
      for(i=0;i<MAX_SQUARES;i++) if( very_ugly_array[i].udpsockfd != -1 )
        if (sendto(very_ugly_array[i].udpsockfd, buffer, strlen(buffer)+1, 0,
                   very_ugly_array[i].p->ai_addr, very_ugly_array[i].p->ai_addrlen) == -1) {
          perror("talker: sendto");
          exit(1);
        }
    }

    usleep(50000);

  }

  /* Remove UDP-related stuff
  freeaddrinfo(servinfo);
  very_ugly_array[square->myindex] = -1;
  WARNING: This will probably never happen since this thread is canceled. */

}

/*
  Thread function that runs when a client is being served.
*/
void* do_square (void * parameters)
{
  char buffer[512];
  pthread_t move_thread_id; // The thread that moves the square

  // Protcol data units to use in dialogue between server and client.
  struct csm_request_entry csm_request_entry_pdu;
  struct csm_click_at csm_click_at_pdu;
  struct scm_grant_entry scm_grant_entry_pdu;

  // Obtain parameters
  struct square_t * square = (struct square_t*)parameters;

  //We must always begin by receiving something from the client.
  if ( ( recv(square->csd, buffer, sizeof(buffer), 0 )) <= 0)
  {
    perror("recv");
    exit(EXIT_FAILURE);
  }

  sscanf(buffer,"%d",&(csm_request_entry_pdu.requested_color));

  // Print out contents of protocol data unit when client is requesting entry
  printf("Client says: %d\n", csm_request_entry_pdu.requested_color);
  square->square_color = csm_request_entry_pdu.requested_color;

  // Grant entry to client
  scm_grant_entry_pdu.success = 0;
  sprintf(buffer, "%d", scm_grant_entry_pdu.success);
  if( send (square->csd,buffer,strlen(buffer)+1,0) == -1 )
  {
    perror("send");
    exit(EXIT_FAILURE);
  }

  // Give the square a random starting point and start the move_square thread:
  square->dest_x = square->square_x = (float)(rand()%600+100);
  square->dest_y = square->square_y = (float)(rand()%400+100);
  square->speed_x = square->speed_y = 0.0;

  pthread_create(&move_thread_id, NULL, move_square, parameters);

  // Main loop that runs when a client is being served
  while(recv(square->csd,buffer,sizeof(buffer),0)>0)
  {
    sscanf(buffer,"%f %f", &(csm_click_at_pdu.x), &(csm_click_at_pdu.y));

    printf("%.2f %.2f\n", csm_click_at_pdu.x, csm_click_at_pdu.y);

    //Set new destination:
    square->dest_x = csm_click_at_pdu.x;
    square->dest_y = csm_click_at_pdu.y;

    //scm_draw_square_at_pdu.x = csm_click_at_pdu.x;
    //scm_draw_square_at_pdu.y = csm_click_at_pdu.y;

/*
    for(i=0;i<10;i++)
    {
      rx = rand()%10; ry = rand()%10;
      sprintf((char *)(buffer),"%d %d %d%c",
               scm_draw_square_at_pdu.x+rx, scm_draw_square_at_pdu.y+ry, square->square_color, '\0');

      if (sendto(udpsockfd, buffer, strlen(buffer), 0, p->ai_addr, p->ai_addrlen) == -1) {
        perror("talker: sendto");
        exit(1);
      }

      usleep(50000);

    }

*/
  }

  pthread_cancel(move_thread_id);

  /* Close the client socket */
  close(square->csd);

  /* Indicate to find_free_slot that we are done */
  square->free = 1;

  return 0;
}

void * db_thread (void * parameters)
{
  struct square_t *square = (struct square_t *)parameters;
  for(;;)
  {
    int i;
    for(i=0;i<MAX_SQUARES;i++)
      db_print(i,square[i]);
    sleep(5);
  }
}

int main(int argc, char **argv)
{
  pthread_t db_thread_id;

  int sockfd, newfd; /* Socket descriptor and new descriptor */
  struct sockaddr_in my_addr; // my address information
  struct sockaddr_in their_addr; // connector's address information
  socklen_t sin_size;
  int yes=1, quit;

  struct square_t square[MAX_SQUARES]; // Array that holds runtime data for squares
  init_squares(square,MAX_SQUARES);    // Init this array

  init_the_very_ugly_array (very_ugly_array, MAX_SQUARES);

  if(argc!=2) // Some error handling
  {
    fprintf(stderr,"usage: %s port", argv[0]);
    exit(EXIT_FAILURE);
  }

  if((sockfd = socket(AF_INET,SOCK_STREAM,0))==-1) // Create an internet TCP-socket
  {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1)
  {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  my_addr.sin_family = AF_INET; // host byte order
  my_addr.sin_port = htons( atoi(argv[1]) ); // short, network byte order
  my_addr.sin_addr.s_addr = INADDR_ANY; //Automatically fill in my IP
  memset(my_addr.sin_zero,'\0',sizeof(my_addr.sin_zero));

  if(bind(sockfd,(struct sockaddr*)&my_addr, sizeof(my_addr))==-1)
  {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  if(listen(sockfd,BACKLOG)==-1)
  {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  pthread_create(&db_thread_id,NULL,db_thread,square);

  /* Main loop waiting for connections */
  quit = 0;
  while (!quit)
  {

    int newIndex=-1;

    /* This check the sd if there is a pending connection.
    * If there is one, accept that, and open a new socket for communicating */
    sin_size = sizeof(their_addr);
    if ((newfd = accept(sockfd,(struct sockaddr*)&their_addr,&sin_size))==-1)
    {
      perror("accept");
      continue;         // Goto loop beginning immediately. This accept failed.
    }

    newIndex = find_free_slot(square,MAX_SQUARES);
    printf("New index: %d.\n", newIndex);
    if(newIndex==-1)
    {
      close(newfd); // Close the socket descriptor to indicate to client that
      continue;     // we have no more space for it. Then goto beginning of loop.
    }

    square[newIndex].free=0;             // Indicate that this slot is no longer free.
    square[newIndex].csd = newfd;        // Note the desciptor to communicate via.
    square[newIndex].myindex = newIndex; // Note which index the thread has

    /* Now we can communicate with the client using csd socket (from accept)
     * sd will remain opened waiting other connections */

    /* Get the remote address */
    printf("Got connection from %s.\n",inet_ntoa(their_addr.sin_addr));
    strcpy(square[newIndex].remote_ip_str, inet_ntoa(their_addr.sin_addr));

    // Set the server port in square struct
    sprintf(square[newIndex].server_port,"%d",atoi(argv[1]) );


    // Serve the client in a separate thread. Main program will accept other clients.
    pthread_create(&(square[newIndex].thread_id), NULL, do_square, &(square[newIndex]));

  }

  return EXIT_SUCCESS;

}

