
#include "x86bios.h"

__asm__ ("jmpl  $0, $main\n");
 
/* and for everything else you can use C! Be it traversing the filesystem, or verifying the kernel image etc.*/
 
void __NORETURN main(){
   kmain();
   while(1);
}

