
#ifndef NET_H
#define NET_H

#include <stddef.h>
#include <stdint.h>
#include "bstrlib.h"

#include "ether.h"

#define RECV_BUFFER_LEN 65535

int net_open_socket( bstring if_name, int* if_idx, uint8_t* src_mac );
void net_close_socket( int socket );
int net_send_frame(
   int socket, int if_idx, struct ether_frame* pkt, size_t pkt_len );
struct ether_frame* net_poll_frame( int socket, int* frame_len );
void net_print_frame( struct ether_frame* pkt, size_t pkg_len );

#endif /* NET_H */

