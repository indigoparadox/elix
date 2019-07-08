
#include <check.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../src/mfat.h"

static uint8_t* g_img_map = NULL;
static size_t g_img_size = 0;

#define IMG_NAME_LEN 12
static char g_img_name[IMG_NAME_LEN] = "testimg.img";

static char* g_data_filename = NULL;
static uint8_t g_data_filename_len = 0;

uint8_t disk_get_byte( uint8_t dev_idx, uint8_t part_idx, size_t offset ) {
   return g_img_map[offset];
}

/* Convenient prototypes for normally static functions. */

uint16_t mfat_get_dir_entry_n_cluster_idx(
   uint32_t entry_offset, uint16_t iter_offset,
   uint8_t dev_idx, uint8_t part_idx );
uint16_t mfat_get_bytes_per_sector( uint8_t dev_idx, uint8_t part_idx );
uint8_t mfat_get_fat_count( uint8_t dev_idx, uint8_t part_idx );
uint8_t mfat_get_sectors_per_cluster(
   uint8_t dev_idx, uint8_t part_idx );
uint16_t mfat_get_sectors_per_fat( uint8_t dev_idx, uint8_t part_idx );
uint16_t mfat_get_root_dir_entries_count(
   uint8_t dev_idx, uint8_t part_idx );
uint32_t mfat_get_data_area_offset( uint8_t dev_idx, uint8_t part_idx );
uint16_t mfat_get_cluster_size( uint8_t dev_idx, uint8_t part_idx );
uint32_t mfat_get_cluster_data_offset(
   uint16_t fat_idx, uint8_t dev_idx, uint8_t part_idx );

#define MFAT_OFFSET_FAT 512

START_TEST( test_mfat_bpb ) {
   ck_assert_uint_eq( 512, mfat_get_bytes_per_sector( 0, 0 ) );
   ck_assert_uint_eq( 4  , mfat_get_sectors_per_cluster( 0, 0 ) );
   ck_assert_uint_eq( 2  , mfat_get_fat_count( 0, 0 ) );
   ck_assert_uint_eq( 20 , mfat_get_sectors_per_fat( 0, 0 ) );
   ck_assert_uint_eq( 512, mfat_get_root_dir_entries_count( 0, 0 ) );
}
END_TEST

START_TEST( test_mfat_entry ) {
   uint16_t entry = mfat_get_fat_entry( _i, 0, 0 );

   ck_assert_uint_eq( entry, g_img_map[0x200 + (2 * _i)] );
}
END_TEST

START_TEST( test_mfat_consistency ) {
   uint16_t last_entry;

   last_entry = mfat_get_fat_entry( _i - 1, 0, 0 );

   ck_assert(
      0x0000 == last_entry ||
      0xffff == last_entry ||
      last_entry == _i );
}
END_TEST

START_TEST( test_mfat_fat_sz ) {
   //mfat_get_dir_entry_n_cluster_idx( 0, 0, 0, 0 );
}
END_TEST

START_TEST( test_mfat_cluster_data ) {
   uint16_t cluster_idx = 0;
   uint32_t file_on_disk_offset = 0;
   uint32_t file_size = 0;
   uint32_t cluster_end_on_disk = 0;
   uint32_t iter_offset = 0;
   uint32_t entry_offset = 0;
   uint32_t cluster_size = 0;
   uint32_t cluster_remainder = 0;
   unsigned char test_byte = 0;

   entry_offset = mfat_get_root_dir_first_entry_offset( 0, 0 );
   entry_offset = mfat_get_dir_entry_offset(
      g_data_filename, g_data_filename_len, entry_offset, 0, 0 );
   file_size = mfat_get_dir_entry_size( entry_offset, 0, 0 );

   ck_assert_int_lt( _i, file_size );

   cluster_size = mfat_get_cluster_size( 0, 0 );
   iter_offset = _i % cluster_size;

   /* Grab the index of the cluster containing the requested chunk of the file
    * on the FAT. Then grab the offset in the data area.
    */
   cluster_idx = mfat_get_dir_entry_n_cluster_idx(
      entry_offset, iter_offset, 0, 0 );

   ck_assert_int_ne( cluster_idx, 0 );

   /* Cluster index is valid, so translate it to a usable offset. */
   file_on_disk_offset = mfat_get_cluster_data_offset( cluster_idx, 0, 0 );
   cluster_remainder = file_on_disk_offset % cluster_size;

   ck_assert_int_ne( file_on_disk_offset, 0 );
   ck_assert_int_lt(
      mfat_get_cluster_size( 0, 0 ), cluster_size + file_on_disk_offset );
   ck_assert_int_eq( 0, cluster_remainder );

   /* Set an end-of-file limit. */
   cluster_end_on_disk = file_on_disk_offset + cluster_size;

   ck_assert_int_lt( file_on_disk_offset + iter_offset, cluster_end_on_disk );

   test_byte = disk_get_byte( 0, 0, file_on_disk_offset + iter_offset );
   printf( "%c\n", test_byte );
}
END_TEST

static void setup_disk() {
   int img_handle = 0;
   struct stat img_stat;

   stat( g_img_name, &img_stat );
   g_img_size = img_stat.st_size;

   img_handle = open( g_img_name, O_RDONLY, 0 );
   assert( 0 < img_handle );

   g_img_map = mmap( NULL, g_img_size, PROT_READ,
      MAP_PRIVATE | MAP_POPULATE, img_handle, 0 );
   ck_assert_ptr_ne( MAP_FAILED, g_img_map );
   close( img_handle );
}

static void teardown_disk() {
   munmap( g_img_map, g_img_size );
}

static void setup_data_short() {
   setup_disk();
   g_data_filename = "fox.txt";
   g_data_filename_len = 7;
}

static void teardown_data_short() {
   teardown_disk();
}

Suite* mfat_suite( void ) {
   Suite* s;
   TCase* tc_metadata;
   TCase* tc_data_short;
   //TCase* tc_data_long;

   s = suite_create( "mfat" );

   /* Core test case */
   tc_metadata = tcase_create( "Metadata" );

   //tcase_add_loop_test( tc_metadata, test_mfat_, 0, 4 );
   tcase_add_checked_fixture( tc_metadata, setup_disk, teardown_disk );
   tcase_add_test( tc_metadata, test_mfat_bpb );
   tcase_add_loop_test( tc_metadata, test_mfat_entry, 2, 50 );
   tcase_add_test( tc_metadata, test_mfat_fat_sz );
   tcase_add_loop_test( tc_metadata, test_mfat_consistency, 2, 50 );

   /* File <= 1 cluster. */
   tc_data_short = tcase_create( "DataShort" );
   tcase_add_checked_fixture(
      tc_data_short, setup_data_short, teardown_data_short );
   tcase_add_loop_test( tc_data_short, test_mfat_cluster_data, 0, 20 );

   /* File > 1 cluster. */
   /*
   tc_data_long = tcase_create( "DataLong" );
   tcase_add_checked_fixture(
      tc_data_long, setup_data_long, teardown_data_long );
   tcase_add_loop_test( tc_data_long, test_mfat_cluster_data, 0, 20 );
   */

   suite_add_tcase( s, tc_metadata );
   suite_add_tcase( s, tc_data_short );

   return s;
}

