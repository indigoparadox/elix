
.data:

   prompt:  "\n>"
   tltext:  "\nline too long\n"
   nftext:  "\nfile not found\n"

   logo1: "     _____     \n"
   logo2: "   .`_| |_`.   \n"
   logo3: "  / /_| |_\\ \\  \n"
   logo4: " |  __| |___|  \n"
   logo5: " | |  | |      \n"
   logo6: "  \\ \\_| |___   \n"
   logo7: "   `._|_____/  \n"
   logo8: "               \n"

   starting: "\nStarting "
   elipses: "...\n"

.cpu:

   ;push     #1          ; Enable foreground I/O.
   ;syscall  flagon      ; Enable foreground I/O.

   pushd    #21         ; 20 chars + 1 NULL.
   malloc   $line

   pushd    #1
   malloc   $line_offst

   pushd    #2
   malloc   $diskpart_id

   pushd    sub_logo
   sjump

start:
   pushd    prompt
   syscall  puts

poll:
   syscall  getc        ; Put input char on the stack.
   push     #0
   jsne     proc_char   ; If input char != 0, process it.
   spop                 ; Else clear the stack.
   pushd    poll
   sjump                ; Poll again.

proc_char:
   push     '\n'        ; Push \n char to compare to input char in jseq.
   jseq     proc_line   ; JSEQ pops \n.
   mpushcd  $line_offst ; Push offset onto stack.
   pushd    #20         ; Push max line len to compare to offset.
   jsged    too_long    ; Jump if line too long (pops max line len).
   mpopo    $line       ; Pop input char to line+offset.
   mpushcd  $line_offst ; Push offset onto stack.
   mpushco  $line       ; Push input char back from line+offset (pops offset).
   syscall  putc        ; Print input char and remove it from stack.
   mpushcd  $line_offst
   pushd    #1
   saddd                ; Add 1 to offset (pops #1).
   mpopd    $line_offst ; Pops of offset to memory.
   push     #0          ; Push NULL to stack.
   mpushcd  $line_offst ; Push copy of offset to stack.
   mpopo    $line       ; Pop NULL to line+new offset (pops offset AND NULL).
   pushd    poll
   sjump

too_long:
   spop                 ; Remove input char from stack.
   spop                 ; Remove line offset from stack.
   spop                 ; Remove line offset from stack.
   pushd    tltext
   syscall  puts      ; Print warning (pops warning).
   pushd    #0
   mpopd    $line_offst ; Pop 0 offset to memory.
   pushd    start
   sjump

not_found:
   spop
   spop                 ; Pop FS offset.
   pushd    nftext
   syscall  puts
   pushd    start
   sjump

fs_match:
   spop                 ; Clear icmp result.
   
   pushd    starting
   syscall  puts
   pushd    $filename
   syscall  mputs
   pushd    elipses
   syscall  puts

   mpushcd  $fs_offset
   mpushcd  $diskpart_id   ; Push as double, read popped as 2 uint8_ts.
   syscall  launch
   spop                 ; Pop fs_offset from launch.
   spop                 ; Pop fs_offset from launch.
   ;push     #1          ; Disable foreground I/O.
   ;syscall  flagoff     ; Disable foreground I/O.

   ;mfree    $filename   ; Free filename buffer.
   ;mfree    $fs_offset
   pushd    #0          ; Push 0 line offset.
   mpopd    $line_offst ; Pop 0 line offset to memory.
   push     #0          ; Push NULL to stack.
   mpop     $line       ; Pop NULL to line char 0.
   pushd    start
   sjump

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
   pushd    #0
   jseqd    not_found
   pushd    #2          ; fs_offset is a double.
   malloc   $fs_offset  ; Allocate fs_offset.
   mpopd    $fs_offset  ; Store FS offset in memory.

fs_iter:
   mpushcd  $fs_offset  ; Place FS offset on the stack.
   pushd    $filename   ; Push address of filename buffer.
   push     #0          ; Push disk ID 0
   push     #0          ; Push part ID 0
   syscall  dname       ; Store entry name in $filename (pops offset and fname).
   spop                 ; Pop fs_offset from dname.
   spop                 ; Pop fs_offset from dname.

   ; DEBUG Show files as iterated.
   ;push     '\n'
   ;syscall  putc
   ;pushd    $filename
   ;syscall  mputs

   ;pushd    $filename
   ;pushd    $line
   pushd    icmp  ; Compare line and filename.
   sjump

icmp_finish:
   push     #0
   jseq     fs_match
   spop                 ; Clear icmp result.
   mpushcd  $fs_offset  ; Place FS offset on the stack.
   push     #0          ; Push disk ID 0
   push     #0          ; Push part ID 0
   syscall  dnext
   pushd    #0
   jseqd    fs_iter_cleanup   ; No more files in this directory.
   mpopd    $fs_offset  ; Store FS offset in memory.
   pushd    fs_iter ; Loop until found or no more.
   sjump

fs_iter_cleanup:
   spop                 ; Pop fs_offset.
   spop                 ; Pop fs_offset.
   pushd    #0          ; Push 0 line offset.
   mpopd    $line_offst ; Pop 0 line offset to memory.
   mfree    $filename   ; Free filename buffer.
   mfree    $fs_offset
   push     #0          ; Push NULL to stack.
   mpop     $line       ; Pop NULL to line char 0.
   pushd    start
   sjump

; Should be called after pushing:
; - line compare length
; Pushes 0 before jumping back if match; otherwise 1.
icmp:
   pushd    #2
   malloc   $icmp_idx   ; Start at index/offset 0.

icmp_loop:

   ; TODO: Insensitive comparison.

   mpushcd  $icmp_idx      ;
   pushd    #13            ; Compare 13 chars.
   jseqd    icmp_match     ; Reached max chars (pops #13).
   
   mpushco  $line          ; Push line address (idx still on stack from before).
   mpushcd  $icmp_idx      ; Push compare offset.
   mpushco  $filename      ; Push filename address.
   jsne     icmp_no_match  ; Chars didn't match.

   ;mpushcd  $icmp_idx
   push     #0
   jseq     icmp_match     ; Reaching NULL is a match.
   spop                    ; Pop filename comparison char.
   mpushcd  $icmp_idx
   pushd    #1
   saddd                   ; Add 1 to offset (pops #1).
   mpopd    $icmp_idx

   pushd    icmp_loop
   sjump

icmp_no_match:
   spop                    ; Pop filename comparison char.
   push     #1             ; 1 for no match.
   pushd    icmp_cleanup
   sjump

icmp_match:
   spop                    ; Pop filename comparison char.
   push     #0             ; 0 for match.
   pushd    icmp_cleanup
   sjump

icmp_cleanup:

   mfree    $icmp_idx

   pushd    icmp_finish
   sjump

sub_logo:

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

   sret

