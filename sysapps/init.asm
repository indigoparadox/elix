
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

   spush    #21         ; 20 chars + 1 NULL.
   spush    $line
   sysc     malloc

   spush    #2
   spush    $line_offst
   sysc     malloc

   spush    #2
   spush    $diskpart_id
   sysc     malloc

   spush   sub_logo
   sjump

start:
   spush    prompt
   sysc     puts

poll:
   sysc     getc        ; Put input char on the stack.
   spush    #0
   jsne     proc_char   ; If input char != 0, process it.
   spop                 ; Else clear the stack.
   jump     poll                ; Poll again.

proc_char:
   spush    '\n'        ; Push \n char to compare to input char in jseq.
   jseq     proc_line   ; JSEQ pops \n.
   spush    #0          ; No MPUSH offset.
   spush    $line_offst
   sysc     mpush       ; Push offset onto stack.
   spush    #20         ; Push max line len to compare to offset.
   jsge     too_long    ; Jump if line too long (pops max line len).
   spush    $line
   sysc     mpop        ; Pop input char to line+offset.
   spush    #0          ; No MPUSH offset.
   spush    $line_offst
   sysc     mpush       ; Push offset onto stack.
   spush    $line
   sysc     mpush       ; Push input char back from line+offset (pops offset).
   sysc     putc        ; Print input char and remove it from stack.
   spush    #0          ; No MPUSH offset.
   spush    $line_offst
   sysc     mpush 
   spush    #1
   sadd                 ; Add 1 to offset (pops #1).
   spush    #0          ; No MPOP offset.
   spush    $line_offst
   sysc     mpop        ; Pops of offset to memory.
   spush    #0          ; Push NULL to stack.
   spush    #0          ; No MPUSH offset.
   spush    $line_offst
   sysc     mpush       ; Push copy of offset to stack.
   spush    $line
   sysc     mpop        ; Pop NULL to line+new offset (pops offset AND NULL).
   jump     poll

too_long:
   spop                 ; Remove input char from stack.
   spop                 ; Remove line offset from stack.
   spush    tltext
   sysc     puts        ; Print warning (pops warning).
   spush    #0
   spush    #0          ; No MPOP offset.
   spush    $line_offst
   sysc     mpop        ; Pop 0 offset to memory.
   jump     start

not_found:
   ;spop                ; Is this a dupe?
   spop                 ; Pop FS offset.
   spush    nftext
   sysc     puts
   jump     start

fs_match:
   spop                    ; Clear icmp result.
   
   spush    starting
   sysc     puts
   spush    $filename
   sysc     mputs
   spush    elipses
   sysc     puts

   spush    #0          ; No MPUSH offset.
   spush    $file_id
   sysc     mpush 
   spush    #0          ; No MPUSH offset.
   spush    $diskpart_id   ; Push as double, read popped as 2 uint8_ts.
   sysc     mpush 
   sysc     launch
   spop                    ; Pop file_id from launch.
   ;push     #1            ; Disable foreground I/O.
   ;sysc     flagoff       ; Disable foreground I/O.

   ;mfree    $filename     ; Free filename buffer.
   ;mfree    $file_id
   spush    #0             ; Push 0 line offset.
   spush    #0          ; No MPOP offset.
   spush    $line_offst
   sysc     mpop           ; Pop 0 line offset to memory.
   spush    #0             ; Push NULL to stack.
   spush    #0          ; No MPOP offset.
   spush    $line
   sysc     mpop           ; Pop NULL to line char 0.
   jump     start

proc_line:
   spop                 ; Remove input char from stack.
   spush    #13         ; 13-char filenames.
   spush    $filename
   sysc     malloc      ; Allocate filename buffer.
   spush    #0          ; Push disk ID 0
   spush    #0          ; Push part ID 0
   sysc     droot       ; Get the root directory offset.
   sysc     dfirst      ; Get the first entry offset.
   spush    #0
   jseq     not_found
   spush    #2          ; file_id is 16 bits.
   spush    $file_id
   sysc     malloc      ; Allocate file_id.
   spush    #0          ; No MPOP offset.
   spush    $file_id
   sysc     mpop        ; Store FS offset in memory.

fs_iter:
   spush    #0          ; No MPUSH offset.
   spush    $file_id  ; Place FS offset on the stack.
   sysc     mpush 
   spush    $filename   ; Push address of filename buffer.
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
   spush    #0
   jseq     fs_match
   spop                       ; Clear icmp result.
   spush    #0          ; No MPUSH offset.
   spush    $file_id
   sysc     mpush             ; Place FS offset on the stack.
   sysc     dnext
   spush    #0
   jseq     fs_iter_cleanup   ; No more files in this directory.
   spush    #0          ; No MPOP offset.
   spush    $file_id
   sysc     mpop              ; Store FS offset in memory.
   jump     fs_iter           ; Loop until found or no more.

fs_iter_cleanup:
   spop                 ; Pop file_id.
   spush    #0          ; Push 0 line offset.
   spush    #0          ; No MPOP offset.
   spush    $line_offst
   sysc     mpop        ; Pop 0 line offset to memory.
   spush    $filename
   sysc     mfree       ; Free filename buffer.
   spush    $file_id
   sysc     mfree
   spush    #0          ; Push NULL to stack.
   spush    #0          ; No MPOP offset.
   spush    $line
   sysc     mpop        ; Pop NULL to line char 0.
   jump     start

; Should be called after pushing:
; - line compare length
; Pushes 0 before jumping back if match; otherwise 1.
icmp:
   spush    #2
   spush    $icmp_idx
   sysc     malloc         ; Start at index/offset 0.

icmp_loop:

   ; TODO: Insensitive comparison.

   spush    #0          ; No MPUSH offset.
   spush    $icmp_idx
   sysc     mpush 
   spush    #13            ; Compare 13 chars.
   jseq     icmp_match     ; Reached max chars (pops #13).
   
   spush    $line          ; Push line address (idx still on stack from before).
   sysc     mpush  
   spush    #0          ; No MPUSH offset.
   spush    $icmp_idx      ; Push compare offset.
   sysc     mpush 
   spush    $filename      ; Push filename address.
   sysc     mpush  
   jsne     icmp_no_match  ; Chars didn't match.

   ;mpushcd  $icmp_idx
   spush    #0
   jseq     icmp_match     ; Reaching NULL is a match.
   spop                    ; Pop filename comparison char.
   spush    #0          ; No MPUSH offset.
   spush    $icmp_idx
   sysc     mpush 
   spush    #1
   sadd                    ; Add 1 to offset (pops #1).
   spush    #0          ; No MPOP offset.
   spush    $icmp_idx
   sysc     mpop

   jump     icmp_loop

icmp_no_match:
   spop                    ; Pop filename comparison char.
   spush    #1             ; 1 for no match.
   jump     icmp_cleanup

icmp_match:
   spop                    ; Pop filename comparison char.
   spush    #0             ; 0 for match.
   jump     icmp_cleanup

icmp_cleanup:

   spush    $icmp_idx
   sysc     mfree

   jump     icmp_finish

sub_logo:

   spush    logo1
   sysc     puts
   spush    logo2
   sysc     puts
   spush    logo3
   sysc     puts
   spush    logo4
   sysc     puts
   spush    logo5
   sysc     puts
   spush    logo6
   sysc     puts
   spush    logo7
   sysc     puts
   spush    logo8
   sysc     puts

   sret

