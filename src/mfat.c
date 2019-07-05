
#ifdef USE_DISK

#include "mfat.h"
#include "disk.h"

#ifdef ALPHA_PRESENT
#include "alpha.h"
#endif /* ALPHA_PRESENT */

#ifdef DEBUG
#include <assert.h>
#else
#define assert( x )
#endif /* DEBUG */

#include <stdbool.h>

#define MFAT_OFFSET_FAT 512
#define MFAT_DIR_ENTRY_SZ 32

static uint8_t mfat_get_fat_count( uint8_t dev_idx, uint8_t part_idx ) {
   return disk_get_byte( dev_idx, part_idx, 16 );
}

static uint16_t mfat_get_bytes_per_sector( uint8_t dev_idx, uint8_t part_idx ) {
   uint16_t out = 0;
   out |= disk_get_byte( dev_idx, part_idx, 12 );
   out <<= 8;
   out |= disk_get_byte( dev_idx, part_idx, 11 );
   return out;
}

static uint8_t mfat_get_sectors_per_cluster(
   uint8_t dev_idx, uint8_t part_idx
) {
   return disk_get_byte( dev_idx, part_idx, 13 );
}

static uint16_t mfat_get_sectors_per_fat( uint8_t dev_idx, uint8_t part_idx ) {
   uint16_t out = 0;
   out |= disk_get_byte( dev_idx, part_idx, 23 );
   out <<= 8;
   out |= disk_get_byte( dev_idx, part_idx, 22 );
   return out;
}

static uint16_t mfat_get_root_dir_entries_count(
   uint8_t dev_idx, uint8_t part_idx
) {
   uint16_t out = 0;
   out |= disk_get_byte( dev_idx, part_idx, 18 );
   out <<= 8;
   out |= disk_get_byte( dev_idx, part_idx, 17 );
   return out;
}

#if 0
static uint16_t mfat_get_sectors_per_track(   uint8_t dev_idx, uint8_t part_idx ) {
   uint16_t out = 0;
   out |= disk_get_byte( dev_idx, part_idx, 25 );
   out <<= 8;
   out |= disk_get_byte( dev_idx, part_idx, 24 );
   return out;
}

static uint32_t mfat_get_sectors_total(       uint8_t dev_idx, uint8_t part_idx ) {
   uint32_t out = 0;
   out |= disk_get_byte( dev_idx, part_idx, 35 );
   out <<= 8;
   out |= disk_get_byte( dev_idx, part_idx, 34 );
   out <<= 8;
   out |= disk_get_byte( dev_idx, part_idx, 33 );
   out <<= 8;
   out |= disk_get_byte( dev_idx, part_idx, 32 );
   return out;
}
#endif

static uint16_t mfat_get_root_dir_offset( uint8_t dev_idx, uint8_t part_idx ) {
   uint16_t dir_offset = 0;

   /* The root starts directly after the EBP and FATs. */
   dir_offset += MFAT_OFFSET_FAT;
   dir_offset += (mfat_get_bytes_per_sector( dev_idx, part_idx ) *
      mfat_get_sectors_per_fat( dev_idx, part_idx ) *
      mfat_get_fat_count( dev_idx, part_idx ));

   return dir_offset;
}

static uint16_t mfat_get_data_area_offset( uint8_t dev_idx, uint8_t part_idx ) {
   uint16_t dir_offset = 0;

   dir_offset = mfat_get_root_dir_offset( dev_idx, part_idx );
   dir_offset += (mfat_get_root_dir_entries_count( dev_idx, part_idx ) *
      MFAT_DIR_ENTRY_SZ);

   return dir_offset;
}

uint16_t mfat_get_cluster_data_offset(
   uint16_t fat_idx, uint8_t dev_idx, uint8_t part_idx
) {
   uint16_t cluster_size = 0;
   uint16_t data_offset = 0;

   #if 0
   if( 0xffff == fat_entry ) {
      /* End of chain. */
      return 0;
   }
   #endif

   fat_idx -= 2; /* FAT quirk; cluster IDs start at 2 while clusters don't. */

   cluster_size = mfat_get_sectors_per_cluster( dev_idx, part_idx ) *
      mfat_get_bytes_per_sector( dev_idx, part_idx );

   data_offset = mfat_get_data_area_offset( dev_idx, part_idx );
   data_offset += fat_idx * cluster_size;

   return data_offset;
}

uint16_t mfat_get_entries_count( uint8_t dev_idx, uint8_t part_idx ) {
   return
      (mfat_get_sectors_per_fat( dev_idx, part_idx ) *
      mfat_get_bytes_per_sector( dev_idx, part_idx )) / 2;
}

