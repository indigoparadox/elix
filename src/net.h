
#ifndef NET_H
#define NET_H

#include <stddef.h>
#include <stdint.h>

int net_open_socket();
int net_send( uint8_t* buffer, size_t len );
int net_poll( uint8_t* buffer );

#endif /* NET_H */

