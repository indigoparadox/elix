
#include "../kernel.h"
#include "../disk.h"

#include <assert.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

uint8_t* g_img_map = NULL;
size_t g_img_sz = 0;

int main( int argc, char** argv ) {
   int img_handle = 0;
   struct stat img_stat;
   int retval = 0;

   stat( "testimg.img", &img_stat );
   g_img_sz = img_stat.st_size;

   assert( 0 < g_img_sz );

   img_handle = open( "testimg.img", O_RDWR, 0 );
   assert( 0 < img_handle );

   g_img_map = mmap( NULL, g_img_sz, PROT_READ | PROT_WRITE,
      MAP_SHARED, img_handle, 0 );
   assert( MAP_FAILED != g_img_map );

   retval = kmain( argc, argv );

   munmap( g_img_map, g_img_sz );
   close( img_handle );

   return retval;
}

