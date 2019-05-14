# Multiboot header.
.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Allocate a temporary 16KB stack, somewhere in memory.
.section .bootstrap_stack
stack_bottom:
.skip 16384 # 16 KB
stack_top:

# Start!
.section .text
.global _start
_start:

movl  $stack_top, %esp
call kmain

# If the kernel returns, disable interrupts (cli) and spin forever.
cli
hang:
hlt
jmp hang
