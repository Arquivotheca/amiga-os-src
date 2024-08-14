
* *** alloc2.asm *************************************************************
* 
* Extra Janus Memory Allocation Routines
* 
* Copyright (C) 1988, Commodore-Amiga Inc.
* 
* CONFIDENTIAL and PROPRIETARY
* 
* Date       Name               Description
* ---------  -----------------  -----------------------------------------------
* 15-Jul-88   -RJ               Changed all files to work with new includes
* 19-Feb-88  -RJ Mical-         Created this file!
* 
* *****************************************************************************


	INCLUDE "assembly.i"


	NOLIST
	INCLUDE "exec/types.i"

	INCLUDE "janus/janusbase.i"
	INCLUDE "janus/memory.i"
	INCLUDE "janus/services.i"

	INCLUDE "asmsupp.i"
	LIST


	XLIB	AllocJanusMem
	XLIB	AllocJRemember
	XLIB	FreeJanusMem
	XLIB	JanusOffsetToMem
	XLIB	JanusMemToOffset
	XLIB	JanusMemType
	XLIB	MakeWordPtr


	XDEF	AllocJRemember
	XDEF	AllocServiceMem
	XDEF	AttachJRemember
	XDEF	FreeJRemember
	XDEF	FreeServiceMem


AllocJRemember:
* === AllocJRemember() ========================================================
* 
* NAME
*     AllocJRemember  --  Allocate Janus memory and link into a Remember list
* 
* SYNOPSIS
*     MemPtr = AllocJRemember(JRememberKey, Size,   Types)
*     D0,A0                   A0            D0:0-15 D1:0-15
* 
*         APTR    MemPtr;
*         struct  JanusRemember **JRememberKey;
*         USHORT  Size;
*         USHORT  Types;
*     From assembly, expects JanusBase in A6
* 
* 
* FUNCTION
*     This routine gets memory for you by calling the Janus library's 
*     AllocJanusMem() routine, but also adds the details of the allocation 
*     into a master list so that you make just one call the FreeJRemember() 
*     routine at a later time to free all the memory you allocated.  
*     You don't have to remember the details of all of your allocations.  
*     This allows you to allocate memory dynamically, as needed, without 
*     requiring you to undergo the bookkeeping required to free at a 
*     later time every last byte that you allocated.  This also allows you 
*     to fail part way through a series of allocations and almost 
*     effortlessly free up any memory you had already allocated.  
* 
*     Once you get used to using this type of memory allocation, you 
*     never go back.  
*     
*     You create the "anchor" for the allocation master list by 
*     declaring a variable to be a pointer to a JanusRemember 
*     structure.  You initialize that pointer to NULL.  
*     This pointer is called the JRememberKey, and is used by calls 
*     to AllocJRemember() and FreeJRemember().  The address of your 
*     pointer is passed to both AllocJRemember() and FreeJRemember().
*     
*     The Size and Type arguments are the same as those passed to the 
*     AllocJanusMem() function.  Please refer to the AllocJanusMem() 
*     documentation for a description of these arguments.  
*     
*     If the call succeeds, you are returned a pointer to the Janus 
*     memory that you desire.  If the call fails, a NULL is returned.  
*     Assembly programmers note that the result is returned in both 
*     registers D0 and A0.  
* 
*     If you have JRememberKeys from two separate calls to AllocJRemember(), 
*     you can merge one key into the other using the AttachJRemember() 
*     routine.  
*     
*     This routine performs the analogy of the Intuition AllocRemember() 
*     call, except that it works with the special Janus memory.  For a 
*     complete explanation of the need for and use of Remember-style 
*     memory allocation and deallocation, see the Intuition reference 
*     manual.  For a quick overview of Intuition memory, see 
*     illustration 11.1 of the Intuition manual.  
*     
*     
* EXAMPLE
*     struct JanusRemember *JRememberKey;
*         JRememberKey = NULL;
*         if (ptr = AllocJRemember(&JRememberKey, BUFSIZE, MEMF_BUFFER))
*             {
*             /* Use the ptr memory */
*             }
*         FreeJRemember(&JRememberKey, TRUE);
*     
*     
* INPUTS
*     JRememberKey = address of a pointer to JanusRemember structures.
*         Before your very first call to AllocJRemember(), the pointer 
*         should be set to NULL.  After that, the Janus library will 
*         manage the contents of the pointer for you
*     Size = the size in bytes of the memory allocation.  Please refer 
*         to the AllocJanusMem() documentation for details about this field
*     Type = the type of the desired memory.  Please refer to the 
*         AllocJanusMem() documentation for details about this field
*     
*     
* RESULTS
*     MemPtr = a pointer to the Janus memory you've requested, or NULL 
*         if no memory was available.  Assembly programmers note that 
*         the result is returned in both registers D0 and A0
*     
*     
* SEE ALSO
*     intuition.library/AllocRemember(), AllocJanusMem(), AttachJRemember(), 
*     FreeJRemember(), AllocServiceMem()

