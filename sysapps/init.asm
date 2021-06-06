
.data:

   prompt:  "\n>"
   tltext:  "\nline too long\n"

.cpu:

   pushd     #20
   malloc   $line

   pushd     #1
   malloc   $line_offst

   pushd     #13
   malloc   $filename

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
   mpushcd  $line_offst ; Push offset onto stack.
   pushd    #20         ; Push max line len to compare to offset.
   jsged    too_long
   mpopco   $line       ; Pop input char copy to line+offset (pops offset).
   syscall  putc        ; Print input char and remove it from stack.
   mpushcd  $line_offst
   pushd    #1
   saddd                ; Add 1 to offset (pops #1).
   mpopd    $line_offst ; Pops offset to memory.
   goto     poll

too_long:
   spop                 ; Remove input char from stack.
   spop                 ; Remove line offset from stack.
   spop                 ; Remove line offset from stack.
   push     tltext
   syscall  printf      ; Print warning (pops warning).
   pushd    #0
   mpopd    $line_offst ; Pop 0 offset to memory.
   goto     start

proc_line:
   spop                 ; Remove input char from stack.
   pushd    #0          ; Push zero offset.
   mpopd    $line_offst ; Pop 0 offset to memory.
   push     #0          ; Push disk ID 0
   push     #0          ; Push part ID 0
   syscall  droot       ; Get the root directory offset.
   push     #0          ; Push disk ID 0
   push     #0          ; Push part ID 0
   syscall  dfirst      ; Get the first entry offset.
   spop
   spop
   goto     start

