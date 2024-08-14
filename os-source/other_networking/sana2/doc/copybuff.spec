/****** any_sana2_protocol/CopyToBuff ***************************************
*
*   NAME
*	CopyToBuff -- Copy n bytes to an abstract data structure.
*
*   SYNOPSIS
*	success = CopyToBuff(to, from, n)
*	d0		     a0  a1    d0
*
*	BOOL CopyToBuff(VOID *, VOID *, ULONG);
*
*   FUNCTION
*	This function first does any initialization and/or allocation
*	required to prepare the abstract data structure pointed at by 'to'
*	to be filled with 'n' bytes of data from 'from'.  It then executes
*	the copy operation.
*
*	If, for example, there is not enough memory available to prepare
*	the abstract data structure, the call is failed and FALSE is returned.
*
*	The buffer management scheme should be such that any memory needed
*	to fulfill CopyToBuff() calls is already allocated from the system
*	before the call to CopyToBuff() is made.
*
*   INPUTS
*	to		- pointer to abstract structure to copy to.
*	from		- pointer to contiguous memory to copy from.
*	n		- number of bytes to copy.
*
*   RESULT
*	success		- TRUE if operation was successful, else FALSE.
*
*   EXAMPLE
*
*   NOTES
*	This function must be callable from interupts.  In particular, this
*	means that this function may not directly or indirectly call any
*	system memory functions (since those functions rely on Forbid() to
*	protect themselves) and that you must not compile this function
*	with stack checking enabled.  See the RKM:Libraries Exec:Interupts
*	chapter for more details on what is legal in a routine called from
*	an interupt handler.
*
*	'C' programmers should not compile with stack checking (option '-v'
*	in SAS) and should geta4() or __saveds.
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/


/****** any_sana2_protocol/CopyFromBuff *************************************
*
*   NAME
*	CopyFromBuff -- Copy n bytes from an abstract data structure.
*
*   SYNOPSIS
*	success = CopyFromBuff(to, from, n)
*	d0		       a0  a1    d0
*
*	BOOL CopyToBuff(VOID *, VOID *, ULONG);
*
*   FUNCTION
*	This function copies 'n' bytes of data in the abstract data structure
*	pointed to by 'from' into the contigous memory pointed to by 'to'.
*	'to' must contain at least 'n' bytes of usable memory or innocent
*	memory will be overwritten.
*
*   INPUTS
*	to		- pointer to contiguous memory to copy to.
*	from		- pointer to abstract structure to copy from.
*	n		- number of bytes to copy.
*
*   RESULT
*	success		- TRUE if operation was successful, else FALSE.
*
*   EXAMPLE
*
*   NOTES
*	This function must be callable from interupts.  In particular, this
*	means that this function may not directly or indirectly call any
*	system memory functions (since those functions rely on Forbid() to
*	protect themselves) and that  you must not compile this function
*	with stack checking enabled.  See the RKM:Libraries Exec:Interupts
*	chapter for more details on what is legal in a routine called from
*	an interupt handler.
*
*	'C' programmers should not compile with stack checking (option '-v'
*	in SAS) and should geta4() or __saveds.
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

/****** any_sana2_protocol/PacketFilter *************************************
*
*   NAME
*	PacketFilter -- Perform filtering operation on CMD_READ's.
*
*   SYNOPSIS
*	keep = PacketFilter(hook, ios2, data)
*	d0		     a0    a2    a1
*
*	BOOL PacketFilter(struct Hook *, struct IOSana2Req *, APTR);
*
*   FUNCTION
*	This function (if supplied by a protocol stack) may be used to
*	reject packets before they are copied into a protocol stack's
*	internal buffers.
*
*	The IOSana2Req structure should be set up to look (almost) exactly
*	as it would if it was successfully returned for the current packet.
*	Specifically, the fields that should be set up correctly are:
*
*	ios2->ios2_DataLength
*	ios2->ios2_SrcAddr
*	ios2->ios2_DstAddr
*
*       The "data" pointer must point to the beginning of the packet data
*	that is stored in contiguous memory.  The data should NOT include
*	any hardware specific headers (unless of course the CMD_READ
*	request wanted RAW packets).
*
*   INPUTS
*	hook		- pointer to the Hook originally supplied during
*			  OpenDevice().
*	ios2		- The IOSana2Req CMD_READ request that will be used
*			  (the "object" of the Hook call).
*	data		- The packet data (the "message" of the Hook call).
*
*   RESULT
*	success		- TRUE if the driver should provide the packet to
*			  the protocol stack, FALSE if the packet should be
*			  ignored.
*
*   EXAMPLE
*
*   NOTES
*	This function must be callable from interupts.  In particular, this
*	means that this function may not directly or indirectly call any
*	system memory functions (since those functions rely on Forbid() to
*	protect themselves) and that  you must not compile this function
*	with stack checking enabled.  See the RKM:Libraries Exec:Interupts
*	chapter for more details on what is legal in a routine called from
*	an interupt handler.
*
*	'C' programmers should not compile with stack checking (option '-v'
*	in SAS) and should geta4() or __saveds.
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
