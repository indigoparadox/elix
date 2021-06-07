
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

   starting: "Starting "
   elipses: "..."

.cpu:

   ;push     #1          ; Enable foreground I/O.
   ;syscall  flagon      ; Enable foreground I/O.

   pushd    #21         ; 20 chars + 1 NULL.
   malloc   $line

   pushd    #1
   malloc   $line_offst

   pushd    #2
   malloc   $diskpart_id

   pushd    logo1
   syscall  puts
   pushd    logo2
   syscall  puts
   pushd    logo3
   syscall  puts
   pushd    logo4
   syscall  puts
   pushd    logo5
   syscall  puts
   pushd    logo6
   syscall  puts
   pushd    logo7
   syscall  puts
   pushd    logo8
   syscall  puts

start:
   pushd    prompt
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
   pushd    tltext
   syscall  puts      ; Print warning (pops warning).
   pushd    #0
   mpopd    $line_offst ; Pop 0 offset to memory.
   jump     start

not_found:
   spop
   spop                 ; Pop FS offset.
   pushd    nftext
   syscall  puts
   jump     start

match:
   spop                 ; Clear icmp result.
   
   pushd    starting
   syscall  puts
   pushd    $filename
   syscall  mputs
   pushd    elipses
   syscall  puts

   mpushcd  $diskpart_id   ; Push as double, read popped as 2 uint8_ts.
   mpushcd  $fs_offset
   syscall  launch
   ;push     #1          ; Disable foreground I/O.
   ;syscall  flagoff     ; Disable foreground I/O.
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
   push     #0          ; Push disk ID 0
   push     #0          ; Push part ID 0
   syscall  droot       ; Get the root directory offset.
   push     #0          ; Push disk ID 0
   push     #0          ; Push part ID 0
   syscall  dfirst      ; Get the first entry offset.
   jszd     not_found
   pushd    #2          ; fs_offset is a double.
   malloc   $fs_offset  ; Allocate fs_offset.
   mpopd    $fs_offset  ; Store FS offset in memory.

fs_iter:
   push     #0          ; Push disk ID 0
   push     #0          ; Push part ID 0
   mpushcd  $fs_offset  ; Place FS offset on the stack.
   pushd    $filename   ; Push address of filename buffer.
   syscall  dname       ; Store entry name in $filename (pops offset and fname).

   ; DEBUG Show files as iterated.
   ;push     '\n'
   ;syscall  putc
   ;pushd    $filename
   ;syscall  mputs

   pushd    $filename
   pushd    $line
   push     #13         ; Compare at most 13 chars.
   push     ' '         ; Use space as separator.
   syscall  icmp
   jsz      match
   spop                 ; Clear icmp result.
   mpushcd  $fs_offset  ; Place FS offset on the stack.
   push     #0          ; Push disk ID 0
   push     #0          ; Push part ID 0
   syscall  dnext
   jsz      fs_iter_cleanup   ; No more files in this directory.
   mpopd    $fs_offset  ; Store FS offset in memory.
   jump     fs_iter     ; Loop until found or no more.

fs_iter_cleanup:
   spop                 ; Pop fs_offset.
   spop                 ; Pop fs_offset.
   pushd    #0          ; Push 0 line offset.
   mpopd    $line_offst ; Pop 0 line offset to memory.
   mfree    $filename   ; Free filename buffer.
   mfree    $fs_offset
   push     #0          ; Push NULL to stack.
   mpop     $line       ; Pop NULL to line char 0.
   jump     start

