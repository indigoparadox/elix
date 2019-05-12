
#include "../net.h"

/* This driver relies on a host OS anyway. */
#include <stdio.h>

#ifdef NET_CON_ECHO
#include "../bstrlib.h"
#endif /* NET_CON_ECHO */
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

int net_open_socket() {
   int sockfd_out = 0;
   
   /* Open a raw socket from the host OS. */
   if( -1 == (sockfd_out = socket( AF_PACKET, SOCK_RAW, IPPROTO_RAW)) ) {
      perror( "Unable to open raw socket." );
      return -1;
   }

   return sockfd_out;
}

int net_send( uint8_t* buffer, size_t len ) {
   size_t sent = 0;

#ifdef NET_CON_ECHO
   bstring echo_buffer = NULL;
#endif /* NET_CON_ECHO */

#ifdef NET_CON_ECHO
   echo_buffer = blk2bstr( buffer, len );
   printf( "%s\n", bdata( echo_buffer ) );
#endif /* NET_CON_ECHO */

cleanup:
#ifdef NET_CON_ECHO
   bdestroy( echo_buffer );
#endif /* NET_CON_ECHO */

   return sent;
}

int net_poll( uint8_t* buffer ) {
   size_t rcvd = 0;

   return rcvd;
}

