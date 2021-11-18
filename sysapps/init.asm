
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
   ;sysc     flagon      ; Enable foreground I/O.

   push     #21         ; 20 chars + 1 NULL.
   push     $line
   sysc     malloc

   push     #2
   push     $line_offst
   sysc     malloc

   push     #2
   push     $diskpart_id
   sysc     malloc

   push    sub_logo
   sjump

start:
   push     prompt
   sysc     puts

poll:
   sysc     getc        ; Put input char on the stack.
   push     #0
   jsne     proc_char   ; If input char != 0, process it.
   spop                 ; Else clear the stack.
   jump     poll                ; Poll again.

proc_char:
   push     '\n'        ; Push \n char to compare to input char in jseq.
   jseq     proc_line   ; JSEQ pops \n.
   push     #0          ; No MPUSH offset.
   push     $line_offst
   sysc     mpush       ; Push offset onto stack.
   push     #20         ; Push max line len to compare to offset.
   jsge     too_long    ; Jump if line too long (pops max line len).
   push     $line
   sysc     mpop        ; Pop input char to line+offset.
   push     #0          ; No MPUSH offset.
   push     $line_offst
   sysc     mpush       ; Push offset onto stack.
   push     $line
   sysc     mpush       ; Push input char back from line+offset (pops offset).
   sysc     putc        ; Print input char and remove it from stack.
   push     #0          ; No MPUSH offset.
   push     $line_offst
   sysc     mpush 
   push     #1
   sadd                 ; Add 1 to offset (pops #1).
   push     #0          ; No MPOP offset.
   push     $line_offst
   sysc     mpop        ; Pops of offset to memory.
   push     #0          ; Push NULL to stack.
   push     #0          ; No MPUSH offset.
   push     $line_offst
   sysc     mpush       ; Push copy of offset to stack.
   push     $line
   sysc     mpop        ; Pop NULL to line+new offset (pops offset AND NULL).
   jump     poll

too_long:
   spop                 ; Remove input char from stack.
   spop                 ; Remove line offset from stack.
   push     tltext
   sysc     puts        ; Print warning (pops warning).
   push     #0
   push     #0          ; No MPOP offset.
   push     $line_offst
   sysc     mpop        ; Pop 0 offset to memory.
   jump     start

not_found:
   ;spop                ; Is this a dupe?
   spop                 ; Pop FS offset.
   push     nftext
   sysc     puts
   jump     start

fs_match:
   spop                    ; Clear icmp result.
   
   push     starting
   sysc     puts
   push     $filename
   sysc     mputs
   push     elipses
   sysc     puts

   push     #0          ; No MPUSH offset.
   push     $file_id
   sysc     mpush 
   push     #0          ; No MPUSH offset.
   push     $diskpart_id   ; Push as double, read popped as 2 uint8_ts.
   sysc     mpush 
   sysc     launch
   spop                    ; Pop file_id from launch.
   ;push     #1            ; Disable foreground I/O.
   ;sysc     flagoff       ; Disable foreground I/O.

   ;mfree    $filename     ; Free filename buffer.
   ;mfree    $file_id
   push     #0             ; Push 0 line offset.
   push     #0          ; No MPOP offset.
   push     $line_offst
   sysc     mpop           ; Pop 0 line offset to memory.
   push     #0             ; Push NULL to stack.
   push     #0          ; No MPOP offset.
   push     $line
   sysc     mpop           ; Pop NULL to line char 0.
   jump     start

proc_line:
   spop                 ; Remove input char from stack.
   push     #13         ; 13-char filenames.
   push     $filename
   sysc     malloc      ; Allocate filename buffer.
   push     #0          ; Push disk ID 0
   push     #0          ; Push part ID 0
   sysc     droot       ; Get the root directory offset.
   push     #0          ; Push disk ID 0
   push     #0          ; Push part ID 0
   sysc     dfirst      ; Get the first entry offset.
   push     #0
   jseq     not_found
   push     #2          ; file_id is 16 bits.
   push     $file_id
   sysc     malloc      ; Allocate file_id.
   push     #0          ; No MPOP offset.
   push     $file_id
   sysc     mpop        ; Store FS offset in memory.

fs_iter:
   push     #0          ; No MPUSH offset.
   push     $file_id  ; Place FS offset on the stack.
   sysc     mpush 
   push     $filename   ; Push address of filename buffer.
   push     #0          ; Push disk ID 0
   push     #0          ; Push part ID 0
   sysc     dname       ; Store entry name in $filename (pops offset and fname).
   spop                 ; Pop file_id from dname.

   ; DEBUG Show files as iterated.
   ;push     '\n'
   ;sysc     putc
   ;push     $filename
   ;sysc     mputs

   ;push     $filename
   ;push     $line
   jump     icmp  ; Compare line and filename.

icmp_finish:
   push     #0
   jseq     fs_match
   spop                       ; Clear icmp result.
   push     #0          ; No MPUSH offset.
   push     $file_id
   sysc     mpush             ; Place FS offset on the stack.
   push     #0                ; Push disk ID 0
   push     #0                ; Push part ID 0
   sysc     dnext
   push     #0
   jseq     fs_iter_cleanup   ; No more files in this directory.
   push     #0          ; No MPOP offset.
   push     $file_id
   sysc     mpop              ; Store FS offset in memory.
   jump     fs_iter           ; Loop until found or no more.

fs_iter_cleanup:
   spop                 ; Pop file_id.
   push     #0          ; Push 0 line offset.
   push     #0          ; No MPOP offset.
   push     $line_offst
   sysc     mpop        ; Pop 0 line offset to memory.
   push     $filename
   sysc     mfree       ; Free filename buffer.
   push     $file_id
   sysc     mfree
   push     #0          ; Push NULL to stack.
   push     #0          ; No MPOP offset.
   push     $line
   sysc     mpop        ; Pop NULL to line char 0.
   jump     start

; Should be called after pushing:
; - line compare length
; Pushes 0 before jumping back if match; otherwise 1.
icmp:
   push     #2
   push     $icmp_idx
   sysc     malloc         ; Start at index/offset 0.

icmp_loop:

   ; TODO: Insensitive comparison.

   push     #0          ; No MPUSH offset.
   push     $icmp_idx
   sysc     mpush 
   push     #13            ; Compare 13 chars.
   jseq     icmp_match     ; Reached max chars (pops #13).
   
   push     $line          ; Push line address (idx still on stack from before).
   sysc     mpush  
   push     #0          ; No MPUSH offset.
   push     $icmp_idx      ; Push compare offset.
   sysc     mpush 
   push     $filename      ; Push filename address.
   sysc     mpush  
   jsne     icmp_no_match  ; Chars didn't match.

   ;mpushcd  $icmp_idx
   push     #0
   jseq     icmp_match     ; Reaching NULL is a match.
   spop                    ; Pop filename comparison char.
   push     #0          ; No MPUSH offset.
   push     $icmp_idx
   sysc     mpush 
   push     #1
   sadd                    ; Add 1 to offset (pops #1).
   push     #0          ; No MPOP offset.
   push     $icmp_idx
   sysc     mpop

   jump     icmp_loop

icmp_no_match:
   spop                    ; Pop filename comparison char.
   push     #1             ; 1 for no match.
   jump     icmp_cleanup

icmp_match:
   spop                    ; Pop filename comparison char.
   push     #0             ; 0 for match.
   jump     icmp_cleanup

icmp_cleanup:

   push     $icmp_idx
   sysc     mfree

   jump     icmp_finish

sub_logo:

   push     logo1
   sysc     puts
   push     logo2
   sysc     puts
   push     logo3
   sysc     puts
   push     logo4
   sysc     puts
   push     logo5
   sysc     puts
   push     logo6
   sysc     puts
   push     logo7
   sysc     puts
   push     logo8
   sysc     puts

   sret

