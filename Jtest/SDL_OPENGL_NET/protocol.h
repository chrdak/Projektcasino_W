
#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAX_SQUARES 3

#define RED 1
#define BLUE 2
#define YELLOW 3


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
  float x, y;
};


// Server granting entry to a user.
// Sent via TCP. (-1 indicates failure.)
struct scm_grant_entry
{
  int success;
};

// Server requesting client to draw a square at x, y
// Previous coordinates sent for the clients to remove
// old image during animation
// Sent via UDP.
struct scm_draw_square_at
{
  float x, y;
  float prevx, prevy;
  int square_color;
};


#endif

