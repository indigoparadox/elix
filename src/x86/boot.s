
#.code16gcc

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

lgdt %cs:gdtr16
lidt %cs:idtr16
ljmp $8, $cont
cont:
.code16
movw $0x10, %ax
movw %ax, %ds
movw %ax, %es
movw %ax, %ss
movw $stack_top, %sp
movl %cr0, %eax
andb $~1, %al
movl %eax, %cr0
jmp cont2
cont2:
xorw %ax, %ax
movw %ax, %ds
movw %ax, %es
movw %ax, %ss
ljmp $0, $kmain

.balign 8
# 16-bit GDTR
gdtr16:
.hword gdt16end-gdt16
.long gdt16

# GDT with 16-bit code segment and data segment descriptor
.set gdt16, .-8
.quad 0x008f9b000000ffff
.quad 0x008f93000000ffff
gdt16end:

# Real mode IDTR
idtr16:
.hword 0x3ff
.long 0
