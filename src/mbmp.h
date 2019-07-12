
#ifndef MBMP_H
#define MBMP_H

#include <stdint.h>

#include <mfat.h>

#ifdef USE_ASTRING
#include "alpha.h"
#endif /* USE_ASTRING */

int32_t mbmp_get_width(
   FILEPTR_T file_ptr, uint8_t dev_idx, uint8_t part_idx );
int32_t mbmp_get_height(
   FILEPTR_T file_ptr, uint8_t dev_idx, uint8_t part_idx );
uint16_t mbmp_get_bpp(
   FILEPTR_T file_ptr, uint8_t dev_idx, uint8_t part_idx );
uint16_t mbmp_get_type(
   FILEPTR_T file_ptr, uint8_t dev_idx, uint8_t part_idx );
uint32_t mbmp_get_hdr_sz(
   FILEPTR_T file_ptr, uint8_t dev_idx, uint8_t part_idx );
FILEPTR_T mbmp_validate(
#ifdef USE_ASTRING
   const struct astring* filename,
#else
   const char* filename, uint8_t filename_len,
#endif /* USE_ASTRING */
   uint8_t dev_idx, uint8_t part_idx );

#endif /* MBMP_H */

