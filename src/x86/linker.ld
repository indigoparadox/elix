
ENTRY(_start)
SECTIONS {
   /* Start kernel at 1MB. */
   /*. = 1M;*/

   . = 0x1000;

   /* Start with multiboot header. */
   .text BLOCK(4K) : ALIGN(4K) {
      *(.multiboot)
      *(.text)
   }

   .rodata BLOCK(4K) : ALIGN(4K) {
      *(.rodata)
   }

   .data BLOCK(4K) : ALIGN(4K) {
      *(.data)
   }

   .bss BLOCK(4K) : ALIGN(4K) {
      *(COMMON)
      *(.bss)
      *(.stack)
   }
}