AJR_REGS	REG	D3-D4/A2-A4

		MOVEM.L	AJR_REGS,-(SP)

	IFGE	INFOLEVEL-1
	MOVEM.L	D0-D1,-(SP)
	MOVEM.L	A0,-(SP)
	PUTMSG	1,<'%s/AllocJRemember( $%lx %ld $%04lx )'>
	LEA	3*4(SP),SP
	ENDC

		MOVE.L	#0,A2		; Set the Remember pointer to NULL
		MOVE.L	A2,A3		; Set the memory pointer to NULL

		MOVE.L	A0,A4		; Grab copies of our args
		MOVEQ.L	#0,D3
		MOVE.L	D3,D4
		MOVE.W	D0,D3
		MOVE.W	D1,D4

		;------	Allocate the memory for the JanusRemember structure
		MOVE.L	#JanusRemember_SIZEOF,D0
		MOVE.L	#MEMF_PARAMETER+MEM_WORDACCESS,D1
		CALLSYS	AllocJanusMem
		TST.L	D0
		BEQ	ajrError
		MOVE.L	D0,A2

		;------	Allocate the caller's memory
		MOVE.L	D3,D0		; Get the user's size
		MOVE.L	D4,D1		; Get the user's type 
		CALLSYS	AllocJanusMem
		TST.L	D0
		BEQ	ajrError
		MOVE.L	D0,A3

		;------	Fill out our new Remember structure
		CALLSYS	JanusMemToOffset
		MOVE.W	D0,jrm_Offset(A2)
		MOVE.W	D3,jrm_Size(A2)		; Get the user's size
		MOVE.W	D4,jrm_Type(A2)		; Get the user's type 

		;------	Link our Remember structure into the key 
		MOVE.L	(A4),D0
		BEQ	10$			; if NULL, it's the first
		CALLSYS	JanusMemToOffset
		BRA	20$
10$		MOVE.W	#-1,D0
20$		MOVE.W	D0,jrm_NextRemember(A2)
		MOVE.L	A2,(A4)

		BRA	ajrRTS

ajrError:	;------	We failed, so deallocate anything we allocated
		CMPA.L	#0,A2
		BEQ	10$
		MOVE.L	A2,A1
		MOVE.L	#JanusRemember_SIZEOF,D0
		CALLSYS	FreeJanusMem
		MOVE.W	#0,A2
10$
		CMPA.L	#0,A3
		BEQ	20$
		MOVE.L	A3,A1
		MOVE.L	D3,D0
		CALLSYS	FreeJanusMem
		MOVE.W	#0,A3
20$

ajrRTS:		MOVE.L	A3,D0
		MOVE.L	A3,A0

		MOVEM.L	(SP)+,AJR_REGS
		RTS



