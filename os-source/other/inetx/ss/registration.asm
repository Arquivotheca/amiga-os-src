; ======================================================================
; SSRegister - register your process with the library. This takes an 
;               argument that is a pointer to a 32-bit variable. 
;
;  possible problem with this. If a program fails to call CloseLibrary
;  on this (fails to get "unregistered") and then another program is 
;  quickly loaded into that same memory space and IT registers with the
;  same pid...   This should be coded so that if a caller tries to 
;  registersuch that an attempt to register
;  a new caller with a pid that matches an existing table entry
;
;  A0: pid
;
;       returns 0 on failure, 1 on success
; ======================================================================
;


SSRegister:    ; A0:   error_addrress    A6: sslib

        movem.l a2/a3/a5/a6,-(sp)          ; save stuff
        move.l  a0,a3                   ; preserve error addr
        move.l  a6,a5                   ; a5 = sslib base
        move.l  ss_RegBlock(a5),a2      ; a2 = careful - CHANGES!
        move.l  #BLOCKSIZE,d1           ; d1 = # of LONGS in array 
                                        ;      (CHANGES!)

            ; scan the array twice. First we look for a match of
            ; the caller's PID. If we find that, we replace the
            ; existing entry with the new one. If we don't find one 
            ; then we look for an empty spot to stick this new entry
            ; into. Insert the pid and the error variable's address 
            ; into the array if a spot is found. 
            ;
            ; Return 1 on success, 0 (zero) on error.
            
            ; get caller's PID

        move.l  ss_SysLib(a5),a6        ; get exec base
        move.l  #0,a1                   ; FindTask(0)
        CALLSYS FindTask                ; d0 = pid
        
scan1:      ; see if there is already a pid in the array that
            ; matches ours
                                        ; a2 now = ptr to 1st entry
                                        ; d0 holds pid of caller.
        cmp.l  0(a2),d0                 ; open slot in the array?
        beq     addit                   ; yes, so add new reg.
        addq.l  #8,a2                   ; next entry to check
        subq.l  #2,d1                   ; decrement counter
        bne     scan1
            ; else fall through to search for empty entries

            ; re-init these register values for loop #2
        move.l  ss_RegBlock(a5),a2      ; a2 = careful, we change this!!
        move.l  #BLOCKSIZE,d1           ; d1 = # of LONGS in array

scan2:      ; if we got here then there is no matching pid so
            ; now we look for am empty spot in the array.
            
                                        ; a2 now = ptr to 1st entry
                                        ; d0 holds pid of caller.
        cmp.l   #0,(a2)                 ; open slot in the array?
        beq     addit                   ; yes, so add new reg.
        addq.l  #8,a2                   ; next entry to check
        subq.l  #2,d1                   ; decrement counter
        bne     scan2                   ; loop end 

        moveq.l #0,d0                   ; error return
        bra     cleanup


addit:      ; stick the pid and the error variable pointer.
        move.l  d0,(a2)                 ; put PID into reg array
        move.l  a3,4(a2)                ; errno into array
        moveq.l #1,d0                   ; successful return

cleanup:
        movem.l (sp)+,a2/a3/a5/a6
        rts

        ds.w    0                       ; word alignment


; ---------------------------------------------------------------
; __SSClearEntry        d0: pid of entry to clear
;
;  *** INTERNAL FUNCTION    INTERNAL FUNCTION ***
;
;     trashes nothing
;
; as currently written this MUST handle being passed a NULL
; This has no return value of any meaning. If it fails, not
; much you can do about it.
; ----------------------------------------------------------

__SSClearEntry:     ; ( no args.  D0:  PID of caller *_or_* NULL)
                    ; ( A6: sysbase   A5: sslib base

        movem.l d0-d7/a0-a7,-(sp)       ; save these!
        
            ; get caller's PID
        move.l  ss_SysLib(a5),a6        ; a6 = execbase
        move.l  #0,a1                   ; FindTask(0)
        CALLSYS FindTask                ; d0 = pid
        move.l  ss_RegBlock(a5),a2      ; a2 = careful!
        move.l  #BLOCKSIZE,d1           ; d1 = # of LONGS in array (!)
                                        ; **   a2 and d1 *change*  **

l5:         ; see if there is already a pid in the array that
            ; matches ours
            
        cmp.l  0(a2),d0                 ; open slot in the array?
        beq     add_it                  ; yes, so add new reg.
        addq.l  #8,a2                   ; next entry to check
        subq.l  #2,d1                   ; decrement counter
        bne     l5
        bra     endit

add_it:     ; clear the pid and error address entries
        clr.l   (a2)                    ; clear pid field
        clr.l   4(a2)                   ; clear err_addr field
endit:
        movem.l (sp)+,d0-d7/a0-a7       ; leave things as we found them
        rts


;-------------------------------------------------------------
; _FindEntry.c			ss.library
;
; returns a pointer to the entry's stored error info_structure
; if successsful, NULL if failed.
;
;------------------------------------------------------------
;

; #include "sslib_def.i

;ULONG *_FindEntry( struct Process *p) ;
;
;#pragma regcall(_FindEntry(D0))
;

ssbase		EQUR	a2
regblock	EQUR	d4
entryptr	EQUR	a3
tmp			EQUR	d0
ptmp		EQUR	a0
blocksize	EQUR	d5
pid			EQUR	d6

	xdef	_FindEntry
	
_FindEntry:
	
	movem.l	d2-d7/a0-a7,-(sp)
	move.l	d0,pid					; save passed pid
	
		; get set up
	move.l	a6,ssbase				; get sslib ptr
	move.l	ss_RegBlock(ssbase),regblock	; get ptr to regblock
	move.l	ss_BlockSize(ssbase),blocksize	; get blocksize

		; ss_BlockSize holds a value in BYTES. We need entry numbers.
		; since each entry is 8 bytes, we divide by 8
	lsr.l	#3,blocksize			; divide block size by 8
	sub.l	#1,blocksize			; decrement by one
									;    test 0 - (blocksize -1)

	move.l	regblock,ptmp			; get regblock ptr into work space
	move.l	pid,tmp					; get pid into tmp reg

loop:
		; try and match pid to entry
	cmp.l	(ptmp),pid				; entry = pid ?
	beq		foundit					; match, so return err entry ptr

		; counter still within range?

	addq.l  #8,ptmp					; next entry
	dbmi    blocksize,loop			; loop until bs == -1

		;	no. so return NULL
	move.l	#$20202,d0					; yes, it's zero, so return NULL
	bra		out

foundit:
	move.l	ptmp,d0					; get entry into d0
	addq.l	#4,d0					; return ptr to pid's ssinfo 
	                                ;              struct entry

out:
	movem.l	(sp)+,d2-d7/a0-a7
	rts
