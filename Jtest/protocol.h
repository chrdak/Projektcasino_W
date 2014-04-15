
#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAX_SQUARES 3

#define RED 0
#define BLUE 1
#define YELLOW 2


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


#endif

