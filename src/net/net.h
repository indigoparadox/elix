
#ifndef NET_H
#define NET_H

#include <stddef.h>
#include <stdint.h>

#include "ether.h"

#define NET_SOCK void*
#define SOCKET_ID char*

#define RECV_BUFFER_LEN 65535

NET_SOCK net_open_socket( SOCKET_ID if_name );
void net_close_socket( NET_SOCK socket );
int net_send_frame(
   NET_SOCK socket, struct ether_frame* frame, int frame_len );
int net_poll_frame(
   NET_SOCK socket, struct ether_frame* frame, int frame_sz );
void net_print_frame( struct ether_frame* frame, int frame_len );

#endif /* NET_H */

