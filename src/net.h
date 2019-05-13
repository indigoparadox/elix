
#ifndef NET_H
#define NET_H

#include <stddef.h>
#include <stdint.h>
#include "bstrlib.h"

#include "ether.h"

int net_open_socket( bstring if_name, int* if_idx, uint8_t* src_mac );
int net_send_packet(
   int socket, int if_idx, struct ether_packet* pkt, size_t pkt_len );
bstring net_poll();

#endif /* NET_H */

