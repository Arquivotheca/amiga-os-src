
* ****************************************************************************
*
* Graphics Blaster Routines for the Zaphod Project
* 
* Copyright (C) 1986, Commodore Amiga, Inc.
*
* CONFIDENTIAL and PROPRIETARY
*
* HISTORY	Name		Description
* ------------	--------------	----------------------------------------------
* 23 Feb 86	=RJ Mical=	Created this file
*
* ****************************************************************************
	INCLUDE "zaphod.i"
			  

	XDEF   _BlastGraphicsPlanes



_BlastGraphicsPlanes:
*******************************************************************************
* To get here, there's been a PC Write to the Graphics PCDisplay
*
* ON ENTRY:
*	- Address of display structure is on the stack
*
* Blast the data from the PCDisplay into the ZaphodBitMaps
							  
ARGS	EQU	((11+1)*4)
	MOVEM.L A2-A6/D2-D7,-(SP)  ; Yep, save the lot, we'll use 'em all!

			   
	MOVE.L	ARGS(SP),A6	   ; Get address of Display structure

	MOVE.W	Modes(A0),D0	   ; Test if the mode is medium or high res
	ANDI.W	#MEDIUM_RES,D0
	BEQ	HIGHRES
	PUTMSG	50,<'Doing a medium res data transfer'>

*****************************************************************************
* Medium Resolution Transfer
*
* We're currently in medium res APA mode.  Rather than having the bits 
* interleaved as they are normally in memory, the Zaphod Graphics Interface 
* rearranges the bits of each word to have all plane 0 bits in the first 
* byte and all plane 1 bits in the second byte, thereby making it easy to 
* transfer the data into Amiga bit planes.  This is a *great* boon from
* the programmer's point of view.  Think of the work that would be required
* without this feature!
*
* To transfer all data from the color display memory, pick up alternating
* bytes from the even lines of the display memory and lay them down in the 
* even lines of the bit planes.  Then do the same for the odd lines.
*		  
* For the pass:
*	A0 = plane 0, row 0
*	A1 = plane 1, row 0
*	A2 = start of PCColorDisplay even lines    
*	A3 = plane 0, row 1
*	A4 = plane 1, row 1 
*	A5 = start of PCColorDisplay odd lines	  

	LEA	BufferBitMap+bm_Planes(A6),A0  ; Get the address of plane 0
	MOVE.L	(A0),A0
	LEA	40(A0),A3	      ; and point A3 to the first odd line  

	LEA	BufferBitMap+bm_Planes+4(A6),A1  ; Get the address of plane 1
	MOVE.L	(A1),A1
	LEA	40(A1),A4	      ; and point A4 to the first odd line 


	MOVE.L	PCDisplay(A6),A2      ; Get address of even PC lines
	LEA	8192(A2),A5	      ; and point A5 at the odd lines

     EXG  A0,A1     ; ??? Someone's not thinking right here, and I'm not
     EXG  A3,A4     ; sure if it's me or Frank

	MOVEQ	#99,D1		; Loop on 100 lines (even, then odd)

1$	MOVEQ	#39,D2		; Transfer 40 words per line

2$	MOVE.B	(A2)+,(A0)+	; Transfer the next byte into plane 0 even
	MOVE.B	(A2)+,(A1)+	; Transfer the next byte into plane 1 even
 
	MOVE.B	(A5)+,(A3)+	; Transfer the next byte into plane 0 odd
	MOVE.B	(A5)+,(A4)+	; Transfer the next byte into plane 1 odd

	DBRA	D2,2$		; Loop until this line is copied

	LEA	40(A0),A0	; Skip the next plane lines
	LEA	40(A1),A1
	LEA	40(A3),A3
	LEA	40(A4),A4
	DBRA	D1,1$		; And loop until all lines are copied
	    
	BRA	RETURN	   
			


HIGHRES:
*****************************************************************************
* High Resolution Transfer
*
* We're currently in high res APA mode which is only one plane deep, with
* 80 bytes worth of pixels per line.  Transfer the data into Amiga bit planes.
*
* To transfer all data from the color display memory, pick up alternating
* bytes from the even lines of the display memory and lay them down in the 
* even lines of the bit plane.	Then do the same for the odd lines.

	PUTMSG	50,<'Doing a high res transfer'>

	MOVE.L	PCDisplay(A6),A0	; Get address of even lines
	LEA	8192(A0),A2		; and the odd PC lines
  

	LEA	BufferBitMap+bm_Planes(A6),A1  ; Get the address of plane 0
	MOVE.L	(A1),A1
	LEA	80(A1),A3		; and point A3 to the first odd line  

	MOVEQ	#99,D0			; Do the loop 100 lines
							  
* Now, move the even line's 80 bytes as two passes of 10 longs per pass.
1$	MOVEM.L (A0)+,A4-A6/D1-D7	; Get 10 longs from the map
	MOVEM.L A4-A6/D1-D7,(A1)	; Write 10 longs to the plane
	LEA	40(A1),A1		; Bump the plane pointer

	MOVEM.L (A0)+,A4-A6/D1-D7	; Get the next 10 longs
	MOVEM.L A4-A6/D1-D7,(A1)
	LEA	40+80(A1),A1		; Plus 80 to skip the odd line
  
* Move the odd line's 80 bytes as two passes of 10 longs per pass.
	MOVEM.L (A2)+,A4-A6/D1-D7	; Get 10 longs from the map
	MOVEM.L A4-A6/D1-D7,(A3)	; Write 10 longs to the plane
	LEA	40(A3),A3		; Bump the plane pointer
  
	MOVEM.L (A2)+,A4-A6/D1-D7	; Get the next 10 longs
	MOVEM.L A4-A6/D1-D7,(A3)
	LEA	40+80(A3),A3		; Plus 80 to skip the even line
  
	DBRA	D0,1$			; Loop for all the lines
		      
RETURN:
	MOVEM.L (SP)+,A2-A6/D2-D7  
	RTS




	END