uint16_t mfat_get_fat_entry( uint16_t idx, uint8_t dev_idx, uint8_t part_idx ) {
   uint16_t out = 0;
   uint16_t entry_offset = 0;

   /* Move past the BPB. */
   entry_offset += MFAT_OFFSET_FAT;
   entry_offset += (idx * 2);

   assert( idx <
      mfat_get_sectors_per_fat( dev_idx, part_idx ) *
      mfat_get_bytes_per_sector( dev_idx, part_idx ) );

   out |= disk_get_byte( dev_idx, part_idx, entry_offset + 1 );
   out <<= 8;
   out |= disk_get_byte( dev_idx, part_idx, entry_offset );
   return out;
}

uint16_t mfat_get_root_dir_first_entry_offset( uint8_t dev_idx, uint8_t part_idx ) {
   uint16_t dir_offset = 0;
   uint8_t entry_id = 0;
   uint8_t entry_attrib = 0;

   /* The root starts directly after the EBP and FATs. */
   dir_offset  = mfat_get_root_dir_offset( dev_idx, part_idx );

   /* Hunt for the first actual entry (i.e. skip LFNs, etc). */
   entry_attrib = mfat_get_dir_entry_attrib( dir_offset, dev_idx, part_idx );
   entry_id = disk_get_byte( dev_idx, part_idx, dir_offset );
   while(
      (0xe5 == entry_id || MFAT_ATTRIB_LFN == (MFAT_ATTRIB_LFN & entry_attrib))
      && 0x00 != entry_id
   ) {
      dir_offset += MFAT_DIR_ENTRY_SZ;
      entry_attrib = mfat_get_dir_entry_attrib( dir_offset, dev_idx, part_idx );
      entry_id = disk_get_byte( dev_idx, part_idx, dir_offset );
   }
   
   return dir_offset;
}

uint8_t mfat_filename_cmp(
   const char filename1[MFAT_FILENAME_LEN],
   const char filename2[MFAT_FILENAME_LEN]
) {
   int i = 0;
   char c1, c2;

   for( i = 0 ; MFAT_FILENAME_LEN > i ; i++ ) {
#ifdef ALPHA_PRESENT
      c1 = alpha_tolower( filename1[i] );
      c2 = alpha_tolower( filename2[i] );
#else
      c1 = filename1[i];
      c2 = filename2[i];
#endif /* ALPHA_PRESENT */
      /* Compare entry name with target. */
      if(
         c1 != c2 &&
         /* Skip spaces and .s. */
         ' ' != filename1[i] &&
         '.' != filename1[i] &&
         ' ' != filename2[i] &&
         '.' != filename2[i]
      ) {
         return 1;
      } else if( '\0' == filename1[i] || '\0' == filename2[i] ) {
         return 0;
      }
   }

   return 0;
}

uint16_t mfat_get_dir_entry_offset(
   const char search_name[MFAT_FILENAME_LEN], uint8_t search_name_len,
   uint16_t dir_offset, uint8_t dev_idx, uint8_t part_idx
) {
   uint16_t offset_out = dir_offset;
   char entry_name[MFAT_FILENAME_LEN];
   
   while( 0 != disk_get_byte( dev_idx, part_idx, offset_out ) ) {
      mfat_get_dir_entry_name( entry_name, offset_out, dev_idx, part_idx );
      if( 0 == mfat_filename_cmp( entry_name, search_name ) ) {
         /* Found it, so skip to the end. */
         goto hit;
      }
      offset_out =
         mfat_get_dir_entry_next_offset( offset_out, dev_idx, part_idx );
      if( 0 == offset_out ) {
         break;
      }
   }
   
   /* Not found. */
   offset_out = 0;

hit:
   return offset_out;
}

uint16_t mfat_get_dir_entry_next_offset(
   uint16_t offset, uint8_t dev_idx, uint8_t part_idx
) {
   uint16_t offset_out = offset;
   uint8_t entry_id = 0;
   uint8_t entry_attrib = 0;

   /* Loop through unused directory entries until we find a used one or just
    * reach the end of the directory. */
   do {
      offset_out += MFAT_DIR_ENTRY_SZ;
      entry_attrib = mfat_get_dir_entry_attrib( offset_out, dev_idx, part_idx );
      entry_id = disk_get_byte( dev_idx, part_idx, offset_out );
   } while(
      /* LFN entries are considered unused. */
      (0xe5 == entry_id || MFAT_ATTRIB_LFN == (MFAT_ATTRIB_LFN & entry_attrib))
      && 0x00 != entry_id
   );

   if( 0x00 == entry_id ) {
      /* End of the directory. */
      return 0;
   } else {
      /* Found a used entry. */
      return offset_out;
   }
}

