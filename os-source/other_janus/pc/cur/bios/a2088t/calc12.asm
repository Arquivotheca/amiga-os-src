       page 66,132
cr	       equ 0dh
lf	       equ 0ah
;
stack  segment para    stack   'stack'
       db      100h dup (?)
stack  ends

work   segment para public 'data'
;
; messages
;
msg0   db cr,lf,'Calculating checksum for 8K-EPROM',cr,lf
       db 'Size of output file will be 12kByte.',cr,lf,0
msg3   db 'File Error',cr,lf,0
vemsg  db 'Verify Error',cr,lf,0
vomsg  db 'Verify OK',cr,lf,0
fsmsg  db 'WARNING - File size exceeds 12 KBytes',cr,lf,0
ckmsg  db 'WARNING - ROM Checksum not 00',cr,lf,0
;
; file extensions
;
exbin  db '.bin',0
;
; buffers
;
filnl  dw 0		       ; length of filename
filnam db 80 dup (0)	       ; file name buffer
infile dw 0		       ; input file handle
inbuf  db 4096+8192 dup (0)
buflen dw 0
;
work   ends

cseg   segment para public 'code'

start  proc far
       assume cs:cseg,ds:cseg,ss:stack,es:work

       push    ds
       sub     ax,ax
       push    ax
       mov     ax,work
       mov     es,ax

       cld
;
; look for filename given with command line
;
       mov     si,80h		      ; offset of parameter area in PSP
       mov     cl,[si]		      ; get file name length
       and     cx,00ffh 	      ; is 0 ?
       je      look1		      ; if eq, yes -> error

look:  inc     si		      ; skip leading spaces
       cmp     byte ptr [si],' '
       jne     look2
       loop    look

       jmp     short look1	      ; illegal filename given

look2: mov     di,offset work:filnam  ; move filename to buffer
       mov     filnl,cx 	      ; save remaining length
       rep     movsb		      ; do move

look1: mov     ax,work		      ; map to our data segment
       mov     ds,ax
       assume  ds:work

       mov     bx,offset msg0	      ; print ID message
       call    pstrng
;
; extension given ?
;
chkxt: mov     ax,filnl
       or      ax,ax		       ; any name given ?
       jnz     ext10		       ; stop, if not
       jmp     filerr
ext10: cld			       ; loop upwards
       mov     cx,ax		       ; length of name
       mov     di,offset filnam        ; point to start of name
       mov     al,'.'		       ; look for '.'
       repnz   scasb		       ; search
       jnz     notfnd		       ; if z, found
;
; extension found
;
       mov     bx,offset filnam
       add     bx,filnl
       mov     byte ptr [bx],0
       jmp     short open
;
; no ext found, set extension .BIN
;
notfnd: 			       ; bx points to 1st byte after name
       mov     si,offset exbin
       mov     di,offset filnam
       add     di,filnl 	       ; point after name
       mov     cx,5		       ; extend with 5 bytes
       rep     movsb
;
; open file for read
;
open:  mov     dx,offset filnam        ; point to file name
       mov     ax,3d00h 	       ; open for read
       int     21h		       ; call DOS
       mov     infile,ax
       jnc     read		       ; if cf, something wrong
       jmp     filerr
;
; Read	8K from input file
;
read:  mov     bx,infile	       ; file handle
       mov     cx,3000h 	       ; 12k bytes
       mov     dx,offset inbuf	       ; to inbuf
       mov     ah,3fh		       ; buffer read function
       int     21h		       ; call dos
       mov     buflen,AX	       ; number of ALL BYTES
;
; Calculate Checksum
;
       mov     si,OFFSET INBUF	       ; offset
       mov     al,0		       ; clear ckecksum
       mov     cx,1FFFH 	       ; ROM length
ckad:  add     al,[si]		       ; add all bytes of RAM
       inc     si		       ; bump pointer
       loop    ckad		       ; until done
       NEG     AL
       MOV     [SI],AL
				
;
; close input file
;
close: mov     ax,3e00h
       mov     bx,infile
       int     21h
       jc      filerr		       ; if cf, something wrong

;
; CREATE FILE AND OPEN IT
;
       mov     dx,offset filnam        ; point to file name
       mov     ax,3C00h 	       ; open for read
       SUB     CX,CX		       ; SET ATTRIBUTE
       int     21h		       ; call DOS
       jnc     WRITE		       ; if cf, something wrong
       jmp     filerr


;
; Write 8K to output file
;
write: mov     bx,infile	       ; file handle
       mov     cx,3000h 	       ; 12k bytes
       mov     dx,offset inbuf	       ; to inbuf
       mov     ah,40h		       ; buffer write function
       int     21h		       ; call dos
       mov     buflen,AX	       ; number of ALL BYTES
       JC	FILERR

;
; close OUTPUT file
;
       mov     ax,3e00h
       mov     bx,infile
       int     21h
       jc      filerr		       ; if cf, something wrong

;
; return to DOS
;

stop:  ret			       ; all done

;
; report file error
;
filerr:
       mov     bx,offset msg3	       ; point to message
       call    pstrng		       ; print it
       jmp     stop		       ; terminate


start  endp
;
; subroutines
;
pstr0  proc    near		       ; print message
       mov     ah,2		       ; pointed to by [bx]
       int     21h		       ; terminated by 00h
       inc     bx
pstrng:
       mov     dl,[bx]
       or      dl,dl
       jnz     pstr0
       ret
pstr0  endp

cseg   ends

       end     start
