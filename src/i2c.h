
#ifndef I2C_H
#define I2C_H

/*! \file i2c.h */

#include "etypes.h"

void i2c_init();
void i2c_start( uint8_t addr );
void i2c_stop();
void i2c_send_byte( uint8_t byte );

#include "globals.h"

#endif /* I2C_H */