AttachJRemember:
* === AttachJRemember() ========================================================
* 
* NAME
*     AttachJRemember  --  Attach the list of one Janus memory key to another  
* 
* SYNOPSIS
*     VOID AttachJRemember(ToKey, FromKey);
*                          A0     A1
*         struct  JanusRemember **ToKey;
*         struct  JanusRemember **FromKey;
*     From assembly, expects JanusBase in A6
* 
* 
* FUNCTION
*     This routine accepts two Janus RememberKeys created by calls to 
*     AllocJRemember() and attaches the contents of the FromKey to 
*     the ToKey.  In the process, FromKey is set equal to NULL.  
*     The result is that ToKey is comprised of a list of all the 
*     memory allocations, while FromKey points, correctly, to no list.  
*     
*     If either key points to NULL, as the result, for instance, of a 
*     failed call to AllocJRemember(), no problem.  This routine detects 
*     those cases and handles them appropriately.  
*     
*     A common use for this type of routine (also outlined below under EXAMPLE):
*         - make a series of local memory allocations using AllocJRemember() 
*         - if any allocations fail, abandon ship by calling FreeJRemember() 
*           to free up the local list
*         - if all allocations succeed use AttachJRemember() to add the list 
*           to a global list of allocations, which is freed in the end 
*           using FreeJRemember() when your program is terminating
*     
*     
* EXAMPLE
*     struct JanusRemember *GlobalJKey = NULL;
*     
*         exampleAllocJ(&GlobalJKey);
*         exampleAllocJ(&GlobalJKey);
*         exampleAllocJ(&GlobalJKey);
*         FreeJRemember(&GlobalJKey, TRUE);
*     
*     exampleAllocJ(globalkey)
*     struct JanusRemember **globalkey;
*     {
*         BOOL success;
*         struct JanusRemember *localkey;
*     
*         success = FALSE;
*         localkey = NULL;
*     
*         if (AllocJRemember(&localkey, 10000, MEMF_BUFFER | MEM_WORDACCESS))
*           if (AllocJRemember(&localkey, 100, MEMF_BUFFER | MEM_WORDACCESS))
*             if (AllocJRemember(&localkey, 1, MEMF_BUFFER | MEM_WORDACCESS))
*               success = TRUE;
*     
*         if (success) AttachJRemember(globalkey, &localkey);
*         else FreeJRemember(&localkey, TRUE);
*     }
*     
*     
* INPUTS
*     ToKey = address of a pointer to JanusRemember structures, which 
*         pointer is going to receive the list pointed to by FromKey
*     FromKey = address of a pointer to JanusRemember structures, which 
*         pointer has the list that's going to be attached ToKey, 
*         after which the FromKey variable will be set to NULL
*     
*     
* RESULTS
*     None
*     
*     
* SEE ALSO
*     intuition.library/AllocRemember(), AllocJanusMem(), AllocJRemember(), 
*     FreeJRemember()

ATR_REGS	REG	A2-A3

		MOVEM.L	ATR_REGS,-(SP)

	IFGE	INFOLEVEL-1
	MOVEM.L	A0-A1,-(SP)
	PUTMSG	1,<'%s/AttachJRemember( $%lx $%lx )'>
	LEA	2*4(SP),SP
	ENDC

		;------	Grab safe word-access copies of our args
		MOVEA.L	A0,A2
		MOVEA.L	A1,A3

		;------	If ToKey is NULL, just attach FromKey directly 
		MOVE.L	(A2),A0
		CMPA.L	#0,A0
		BNE	atrFindEnd
		MOVE.L	(A3),(A2)
		BRA	atrDone

atrFindEnd:
		;------	Find the end of the ToKey list 
		MOVE.W	jrm_NextRemember(A0),D0
		CMP.W	#-1,D0
		BEQ	atrEndFound
		MOVE.W	#MEMF_PARAMETER+MEM_WORDACCESS,D1
		CALLSYS	JanusOffsetToMem
		BRA	atrFindEnd

atrEndFound
		;------	Attach FromKey to the end of ToKey
		MOVE.L	A0,A2
		MOVE.L	(A3),D0
		CALLSYS	JanusMemToOffset
		MOVE.W	D0,jrm_NextRemember(A2)

atrDone:	MOVE.L	#0,(A3)
		MOVEM.L	(SP)+,ATR_REGS
		RTS



