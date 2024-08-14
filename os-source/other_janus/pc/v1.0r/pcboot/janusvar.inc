;******************************************************************************
;
; janusvar.inc	  Software data structures for the janus board
; ------------
;		  It is used to keep track of memory used between
;		  the 8088 and the 68000. This structure will be filled
;		  from AMIGA before starting the PC.
;
;
; Table of janus memory offsets:
;					       Define as   filled   Offset
;							    from	
; JanusBase ->	jb_Lock 			    db	    A/PC      0h
;		jb_pad0 			    db	    A/PC      1h
;		jb_ParamMem ->	jpm_Lock	    db	    A/PC      2h
;				jpm_pad0	    db	     -	      3h
;				jpm_68000Base	    dd2      A	      4h
;						      3      A 
;						      0      A 
;						      1      A 
;				jpm_8088Segment     dw0      PC       8h
;						      1      PC    
;				jpm_First	    dw0      A	      Ah
;						      1      A 
;				jpm_Max 	    dw0      A	      Ch
;						      1      A	 
;				jpm_Free	    dw0      A	      Eh
;						      2      A 
;		jb_BufferMem -> jbm_Lock	    db	    A/CP     10h
;				jbm_pad0	    db	     -	     11h
;				jbm_68000Base	    dd2      A	     12h
;						      3      A 
;						      0      A	
;						      1      A	
;				jbm_8088Segment     dw0      PC      16h
;						      1      PC
;				jbm_First	    dw0      A	     18h
;						      1      A 
;				jbm_Max 	    dw0      A	     1ah
;						      1      A	
;				jbm_Free	    dw0      A	     1ch
;						      1      A	
;		jb_Interrupts			    dw0      A	     1eh
;						      1      A	       
;		jb_Parameters			    dw0      A	     20h
;						      1      A	  
;		jb_NumInterrupt 		    dw0      A	     22h
;						      1      A	
; JanusBase SIZE = 24h
;
; New code : 28-Feb-86 TB
; Update   : 
;
;*****************************************************************************
.xlist
;
JanusBase      struc		   ; (fixed at 0f000:0h)

jb_Lock 	    db	 0	   ; Bit7 = '1': locked from AMIGA /
jb_pad0 	    db	 0	   ;	Bit6 = '1': locked from PC

;----------------------------------  JanusMemHead of ParamMem
jpm_Lock	    db	 0	   ; Bit7 = '1': locked from AMIGA /
jpm_pad0	    db	 0	   ;	Bit6 = '1': locked from PC
jpm_68000Base	    dd	 0	   ; base address of ParamMem from AMIGA side
jpm_8088Segment     dw	 0	   ; segment of ParamMem from PC side
jpm_First	    dw	 0	   ; pointer to first free mem chunk
jpm_Max 	    dw	 0	   ; max address for this mem type
jpm_Free	    dw	 0	   ; count of free bytes

;----------------------------------  JanusMemHead of BufferMem
jbm_Lock	    db	 0	   ; Bit7 = '1': locked from AMIGA /
jbm_pad0	    db	 0	   ;	Bit6 = '1': locked from PC
jbm_68000Base	    dd	 0	   ; base address of BufferMem from AMIGA side
jbm_8088Segment     dw	 0	   ; segment of BufferMem from PC side
jbm_First	    dw	 0	   ; pointer to first free mem chunk
jbm_Max 	    dw	 0	   ; max address for this mem type
jbm_Free	    dw	 0	   ; count of free bytes

;----------------------------------
jb_Interrupts	    dw	 0	   ; pointer to interrupt requests 
jb_Parameters	    dw	 0	   ; pointer to interrupt parameter blokcks
jb_NumInterrupts    dw	 0	   ; # of requested interrupt
 
JanusBase      ends  


JanusMemChunk  struc

jmc_Next	    dw	 0	   ; pointer to next mem chunk 
jmc_Size	    dw	 0	   ; size of current mem chunk

JanusMemChunk  ends 

.list

