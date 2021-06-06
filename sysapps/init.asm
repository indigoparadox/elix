
.data:

   prompt:  "\n>"
   tltext:  "\nline too long\n"

.cpu:

   push     #20
   malloc   $line

   push     #1
   malloc   $line_offst

start:
   push     prompt
   syscall  printf

poll:
   syscall  getc        ; Put input char on the stack.
   jsnz     proc_char   ; If input char != 0, process it.
   spop                 ; Else clear the stack.
   goto     poll        ; Poll again.

proc_char:
   push     '\n'        ; Push \n char to compare to input char in jseq.
   jseq     proc_line   ; JSEQ pops \n.
   mpushc   $line_offst ; Push offset onto stack.
   push     #20         ; Push max line len to compare to offset.
   jsge     too_long
   mpopco   $line       ; Pop input char copy to line+offset (pops offset).
   syscall  putc        ; Print input char and remove it from stack.
   mpushc   $line_offst
   push     #1
   sadd                 ; Add 1 to offset (pops #1).
   mpop     $line_offst ; Pops offset to memory.
   goto     poll

too_long:
   spop                 ; Remove input char from stack.
   spop                 ; Remove line offset from stack.
   push     tltext
   syscall  printf      ; Print warning (pops warning).
   push     #0
   mpop     $line_offst ; Pop 0 offset to memory.
   goto     start

proc_line:
   spop                 ; Remove input char from stack.
   push     #0
   mpop     $line_offst ; Pop 0 offset to memory.
   goto     start

