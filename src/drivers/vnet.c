
#include "../net.h"

/* This driver relies on a host OS anyway. */
#include <stdio.h>

#include "../ether.h"
#include "../mem.h"
#ifdef NET_CON_ECHO
#include "../bstrlib.h"
#endif /* NET_CON_ECHO */
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
//#include <netinet/ether.h>
#include <unistd.h>

/* Open a socket and get the MAC address the socket will send from. */
int net_open_socket( bstring ifname, int* if_idx, uint8_t mac_addr[6] ) {
   int sockfd_out = 0;
   struct ifreq if_idx_req;
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
   memset( &if_idx_req, 0, sizeof( struct ifreq ) );
   strncpy( if_idx_req.ifr_name, if_name_c, if_name_len );
   if( 0 > ioctl( sockfd_out, SIOCGIFINDEX, &if_idx_req ) ) {
      do_error( "SIOCGIFINDEX\n" );
      close( sockfd_out );
      sockfd_out = -1;
      goto cleanup;
   }
   *if_idx = if_idx_req.ifr_ifindex;

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

int net_send_packet( 
   int socket, int if_idx, struct ether_packet* pkt, size_t pkt_len
) {
   size_t sent = 0;
   struct sockaddr_ll socket_address = { 0 };
#ifdef NET_CON_ECHO
   int i = 0;
   bstring buffer = NULL;
#endif /* NET_CON_ECHO */

#ifdef NET_CON_ECHO
   buffer = blk2bstr( pkt, pkt_len );
   for( i = 0 ; blength( buffer ) > i ; i++ ) {
      if( '\0' == bchar( buffer, i ) ) {
         printf( "%02X", (unsigned int)(unsigned char)bchar( buffer, i ) );
      } else {
         printf( "%c", bchar( buffer, i ) );
      }
   }
   printf( "\n" );
#endif /* NET_CON_ECHO */

   socket_address.sll_halen = ETHER_ADDRLEN;
   socket_address.sll_ifindex = if_idx;
   memcpy( socket_address.sll_addr, pkt->header.src_mac, ETHER_ADDRLEN );

   if( 0 > sendto( socket, bdata( buffer ), blength( buffer ), 0,
      (struct sockaddr*)&socket_address, sizeof( struct sockaddr_ll ) )
   ) {
      perror( "Unable to send packet" );
   }

   return sent;
}

bstring net_poll() {
   bstring rcvd = NULL;

   return rcvd;
}

