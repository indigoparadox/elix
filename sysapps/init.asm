
.data:

   prompt:  "\n>"
   tltext:  "\nline too long\n"
   nftext:  "\nfile not found\n"

   logo1: "     _____     \n",
   logo2: "   .`_| |_`.   \n",
   logo3: "  / /_| |_\\ \\  \n",
   logo4: " |  __| |___|  \n",
   logo5: " | |  | |      \n",
   logo6: "  \\ \\_| |___   \n",
   logo7: "   `._|_____/  \n",
   logo8: "               \n"

.cpu:

   pushd     #21        ; 20 chars + 1 NULL.
   malloc   $line

   pushd     #1
   malloc   $line_offst

   push     logo1
   syscall  puts
   push     logo2
   syscall  puts
   push     logo3
   syscall  puts
   push     logo4
   syscall  puts
   push     logo5
   syscall  puts
   push     logo6
   syscall  puts
   push     logo7
   syscall  puts
   push     logo8
   syscall  puts

start:
   push     prompt
   syscall  puts

poll:
   syscall  getc        ; Put input char on the stack.
   jsnz     proc_char   ; If input char != 0, process it.
   spop                 ; Else clear the stack.
   jump     poll        ; Poll again.

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
   mpopd    $line_offst ; Pops of offset to memory.
   push     #0          ; Push NULL to stack.
   mpushcd  $line_offst ; Push copy of offset to stack.
   mpopo    $line       ; Pop NULL to line+new offset (pops offset AND NULL).
   jump     poll

too_long:
   spop                 ; Remove input char from stack.
   spop                 ; Remove line offset from stack.
   spop                 ; Remove line offset from stack.
   push     tltext
   syscall  puts      ; Print warning (pops warning).
   pushd    #0
   mpopd    $line_offst ; Pop 0 offset to memory.
   jump     start

not_found:
   spop
   spop                 ; Pop FS offset.
   push     nftext
   syscall  puts
   jump     start

match:
   spop                 ; Clear icmp result.
   pushd    $filename
   syscall  mputs
   mfree    $filename   ; Free filename buffer.
   mfree    $fs_offset
   pushd    #0          ; Push 0 line offset.
   mpopd    $line_offst ; Pop 0 line offset to memory.
   push     #0          ; Push NULL to stack.
   mpop     $line       ; Pop NULL to line char 0.
   jump     start       ; TODO: Use sjump.

proc_line:
   spop                 ; Remove input char from stack.
   pushd    #13         ; 13-char filenames.
   malloc   $filename   ; Allocate filename buffer.
   push     #0          ; Push part ID 0
   push     #0          ; Push disk ID 0
   syscall  droot       ; Get the root directory offset.
   push     #0          ; Push part ID 0
   push     #0          ; Push disk ID 0
   syscall  dfirst      ; Get the first entry offset.
   pushd    #2
   jszd     not_found
   malloc   $fs_offset
   mpopd    $fs_offset  ; Store FS offset in memory.
   push     #0          ; Push part ID 0
   push     #0          ; Push disk ID 0
   mpushcd  $fs_offset  ; Place FS offset on the stack.
   pushd    $filename   ; Push address of filename buffer.
   syscall  dname       ; Store entry name in $filename (pops offset and fname).
   pushd    $filename
   pushd    $line
   push     #13         ; Compare at most 13 chars.
   push     ' '         ; Use space as separator.
   syscall  icmp
   jsz      match
   spop                 ; Clear icmp result.
   pushd    #0          ; Push 0 line offset.
   mpopd    $line_offst ; Pop 0 line offset to memory.
   mfree    $filename   ; Free filename buffer.
   mfree    $fs_offset
   push     #0          ; Push NULL to stack.
   mpop     $line       ; Pop NULL to line char 0.
   jump     start

