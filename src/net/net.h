
#ifndef NET_H
#define NET_H

#include "../etypes.h"
#include "ether.h"
#include "../adhd.h"

typedef void* NET_SOCK;
typedef const char* SOCKET_ID;

#define RECV_BUFFER_LEN 65535

#define NET_MID_RECEIVED 2
#define NET_MID_RESPONDED 3

NET_SOCK net_open_socket( const SOCKET_ID if_name );
void net_close_socket( NET_SOCK socket );
int net_send_frame(
   const NET_SOCK socket, struct ether_frame* frame, int frame_len );
int net_poll_frame(
   const NET_SOCK socket, struct ether_frame* frame, int frame_sz );
void net_print_frame( struct ether_frame* frame, int frame_len );
TASK_RETVAL net_respond_task();
void net_init();

#endif /* NET_H */

