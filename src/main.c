
#include <stdint.h>
#include <stddef.h>
#include "bstrlib.h"
#include "net.h"
#include "ether.h"
#include "mem.h"

uint8_t g_dest_mac[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

int main( int argc, char** argv ) {
   int sockfd = 0;
   bstring if_name = NULL;
   bstring send_buffer = NULL;
   uint8_t src_mac[6] = { 0 };
   struct ether_packet* packet = NULL;
   uint16_t type = 0;
   int retval = 0;
   bstring data = NULL;

   if_name = bfromcstr( "eth0" );
   data = bfromcstr( "tcp packet goes here" );

   /* Open a socket. */
   sockfd = net_open_socket( if_name, src_mac );
   if( 0 > sockfd ) {
      do_error( "Unable to open socket" );
      retval = 1;
      goto cleanup;
   }

   /* Create a packet using the MAC from the socket above and flatten it into
    * a buffer to send.
    */
   packet = ether_new_packet( src_mac, g_dest_mac, type, data );
   if( NULL == packet ) {
      do_error( "Unable to create packet" );
      retval = 1;
      goto cleanup;
   }
   send_buffer = blk2bstr( packet, sizeof( struct ether_packet ) );
   net_send( send_buffer );

cleanup:
   mem_free( packet );
   return retval;
}

