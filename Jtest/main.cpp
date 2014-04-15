#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL_net.h>
#include <SDL_opengl.h>

#define MAX_SQUARES 3

#define RED 0
#define BLUE 1
#define YELLOW 2

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCREEN_BPP 32

#define SQUARE_WIDTH 15
#define SQUARE_HEIGHT 15


int init_GL()
{
    //Set clear color
    glClearColor( 0, 0, 0, 0 );
    //Set projection
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1, 1 );
    //Initialize modelview matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    //If there was any errors
    if( glGetError() != GL_NO_ERROR )
    {
        return 0;
    }

    return 1;
}

int init_SDL_Window()
{
    if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
    {
        return 0;
    }

    if( SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_OPENGL ) == NULL )
    {
        return 0;
    }

    if( init_GL() == 0 )
    {
        return 0;
    }

    SDL_WM_SetCaption( "OpenGL Test", NULL );

    return 1;

}


//Client requesting entry.
//Sent via TCP.
struct csm_request_entry
{
  int requested_color;
};

//Client notifying server of a click.
//Sent via TCP
struct csm_click_at
{
  int x, y;
};


//Server granting entry to a user.
//Sent via TCP. (-1 indicates failure.)
struct scm_grant_entry
{
  int success;
};

//Server requesting client to draw a square at x, y
//Sent via UDP.
struct scm_draw_square_at
{
  int x, y;
  int square_color;
};



int main(int argc, char* argv[])
{
	IPaddress ip;		/* Server address */
	TCPsocket sd;		/* Socket descriptor */
	int quit, len;
	char buffer[512];

 	char serverip[20] = "";
	int port;

	int square_color=0;

    struct csm_request_entry csm_request_entry_pdu;
    struct csm_click_at csm_click_at_pdu;
    struct scm_grant_entry scm_grant_entry_pdu;
    struct scm_draw_square_at scm_draw_square_at_pdu;

    freopen("CON","w",stdout);
    freopen("CON","w",stderr);

	if (SDLNet_Init() < 0)
	{
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

    printf("Server ip: ");
    scanf("%s", serverip);

    printf("Port: ");
    scanf("%d", &port);

	if (SDLNet_ResolveHost(&ip, serverip, port) < 0)
	{
		fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	if (!(sd = SDLNet_TCP_Open(&ip)))
	{
		fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	printf("Connection to server acquired.\n");
	printf("Which color do you want for your square: \n");
	printf("1. Red.\n");
	printf("2. Blue.\n");
	printf("3. Yellow.\n");

	while(square_color!=1&&square_color!=2&&square_color!=3)
	  scanf("%d", &square_color);

    csm_request_entry_pdu.requested_color = square_color;

    if (SDLNet_TCP_Send(sd, (void *)&csm_request_entry_pdu,
            sizeof(struct csm_request_entry)) < sizeof(struct csm_request_entry))
    {
        fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if(SDLNet_TCP_Recv(sd, &scm_grant_entry_pdu, sizeof(scm_grant_entry))>0)
    {
        printf("Server says: %d.\n", scm_grant_entry_pdu.success);
    }
    else
    {
        fprintf(stderr, "Connection to server lost.\n");
        exit(EXIT_FAILURE);
    }

    if(scm_grant_entry_pdu.success==0)
    {

        UDPsocket udp_sd;
        IPaddress srvadd;
        UDPpacket *p;

	    if (!(udp_sd = SDLNet_UDP_Open(2001)))
	    {
		    fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		    exit(EXIT_FAILURE);
	    }

	    /* Resolve server name  */
	    if (SDLNet_ResolveHost(&srvadd, argv[1], atoi(argv[2])) == -1)
	    {
		    fprintf(stderr, "SDLNet_ResolveHost(%s %d): %s\n", argv[1], atoi(argv[2]), SDLNet_GetError());
		    exit(EXIT_FAILURE);
	    }

    	/* Allocate memory for the packet */

	    if (!(p = SDLNet_AllocPacket(512)))
	    {
		    fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		    exit(EXIT_FAILURE);
	    }

        printf("Starting window...\n");

        init_GL();
        init_SDL_Window();

        glClear( GL_COLOR_BUFFER_BIT );

        quit=0;
        while( quit == 0 )
        {
            SDL_Event event;

            //While there are events to handle
            while( SDL_PollEvent( &event ) )
            {
            //Handle events:

                switch(event.type)
                {
                    case SDL_MOUSEBUTTONDOWN:
                        switch(event.button.button)
                        {
                            case SDL_BUTTON_LEFT: // Change direction of square
                                csm_click_at_pdu.x = event.button.x;
                                csm_click_at_pdu.y = event.button.y;
                                if(SDLNet_TCP_Send(sd,&csm_click_at_pdu,
                                                   sizeof(struct csm_click_at))<sizeof(struct csm_click_at))
                                {
                                    fprintf(stderr,"SDL_Net_TCP_Send: %s\n", SDLNet_GetError());
                                    exit(EXIT_FAILURE);
                                }
                                break;
                        }
                    break;
                }

                if( event.type == SDL_QUIT )
                {
                    quit = 1;
                }
            }

            //glClear( GL_COLOR_BUFFER_BIT );

            if(SDLNet_UDP_Recv(udp_sd,p))
            {
                //char str[12];
                struct scm_draw_square_at * square_at;
                int sax, say;

                //square_at = (struct scm_draw_square_at*)(p->data);

                sscanf((char *)(p->data),"%d %d", &sax, &say);

                //freopen("CON","w","stdout");
                printf("> %s\n", (char *)(p->data));

                //glTranslatef( square_at->x, square_at->y, 0 );
                //glClear( GL_COLOR_BUFFER_BIT );
                glLoadIdentity();
                glTranslatef( sax, say, 0 );

                glBegin( GL_QUADS );
                    //Set color to white
                    glColor4f( 1, 1, 1, 1.0 );

                    //Draw square
                    glVertex3f( 0,            0,             0 );
                    glVertex3f( SQUARE_WIDTH, 0,             0 );
                    glVertex3f( SQUARE_WIDTH, SQUARE_HEIGHT, 0 );
                    glVertex3f( 0,            SQUARE_HEIGHT, 0 );
                glEnd();
            }

            glLoadIdentity();
            glTranslatef( 200, 200, 0 );

            glBegin( GL_QUADS );
                //Set color to white
                glColor4f( 1, 1, 1, 1.0 );

                //Draw square
                glVertex3f( 0,            0,             0 );
                glVertex3f( SQUARE_WIDTH, 0,             0 );
                glVertex3f( SQUARE_WIDTH, SQUARE_HEIGHT, 0 );
                glVertex3f( 0,            SQUARE_HEIGHT, 0 );
            glEnd();

            glFlush();

            //Update screen
            SDL_GL_SwapBuffers();

            SDL_Delay(100);

        }

        SDL_Quit();
    }
    else
    {
        printf("Server full.\n");
    }

	SDLNet_TCP_Close(sd);
	SDLNet_Quit();

	return EXIT_SUCCESS;
}