FreeJRemember:
* === FreeJRemember() =========================================================
* 
* NAME
*     FreeJRemember  --  Free memory allocated by calls to AllocJRemember()
*
* SYNOPSIS
*     VOID FreeJRemember(JRememberKey, ReallyForget)
*                        A0            D0:0-15
*         struct  JanusRemember **JRememberKey;
*         BOOL    ReallyForget;
*     From assembly, expects JanusBase in A6
* 
* 
* FUNCTION
*     This function frees up Janus memory allocated by the AllocJRemember() 
*     function.  It will either free up just the JanusRemember structures, 
*     which supply the link nodes that tie your allocations together, 
*     or it will deallocate both the link nodes AND your memory buffers too.
*
*     If you want to deallocate just the JanusRemember structure link nodes,
*     you should set the ReallyForget argument to FALSE.  However, if you
*     want FreeJRemember() to really forget about all the memory, including
*     both the JanusRemember structure link nodes and the buffers allocated 
*     with earlier calls to AllocJRemember(), then you should set the 
*     ReallyForget argument to TRUE.
* 
*     Note that this routine sets your JRememberKey variable to NULL.  
* 
*
* EXAMPLE
*     struct JanusRemember *JRememberKey;
*         JRememberKey = NULL;
*         while (AllocJRemember(&JRememberKey, BUFSIZE, MEMF_BUFFER)) ;
*         FreeJRemember(&JRememberKey, TRUE);
*     
*     
* INPUTS
*     JRememberKey = the address of a pointer to struct JanusRemember.  This
*          pointer should either be NULL or set to some value (possibly
*          NULL) by a call to AllocRemember()
*     ReallyForget = a BOOL FALSE or TRUE describing, respectively, 
*          whether you want to free up only the Remember nodes or 
*          if you want this procedure to really forget about all of 
*          the memory, including both the nodes and the memory buffers 
*          pointed to by the nodes.
*
* RESULT
*     None
*
* SEE ALSO
*     AllocJRemember(), intuition.library/FreeRemember()

FJR_REGS	REG	D2-D3/A2

		MOVEM.L	FJR_REGS,-(SP)

	IFGE	INFOLEVEL-1
	MOVEM.L	D0,-(SP)
	MOVEM.L	A0,-(SP)
	PUTMSG	1,<'%s/FreeJRemember( $%lx %ld )'>
	LEA	2*4(SP),SP
	ENDC


		;------	Get the address of the first JanusRemember struct
		;------	and zero out the caller's pointer to the first
		MOVE.L	(A0),A2
		MOVE.L	#0,(A0)

		;------	If the address of the first struct is NULL, we're done
		CMPA.L	#0,A2
		BEQ	fjrDone

		;------	Grab a copy of the ReallyForget argument
		MOVE.W	D0,D2


fjrLoop:	;------	Grab a copy of the offset to the next structure 
		;------	before we free up the memory of this structure
		MOVE.W	jrm_NextRemember(A2),D3


		;------	If the user wants to really forget all memory, 
		;------	free the memory as well as the JanusRemember structure
		TST.W	D2		; Test the ReallyForget arg for zero
		BEQ	10$		; Zero means "don't really forget"
		MOVE.W	jrm_Offset(A2),D0 ; Get the memory offset 
		MOVEQ.L	#0,D1		; (after preparing D1 for a word)
		MOVE.W	jrm_Type(A2),D1	; and memory type
		CALLSYS	JanusOffsetToMem ; and turn it into a pointer
		MOVE.L	D0,A1		; which
		MOVEQ.L	#0,D0		; (after preparing D0 for a word)
		MOVE.W	jrm_Size(A2),D0	; when combined with the size
		CALLSYS	FreeJanusMem	; can now be freed
10$

		;------	Free the Remember structure
		MOVE.L	A2,A1
		MOVE.L	#JanusRemember_SIZEOF,D0
		CALLSYS	FreeJanusMem


		;------	Any more to do?  If the offset to the next is -1, 
		;------	then there's no more to do.
		CMP.W	#-1,D3
		BEQ	fjrDone
		MOVE.W	D3,D0
		MOVE.L	#MEMF_PARAMETER+MEM_WORDACCESS,D1
		CALLSYS	JanusOffsetToMem
		MOVE.L	D0,A2
		BRA	fjrLoop


fjrDone:	MOVEM.L	(SP)+,FJR_REGS
		RTS



