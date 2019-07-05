
#include "../kernel.h"
#include "../disk.h"

#include <assert.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main( int argc, char** argv ) {
   int img_handle = 0;
   uint8_t* img_map = NULL;
   struct stat img_stat;
   size_t img_size = 0;
   int retval = 0;

   stat( "testimg.img", &img_stat );
   img_size = img_stat.st_size;

   img_handle = open( "testimg.img", O_RDONLY, 0 );
   assert( 0 < img_handle );

   img_map = mmap( NULL, img_size, PROT_READ,
      MAP_PRIVATE | MAP_POPULATE, img_handle, 0 );
   assert( MAP_FAILED != img_map );
   close( img_handle );
   disk_set_ptr( 0, img_map );

   retval = kmain( argc, argv );

   munmap( img_map, img_size );

   return retval;
}

