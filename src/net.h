
#ifndef NET_H
#define NET_H

#include <stddef.h>
#include <stdint.h>
#include "bstrlib.h"

int net_open_socket();
int net_send( bstring buffer );
bstring net_poll();

#endif /* NET_H */