AllocServiceMem:
* === AllocServiceMem() =======================================================
* 
* NAME
*     AllocServiceMem  --  Allocate Janus memory linked to a ServiceData struct
* 
* SYNOPSIS
*     MemPtr = AllocServiceMem(ServiceData, Size,   Types)
*     D0,A0                    A0           D0:0-15 D1:0-15
* 
*         APTR    MemPtr;
*         struct  ServiceData *ServiceData;
*         USHORT  Size;
*         USHORT  Types;
*     From assembly, expects JanusBase in A6
* 
* 
* FUNCTION
*     This routine allocates memory for you and records the details of 
*     the allocation in the specified ServiceData structure, which memory, 
*     unless you free it explicitly with a call to FreeServiceMem(), 
*     will be automatically freed when the service is deleted and 
*     removed from the system.  
*     
*     This routine calls the Janus library's AllocJRemember() call for you, 
*     using the ServiceData's JRememberKey field to record the parameters 
*     of the allocation for you.  You may then free the memory using 
*     the FreeServiceMem() routine.  Alternatively, you may pay no 
*     further attention to this memory allocation because after the 
*     service is deleted and all users have released it, any memory 
*     allocated using the ServiceData's JRememberKey will be freed 
*     using the FreeJRemember() function.  
*     
*     You are allowed to call this routine whether you have acquired the 
*     ServiceData address from AddService() or GetService().  
*     
*     The ServiceData structure pointer that you provide to this routine 
*     doesn't have to point to any particular Janus memory-access address 
*     space (although it must point to Janus memory of course).  What this 
*     means is that if you translate the ServiceData pointer you get from 
*     AddService() or GetService() from word-access to byte-access or 
*     anything else, you don't have to translate it back before calling 
*     AllocServiceMem().  
*     
*     The Size and Type arguments are the same as those passed to the 
*     AllocJanusMem() function.  Please refer to the AllocJanusMem() 
*     documentation for a description of these arguments.  
*     
*     If the call succeeds, you are returned a pointer to the Janus 
*     memory that you desire.  If the call fails, a NULL is returned.  
*     Assembly programmers note that the result is returned in both 
*     registers D0 and A0.  
* 
* 
* EXAMPLE
*     struct ServiceData *SData;
*     if (GetService(&SData, ...) == JSERV_OK))
*         {
*         ptr1 = AllocServiceMem(SData, 100, MEMF_BUFFER | MEM_BYTEACCESS);
*         ptr2 = AllocServiceMem(SData, 100, MEMF_BUFFER | MEM_BYTEACCESS);
*         ReleaseService(SData);
*         SData = NULL;
*         }
*     
*     
* INPUTS
*     ServiceData = address of a ServiceData structure.  This may point to 
*         any type of Janus memory-access address, not necessarily word-access
*     Size = the size in bytes of the memory allocation.  Please refer 
*         to the AllocJanusMem() documentation for details about this field
*     Type = the type of the desired memory.  Please refer to the 
*         AllocJanusMem() documentation for details about this field
*     
*     
* RESULTS
*     MemPtr = a pointer to the Janus memory you've requested, or NULL 
*         if no memory was available.  Assembly programmers note that 
*         the result is returned in both registers D0 and A0
*     
*     
* SEE ALSO
*     AllocJRemember(), FreeServiceMem()

ASM_REGS	REG	A2-A3

	IFGE	INFOLEVEL-1
	MOVEM.L	D0-D1,-(SP)
	MOVEM.L	A0,-(SP)
	PUTMSG	1,<'%s/AllocServiceMem( $%lx %ld $%04lx )'>
	LEA	3*4(SP),SP
	ENDC

		MOVEM.L	ASM_REGS,-(SP)

		;------	Create a storage longword, pointed to by A2
		MOVE.L	SP,A2
		LEA	-4(SP),SP

		;------	Stash the args until the call to AllocJRemember()
		MOVEM.L	D0-D1,-(SP)

		;------	Turn the ServiceData pointer into word-access
		CALLSYS	MakeWordPtr
		MOVE.L	A0,A3		; A3 has the address of ServiceData.WA

		;------	Translate the JRememberKey offset into a pointer
		MOVE.W	sd_JRememberKey(A3),D0
		CMP.W	#-1,D0
		BNE	100$
		MOVEQ.L	#0,D0
		BRA	200$
100$		MOVE.W	#MEMF_PARAMETER+MEM_WORDACCESS,D1
		CALLSYS	JanusOffsetToMem
