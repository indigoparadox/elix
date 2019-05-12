
#include "../net.h"

/* This driver relies on a host OS anyway. */
#include <stdio.h>

#include "../mem.h"
#ifdef NET_CON_ECHO
#include "../bstrlib.h"
#endif /* NET_CON_ECHO */
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/ether.h>
#include <unistd.h>

/* Open a socket and get the MAC address the socket will send from. */
int net_open_socket( bstring ifname, uint8_t mac_addr[6] ) {
   int sockfd_out = 0;
   struct ifreq if_idx;
   struct ifreq if_mac;
   char* if_name_c = NULL;
   size_t if_name_len = 0;
   
   /* Open a raw socket from the host OS. */
   if( -1 == (sockfd_out = socket( AF_PACKET, SOCK_RAW, IPPROTO_RAW)) ) {
      do_error( "Unable to open raw socket.\n" );
      goto cleanup;
   }

   /* Prevent warnings later. */
   if_name_c = bdata( ifname );
   if_name_len = blength( ifname );

   /* Get the sending interface index. */
   memset( &if_idx, 0, sizeof( struct ifreq ) );
   strncpy( if_idx.ifr_name, if_name_c, if_name_len );
   if( 0 > ioctl( sockfd_out, SIOCGIFINDEX, &if_idx ) ) {
      do_error( "SIOCGIFINDEX\n" );
      close( sockfd_out );
      sockfd_out = -1;
      goto cleanup;
   }

   /* Get the sending interface MAC to return. */
   memset( &if_mac, 0, sizeof( struct ifreq ) );
   strncpy( if_mac.ifr_name, if_name_c, if_name_len );
   if( 0 > ioctl( sockfd_out, SIOCGIFHWADDR, &if_mac ) ) {
      do_error( "SIOCGIFHWADDR\n" );
      close( sockfd_out );
      sockfd_out = -1;
      goto cleanup;
   }

cleanup:
   return sockfd_out;
}

int net_send( bstring buffer ) {
   size_t sent = 0;

#ifdef NET_CON_ECHO
   printf( "%s\n", bdata( buffer ) );
#endif /* NET_CON_ECHO */

   return sent;
}

bstring net_poll() {
   bstring rcvd = NULL;

   return rcvd;
}

