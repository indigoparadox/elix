
#include "../net/net.h"

NET_SOCK net_open_socket( SOCKET_ID if_name ) {
   return NULL;
}

void net_close_socket( NET_SOCK socket ) {
}

int net_send_frame(
   NET_SOCK socket, struct ether_frame* frame, int frame_len
) {
   return -1;
}

int net_poll_frame(
   NET_SOCK socket, struct ether_frame* frame, int frame_sz
) {
   return -1;
}

