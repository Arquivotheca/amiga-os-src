;************************************************************************/
;*                                                                      */
;* EGDFIOSL.ASM : EGDrive file I/O routine    for large Model           */
;*                                                      MS-C            */
;*                   Version 3.41                                       */
;*                                                                      */
;*          Designed    by    I.Iwata         07/31/1987                */
;*          Coded       by    I.Iwata         07/31/1987                */
;*          Modified    by    Y.Katogi        08/17/1989                */
;*          Modified    by    Y.Katogi        06/24/1991                */
;*                            (mod) bsfopen : select open method        */
;*          Modified    by    H.Yanata        06/24/1991                */
;*                            (mod) bsfread : file end process          */
;*                            (add) bsftell : get file length           */
;*                                                                      */
;*    (C) Copyright 987-89 ERGOSOFT Corp.                               */
;*        All Rights Reserved                                           */
;*                                                                      */
;*                          --- NOTE ---                                */
;*                                                                      */
;*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
;*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
;*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
;*                                                                      */
;*----------------------------------------------------------------------*/
;*                                                                      */
;*    1.    bsfopen(fname,mode)                                         */
;*    2.    bsfclose(hdl)                                               */
;*    3.    bsfread(buffer,size,count,hdl)                              */
;*    4.    bsfwrite(buffer,size,count,hdl)                             */
;*    5.    bsfseek(hdl,offset,origin)                                  */
;*    6.    bsftell(hdl)                                                */
;************************************************************************/
    include        prologul.h

    public    _bsfopen     ;open  file
    public    _bsfclose    ;close file
    public    _bsfwrite    ;write to file
    public    _bsfread     ;read  from file
    public    _bsfseek     ;seek  file
    public    _bsftell     ;get   file size

;**********************************************************
;*                                                        *
;*        WORD bsfopen(fname,mode)                        *
;*                                                        *
;*        open file                                       *
;*                                                        *
;*        BYTE    *fname;                                 *
;*        WORD    mode; read=0, write=1, read/write=2     *
;*                                                        *
;*        f/v     file handle  (0 ; error)                *
;*                                                        *
;**********************************************************

_bsfopen proc    far 

    push    bp                   ;save bp
    mov     bp,sp                ;bp point first stack
    push    dx                   ;save dx
    push    ds

;                        dx = path name address
;   read or read/write : ah = 3dh  al = 0 or 2
;   write              : ah = 3ch  al = 1

    lds     dx,ss:[bp+@ab]        ;get fname pointer/segment
    mov     al,ss:[bp+@ab+4]      ;get mode
    cmp     al,01h
    jz      bcreat
    mov     ah,3dh                ;function = open
    jmp     opengo
bcreat:
    mov     ah,3ch                ;function = create
    mov     cx,0h
opengo:
    int     21h                   ;exec file open
    jnc     bopext                ;normal end
    mov     ax,0h                 ;set error
bopext: 
    pop     ds
    pop     dx                    ;recover dx 
    mov     sp,bp                 ;recover sp
    pop     bp                    ;recover bp
    ret
_bsfopen endp


;**********************************************************
;*                                                        *
;*        WORD bsfclose(hdl)                              *
;*                                                        *
;*        close file                                      *
;*                                                        *
;*        WORD  hdl; file handle                          *
;*                                                        *
;*        f/v   0 ; normal end                            *
;*              1 ; error  end                            *
;*                                                        *
;**********************************************************

_bsfclose proc     far

    push    bp                    ;save bp to stack
    mov     bp,sp                 ;bp-->argument
    push    bx                    ;save bx

    mov     bx,ss:[bp+@ab]        ;get file handle
    mov     ah,3eh                ;function = close
    int     21h                   ;exec close file
    jnc     bclextn               ;normal end
    mov     ax,1                  ;set error    end ax=1
    jmp     short bclexte         ;error end
bclextn:
    mov     ax,0                  ;set normal end ax=0    
bclexte:
    pop     bx                    ;recover bx
    mov     sp,bp                 ;recover sp
    pop     bp                    ;recover bp
    ret                           ;all done
_bsfclose endp

;**********************************************************
;*                                                        *
;*        WORD    bsfread(buffer,size,count,hdl)          *
;*                                                        *
;*        read block to I/O buffer                        *
;*                                                        *
;*        BYTE  *buffer;     I/O buffer                   *
;*        WORD  size;      I/O size                       *
;*        WORD  count;     I/O count                      *
;*        WORD  hdl;         file handle                  *
;*                                                        *
;*      f/v   0 ; normal end (not end of file)            *
;*            1 ; error  end                              *
;*            2 ; normal end (end of file)                *
;**********************************************************

_bsfread proc    far

    push    bp                    ;save bp
    mov     bp,sp                ;bp-->arg

    push    bx                    ;save bx
    push    cx                    ;save cx
    push    dx                    ;save dx
    push    ds                    ;save ds

;        ds/dx : I/O buffer ptr

    mov     ax,ss:[bp+@ab+4]
    mul     word ptr ss:[bp+@ab+6]
    mov     cx,ax                 ;get I/O length    
    mov     bx,ss:[bp+@ab+8]      ;get files handle
    lds     dx,ss:[bp+@ab]        ;get I/O buffer ptr offset/seg
    mov     ah,3fh                ;function code=read
    int     21h                   ;exec read
    jc      brderr                ;error return
    jmp     short brdextn         ;normal return(not file end)