200$

		;------	Save the pointer into our temporary storage longword
		MOVE.L	D0,(A2)

		;------	Call AllocJRemember() with the address of our
		;------	temporary storage longword
		MOVE.L	A2,A0
		MOVEM.L	(SP)+,D0-D1
		CALLSYS	AllocJRemember

		MOVE.L	D0,-(SP)	; Stash a copy of address

		;------	Translate the new JanusRemember pointer 
		;------	storage back into a Janus offset, and stash the 
		;------	result into the key.  If the new JanusRemember 
		;------	pointer is NULL, stash an $FFFF
		MOVE.L	(A2),D0
		BNE	10$
		MOVE.W	#-1,D0
		BRA	20$
10$		CALLSYS	JanusMemToOffset
20$		MOVE.W	D0,sd_JRememberKey(A3)

		MOVE.L	(SP)+,D0	; Restore MemPtr
		MOVE.L	D0,A0		; return in A0 too

		LEA.L	4(SP),SP	; Free up our temporary longword storage

		MOVEM.L	(SP)+,ASM_REGS
		RTS


FreeServiceMem:
* === FreeServiceMem() ========================================================
* 
* NAME
*     FreeServiceMem  --  Free mem added to a ServiceData by AllocServiceMem()
* 
* SYNOPSIS
*     VOID FreeServiceMem(ServiceData, MemPtr)
*                         A0           A1
*         struct  ServiceData *ServiceData;
*         APTR    MemPtr;
*     From assembly, expects JanusBase in A6
* 
* 
* FUNCTION
*     This routine frees memory that had been allocated with a call 
*     to AllocServiceMem().  You can choose to free a single block of 
*     memory or all the memory of the ServiceData structure.  
*     
*     The ServiceData structure pointer that you provide to this routine 
*     doesn't have to point to any particular Janus memory-access address 
*     space (although it must point to Janus memory of course).  What this 
*     means is that if you translate the ServiceData pointer you get from 
*     AddService() or GetService() from word-access to byte-access or 
*     anything else, you don't have to translate it back before calling 
*     FreeServiceMem().  
*     
*     Note that the address of the ServiceData structure that you supply to 
*     this routine must be the same address that you passed to the associated 
*     AllocServiceMem() call, if any.  
*     
*     The MemPtr argument may be one of two things:
*         - the pointer to memory returned by a call to AllocServiceMem() 
*           (using the same ServiceData structure as the one provided to 
*           this call), or
*         - NULL to designate that you want all (if any) of the ServiceData's 
*           allocated memory to be freed (note that this does not include 
*           freeing the memory allocated at time of AddService()
*     
*     Note that only memory allocated by AllocServiceMem can be freed.  
*     The memory allocated for the service during AddService() cannot be 
*     deleted using FreeServiceMem(), not even if you call FreeServiceMem() 
*     with a MemPtr argument of NULL.  
*     
*     You are allowed to call this routine whether you have acquired the 
*     ServiceData address from AddService() or GetService().  
*     
* 
* EXAMPLE
*     struct ServiceData *SData;
*     if (GetService(&SData, ...) == JSERV_OK))
*         {
*         /* Allocate a bunch of memory */
*         AllocServiceMem(SData, 100, MEMF_BUFFER | MEM_BYTEACCESS);
*         AllocServiceMem(SData, 100, MEMF_BUFFER | MEM_BYTEACCESS);
*         ptr1 = AllocServiceMem(SData, 100, MEMF_BUFFER | MEM_BYTEACCESS);
* 
*         /* Free the last one allocated */
*         FreeServiceMem(SData, ptr1);
* 
*         /* Free all the rest */
*         FreeServiceMem(SData, NULL);
* 
*         ReleaseService(SData);
*         SData = NULL;
*         }
*     
*     
* INPUTS
*     ServiceData = address of a ServiceData structure.  This may point to 
*         any type of Janus memory-access address, not necessarily word-access
*     MemPtr = a pointer to the Janus memory returned by a call 
*         to AllocJanusMem(), or NULL if you want to delete all of the 
*         ServiceData's memory
*     
*     
* RESULTS
*     None
*     
*     
* SEE ALSO
*     AllocServiceMem()

FSM_REGS	REG	D2-D3/A2-A4


	IFGE	INFOLEVEL-1
	MOVEM.L	A0-A1,-(SP)
	PUTMSG	1,<'%s/FreeServiceMem( $%lx $%lx )'>
	LEA	2*4(SP),SP
	ENDC

		MOVEM.L	FSM_REGS,-(SP)