static uint16_t mfat_get_dir_entry_first_cluster_idx(
   uint16_t entry_offset, uint8_t dev_idx, uint8_t part_idx
) {
   uint16_t out = 0;
   out |= disk_get_byte( dev_idx, part_idx, entry_offset + 27 );
   out <<= 8;
   out |= disk_get_byte( dev_idx, part_idx, entry_offset + 26 );

   /* TODO: Does this work? */
   //out += mfat_get_data_area_offset( dev_idx, part_idx );
   //out = mfat_get_cluster_data_offset( out, dev_idx, part_idx );

   return out;
}

uint8_t mfat_get_dir_entry_data(
   uint16_t entry_offset, uint16_t iter_offset, char* buffer, uint16_t blen,
   uint8_t dev_idx, uint8_t part_idx
) {
   uint16_t cluster_idx = 0;
   //uint16_t iter_offset = 0;
   //uint16_t iter_file_offset = 0;
   uint16_t read = 0;
   uint16_t file_size = 0;

   file_size = mfat_get_dir_entry_size( entry_offset, dev_idx, part_idx );
   if( file_size <= iter_offset ) {
      /* Seek past end of file. */
      buffer[0] = '\0';
      return 0;
   }

   cluster_idx = mfat_get_dir_entry_first_cluster_idx(
      entry_offset, dev_idx, part_idx );

   iter_offset += mfat_get_cluster_data_offset(
      cluster_idx, dev_idx, part_idx );
   file_size += mfat_get_cluster_data_offset(
      cluster_idx, dev_idx, part_idx );
#if 0
   do {
      /* Get disk offset containing the data at the requested file offset. */
      data_offset = mfat_get_cluster_data_offset(
         cluster_idx, dev_idx, part_idx );
      if( 0 == data_offset ) {
         /* Invalid cluster. */
         goto cleanup;
      }
      /* We've read sector * bps bytes from the file so far. */
      iter_file_offset += (mfat_get_bytes_per_sector( dev_idx, part_idx ) *
         mfat_get_sectors_per_cluster( dev_idx, part_idx ));
   while( file_offset > iter_file_offset );

   /* Determine the data offset within this cluster. */
   file_offset -= iter_file_offset;
#endif
   for(
      read = 0;
      blen > read && file_size > iter_offset + read;
      read++
   ) {
      buffer[read] =
         disk_get_byte( dev_idx, part_idx, iter_offset + read );
   }

//cleanup:
   return read;
}

void mfat_get_dir_entry_name(
   char buffer[MFAT_FILENAME_LEN],
   uint16_t offset, uint8_t dev_idx, uint8_t part_idx
) {
   int8_t src_i = 0, dest_i = 0;
   char c;

   /* Copy the entry name into the provided buffer. */
   for( src_i = 0 ; MFAT_FILENAME_LEN - 2 > src_i ; src_i++ ) {
      c = disk_get_byte( dev_idx, part_idx, offset + src_i );

      /* Just skip blanks. */
      if( ' ' != c ) {
         /* Only add the . if there's something after it. */
         if( 8 == src_i ) {
            buffer[dest_i++] = '.';
         }

         buffer[dest_i++] = c;
      }
   }

   /* Zero out the rest of the buffer. */
   while( dest_i < MFAT_FILENAME_LEN ) {
      buffer[dest_i++] = '\0';
   }
}

uint8_t mfat_get_dir_entry_cyear(
   uint16_t offset, uint8_t dev_idx, uint8_t part_idx
) {
   uint16_t year_out = 0;

   year_out = disk_get_byte( dev_idx, part_idx, offset + 17 );
   year_out &= 0xfe;
   year_out >>= 1;
   
   return (uint8_t)year_out;
}

uint32_t mfat_get_dir_entry_size(
   uint16_t offset, uint8_t dev_idx, uint8_t part_idx
) {
   uint32_t out = 0;
   out |= disk_get_byte( dev_idx, part_idx, offset + 31 );
   out <<= 8;
   out |= disk_get_byte( dev_idx, part_idx, offset + 30 );
   out <<= 8;
   out |= disk_get_byte( dev_idx, part_idx, offset + 29 );
   out <<= 8;
   out |= disk_get_byte( dev_idx, part_idx, offset + 28 );
   return out;
}

uint8_t mfat_get_dir_entry_attrib(
   uint16_t offset, uint8_t dev_idx, uint8_t part_idx
) {
   return disk_get_byte( dev_idx, part_idx, offset + 11 );
}

uint8_t mfat_get_dir_entry_cluster(
   uint16_t cluster_idx, uint16_t entry_offset, uint8_t dev_idx, uint8_t part_idx
) {
   /* TODO */
   return 0;
}

#endif /* USE_DISK */