brdextn:
    cmp     ax,00h                ;end of file?
    je      brdend                ;normal return(file end)
    mov     ax,00h                ;set normal return value(not end of file)
    jmp     brdexte               ;normal end
brdend:
    mov     ax,01h                ;set normal return value(end of file)
    jmp     short brdexte

brderr: 
    mov     ax,01h                ;set error    end ax=1

brdexte:
    pop     ds                    ;recover ds
    pop     dx                    ;recover dx
    pop     cx                    ;recover cx
    pop     bx                    ;recover bx

    mov     sp,bp                 ;recover sp
    pop     bp                    ;recover bp
    ret                           ;all done
_bsfread endp

;**********************************************************
;*                                                        *
;*        WORD    bsfwrite(buffer,size,count,hdl)         *
;*                                                        *
;*        read block to I/O buffer                        *
;*                                                        *
;*        BYTE  *buffer;    I/O buffer                    *
;*        WORD   size;      I/O size                      *
;*        WORD   count;     I/O count                     *
;*        WORD   hdl;       file handle                   *
;*                                                        *
;*      f/v    0 ;    normal end                          *
;*             1 ;    error  end                          *
;*                                                        *
;**********************************************************

_bsfwrite proc    far

    push    bp                    ;save bp
    mov     bp,sp                    ;bp-->arg

    push    bx                    ;save bx
    push    cx                    ;save cx
    push    dx                    ;save dx
    push    ds                    ;save ds
    
;        ds/dx : I/O 

    mov     ax,ss:[bp+@ab+4]
    mul     word ptr ss:[bp+@ab+6]    
    mov     cx,ax                ;get I/O length    
    mov     bx,ss:[bp+@ab+8]     ;get files handle
    lds     dx,ss:[bp+@ab]       ;get I/O buffer ptr offset
    mov     ah,40h               ;function code=write
    int     21h                  ;exec wirte

    jc      bwterr               ;error return
    jmp     short bwtextn        ;normal return

bwterr: 
    mov     ax,1                 ;set error    end ax=1
    jmp     short bwtexte        ;error end
bwtextn:
    mov     ax,0                 ;set normal end ax=0    
bwtexte:
    pop     ds                   ;recover ds
    pop     dx                   ;recover dx
    pop     cx                   ;recover cx
    pop     bx                   ;recover bx

    mov     sp,bp                ;recover sp
    pop     bp                   ;recover bp
    ret                          ;all done
_bsfwrite endp

;**********************************************************
;*                                                        *
;*        WORD    bsfseek(hdl,offset,orign)               *
;*                                                        *
;*        read block to I/O buffer                        *
;*                                                        *
;*        WORD  hdl;         file handle                  *
;*        LONG  offset;     seek offset                   *
;*        WORD  orign;     seek orign                     *
;*                                                        *
;*      f/v    0 ;    normal end                          *
;*             1 ;    error  end                          *
;*                                                        *
;**********************************************************

_bsfseek proc  far

    push    bp                    ;save bp
    mov     bp,sp                 ;bp-->arg

    push    bx                    ;save bx
    push    cx                    ;save cx
    push    dx                    ;save dx

;        cx/dx : set I/O location

    mov     dx,ss:[bp+@ab+2]    ;get I/O position lower value
    mov     cx,ss:[bp+@ab+4]    ;get I/O position upper value
    mov     bx,ss:[bp+@ab]      ;get files handle
    mov     ax,ss:[bp+@ab+6]    ;get seek orign
    mov     ah,42h              ;function=lseek
    int     21h                 ;exec lseek
    jc      bskerr              ;if zero error
    jmp     short bskextn       ;normal return
bskerr: 
    mov     ax,1                ;set error    end ax=1
    jmp     short bskexte       ;error end
bskextn:
    mov     ax,0                ;set normal end ax=0    
bskexte:
    pop     dx                  ;recover dx
    pop     cx                  ;recover cx
    pop     bx                  ;recover bx

    mov     sp,bp               ;recover sp
    pop     bp                  ;recover bp
    ret                         ;all done
_bsfseek endp

;**********************************************************
;*                                                        *
;*        WORD    bsftell(hdl)                            *
;*                                                        *
;*                                                        *
;*        WORD  hdl;         file handle                  *
;*                                                        *
;*      f/v     ax = size ;    normal end                 *
;*                      1 ;    error  end                 *
;*                                                        *
;**********************************************************

_bsftell proc  far

    push    bp                    ;save bp
    mov     bp,sp                 ;bp-->arg
    push    bx
    push    cx
    mov     bx,ss:[bp+@ab]        ;get file handle
    mov     cx,00h                ;seek 0byte from offset
    mov     dx,00h                ;
    mov     al,02h                ;set file end
    mov     ah,42h                ;function = lseek
    int     21h                   ;
    jnc     btell                 ;normal end
    mov     ax,00h                ;set error value
btell:
    pop     cx                    ;recover cx
    pop     bx                    ;recover bx

    mov     sp,bp                 ;recover sp
    pop     bp                    ;recover bp
    ret                           ;all done
_bsftell endp

    include        epilogul.h
    end