* - below, the phrase "masked-type" refers to taking a Type of Janus 
*   memory and masking it with the NOT of the constant MEM_ACCESSMASK, 
*   like this:  
*   maskedtype = mask & ~MEM_ACCESSMASK

* - if the MemPtr arg is NULL, we will be freeing all
		MOVEA.L	A1,A2

* - else look up this entry in the jremember list, 
*   and if found then unlink and free it.  
* 	- turn the ServiceData pointer into word-access
		CALLSYS	MakeWordPtr
		MOVEA.L	A0,A3		; A3 has the address of ServiceData.WA

* 	- if we're not going to free all memory, 
* 	  turn the MemPtr into an offset and a type with the 
* 	  memory-access address space specifier masked off
		MOVE.L	A2,D0
		BEQ	1555$
		CALLSYS	JanusMemToOffset
		MOVE.W	D0,D2
		MOVE.L	A2,D0
		CALLSYS	JanusMemType
		MOVE.W	D0,D3
		AND.W	#~MEM_ACCESSMASK,D3
1555$

		MOVE.W	sd_JRememberKey(A3),D0
		MOVE.L	#0,A4		; Set our previous ptr to NULL

FSM_LOOP:
* 	- if offset to next JanusRemember structure equals -1, 
* 	  we're done, so goto FSM_DONE
		CMP.W	#-1,D0
		BEQ	FSM_DONE

* 	- else turn the offset into an actual address of a JanusRemember struct
		MOVE.W	#MEMF_PARAMETER+MEM_WORDACCESS,D1
		CALLSYS	JanusOffsetToMem

* 	- if we're freeing all memory or offset and masked-type equal ours then
		CMPA.L	#0,A2			; Are we freeing all?
		BEQ	freeThisOne		; Branch if yes
		CMP.W	jrm_Offset(A0),D2
		BNE	FSM_NEXT		; If offsets not equal, branch
		MOVE.W	jrm_Type(A0),D0
		ANDI.W	#~MEM_ACCESSMASK,D0
		CMP.W	D0,D3			; If masked types aren't equal, 
		BNE	FSM_NEXT		; branch

freeThisOne:
* 			- unlink and delete this one
* 			- goto FSM_DONE
		CMP.L	#0,A4
		BNE	10$
		MOVE.W	jrm_NextRemember(A0),sd_JRememberKey(A3)
		BRA	20$
10$		MOVE.W	jrm_NextRemember(A0),jrm_NextRemember(A4)
20$

		MOVE.L	A0,-(SP)		; Stash JanusRemember.WA

		;------	Free the memory first
		MOVE.W	jrm_Offset(A0),D0
		MOVE.W	jrm_Type(A0),D1
		CALLSYS	JanusOffsetToMem
		MOVE.L	D0,A1
		MOVEQ.L	#0,D0
		MOVE.L	(SP),A0			; Get copy of addr of JRem.WA
		MOVE.W	jrm_Size(A0),D0
		CALLSYS	FreeJanusMem		; (A1 = ptr, D0 = size)

		;------	Free the JanusRemember structure second 
		MOVE.L	(SP),A1			; Get copy of JRem.WA
		MOVE.L	#JanusRemember_SIZEOF,D0
		CALLSYS	FreeJanusMem		; (A1 = ptr, D0 = size)

		MOVE.L	(SP)+,A0		; Restore address of JRem.WA


		;------	Here, we found the memory to free and we've freed 
		;------	it up.  Check whether we got here by matching A2.  
		;------	We check by checking whether A2 has zero or not.  
		;------	If A2 = NULL, we're matching everything so continue.  
		;------	If A2 != NULL, we matched it so our job is done.
		CMPA.L	#0,A2
		BNE	FSM_DONE

FSM_NEXT:
		MOVE.L	A0,A4	; Save copy of ptr as previous
* 	- get offset to next
		MOVE.W	jrm_NextRemember(A0),D0
* 	- goto FSM_LOOP
		BRA	FSM_LOOP


FSM_DONE:
		MOVEM.L	(SP)+,FSM_REGS
		RTS



		END


