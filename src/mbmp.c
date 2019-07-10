
#include "code16.h"

#include "mbmp.h"
#include "mfat.h"

#ifdef DEBUG
#include <assert.h>
#else
#define assert( x )
#endif /* DEBUG */

int32_t mbmp_get_width(
   FILEPTR_T file_ptr, uint8_t dev_idx, uint8_t part_idx
) {
   int32_t out = 0;
   mfat_get_dir_entry_data(
      file_ptr, 0x12, (uint8_t*)&out, 4, dev_idx, part_idx );
   return out;
}

int32_t mbmp_get_height(
   FILEPTR_T file_ptr, uint8_t dev_idx, uint8_t part_idx
) {
   int32_t out = 0;
   mfat_get_dir_entry_data(
      file_ptr, 0x16, (uint8_t*)&out, 4, dev_idx, part_idx );
   return out;
}

uint16_t mbmp_get_bpp(
   FILEPTR_T file_ptr, uint8_t dev_idx, uint8_t part_idx
) {
   int32_t out = 0;
   mfat_get_dir_entry_data(
      file_ptr, 0x1c, (uint8_t*)&out, 2, dev_idx, part_idx );
   return out;
}

uint32_t mbmp_get_colors(
   FILEPTR_T file_ptr, uint8_t dev_idx, uint8_t part_idx
) {
   uint32_t out = 0;
   mfat_get_dir_entry_data(
      file_ptr, 0x1c, (uint8_t*)&out, 4, dev_idx, part_idx );
   return out;
}

uint16_t mbmp_get_type(
   FILEPTR_T file_ptr, uint8_t dev_idx, uint8_t part_idx
) {
   uint16_t out = 0;
   mfat_get_dir_entry_data(
      file_ptr, 0x0, (uint8_t*)&out, 2, dev_idx, part_idx );
   return out;
}

uint32_t mbmp_get_hdr_sz(
   FILEPTR_T file_ptr, uint8_t dev_idx, uint8_t part_idx
) {
   uint32_t out = 0;
   mfat_get_dir_entry_data(
      file_ptr, 0x0e, (uint8_t*)&out, 4, dev_idx, part_idx );
   return out;
}

/*
struct bmp_header {
   uint16_t type;
   uint32_t size;
   uint16_t reserved1;
   uint16_t reserved2;
   uint32_t offset;
} __attribute__( (packed) );

struct bmp_info {
   uint32_t size;
   int32_t width;
   int32_t height;
   uint16_t planes;
   uint16_t bpp;
   uint32_t compression;
   uint32_t img_size;
   int32_t xppm;
   int32_t yppm;
   uint32_t colors;
   uint32_t colors_important;
} __attribute__( (packed) );
*/

FILEPTR_T mbmp_validate(
#ifdef USE_ASTRING
   const struct astring* filename,
#else
   const char* filename, uint8_t filename_len,
#endif /* USE_ASTRING */
   uint8_t dev_idx, uint8_t part_idx
) {
   FILEPTR_T offset = 0;

   offset = mfat_get_root_dir_offset( dev_idx, part_idx );

   offset = mfat_get_dir_entry_offset(
#ifdef USE_ASTRING
      filename->data, filename->len,
#else
      filename, filename_len,
#endif /* USE_ASTRING */
      offset, 0, 0 );

#ifdef MBMP_AGGRESSIVE_VALIDATE
   assert( 0x4d42 == mbmp_get_type( offset, dev_idx, part_idx ) );
   assert( 40 == mbmp_get_hdr_sz( offset, dev_idx, part_idx ) );
   assert( 24 == mbmp_get_bpp( offset, dev_idx, part_idx ) );
   assert( 0 < mbmp_get_height( offset, dev_idx, part_idx ) );
   assert( 0 == ((mbmp_get_bpp( offset, dev_idx, part_idx ) *
      mbmp_get_width( offset, dev_idx, part_idx )) % 4 ) );
#endif /* MBMP_AGGRESSIVE_VALIDATE */

   if(
      0x4d42 != mbmp_get_type( offset, dev_idx, part_idx ) ||
      40 != mbmp_get_hdr_sz( offset, dev_idx, part_idx ) ||
      24 != mbmp_get_bpp( offset, dev_idx, part_idx ) ||
      0 >= mbmp_get_height( offset, dev_idx, part_idx ) ||
      0 < ((mbmp_get_bpp( offset, dev_idx, part_idx ) *
         mbmp_get_width( offset, dev_idx, part_idx )) % 4 )
      /* 0 < mbmp_get_colors( offset, dev_idx, part_idx ) */
   ) {
      return 0;
   }

   return offset;
}

