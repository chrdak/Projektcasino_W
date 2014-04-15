#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL_net.h>

#define SMTP_PORT 25 // SMTP request

#define BUF_SIZE 1024

/* Send a message using the socket */
void send(TCPsocket socket, const char *message);

/* Wait for, and receive, a response on the socket.
   size: size of the response buffer */
void receive(TCPsocket socket, char *response, int size);

int main(int argc, char **argv)
{
    /*
    SDL specific: redirect stdin back to the console.
    These lines should probably be removed in your release code.
    */
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);

    IPaddress server_ip;		/* Server address */
    TCPsocket socket;
    char buffer[BUF_SIZE];

    /* Check input parameters */
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s <mail server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (SDLNet_Init() < 0)
    {
        fprintf(stderr, "SDLNet_Init error: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    /* Resolve the host we are connecting to */
    if (SDLNet_ResolveHost(&server_ip, argv[1], SMTP_PORT) < 0)
    {
        fprintf(stderr, "SDLNet_ResolveHost error: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    /* Open a connection with the IP address provided */
    if (!(socket= SDLNet_TCP_Open(&server_ip)))
    {
        fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }
    printf("SDLNet_TCP_Open ok\n");

    /*
    Let's talk to the mail server;
    receive and send, following the SMTP protocol
    */
    receive(socket, buffer, BUF_SIZE);
    send(socket, "HELO smtp.kth.se\n");
    receive(socket, buffer, BUF_SIZE);
    send(socket, "MAIL FROM: <anderslm@kth.se>\n");
    receive(socket, buffer, BUF_SIZE);
    send(socket, "RCPT TO:<j.anders.lindstrom@gmail.com>\n");
    receive(socket, buffer, BUF_SIZE);
    send(socket, "DATA\n");
    receive(socket, buffer, BUF_SIZE);
    /* Data section */
    send(socket, "From: \"Anders Lindström\" <anderslm@sth.kth.se>\n");
    send(socket, "To: <j.anders.lindstrom@gmail.com>\n");
    send(socket, "Subject: Hello!\n");
    send(socket, "Do you get the message?\n");
    send(socket, "\n.\n"); /* End data section */

    send(socket, "QUIT\n");
    receive(socket, buffer, BUF_SIZE);

    /*
    Don't forget to clean up, i.e. close sockets
    and exit SDLNet.
    */
    SDLNet_TCP_Close(socket);
    SDLNet_Quit();

    return EXIT_SUCCESS;
}


/*
This is where the functions are defined
*/
void send(TCPsocket socket, const char *message)
{
    int len = strlen(message);
    if (SDLNet_TCP_Send(socket, (void *)message, len) < len)
    {
        fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }
}

void receive(TCPsocket socket, char *response, int size)
{
    int result;
    if ((result = SDLNet_TCP_Recv(socket, response, size)) > 0)
    {
        response[result] = '\0';
        printf("%s", response);
    }
}

