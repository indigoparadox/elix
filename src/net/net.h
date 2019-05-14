
#ifndef NET_H
#define NET_H

#include <stddef.h>
#include <stdint.h>
#include "../bstrlib.h"

#include "ether.h"

#define NET_SOCK void*

#define RECV_BUFFER_LEN 65535

NET_SOCK net_open_socket( bstring if_name );
void net_close_socket( NET_SOCK socket );
int net_send_frame(
   NET_SOCK socket, struct ether_frame* frame, size_t frame_len );
struct ether_frame* net_poll_frame( NET_SOCK socket, int* frame_len );
void net_print_frame( struct ether_frame* frame, size_t frame_len );

#endif /* NET_H */

