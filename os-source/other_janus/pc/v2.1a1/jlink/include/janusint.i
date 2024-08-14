;
; PC JANUS Service
;
; This service is called via INT JANUS.
; AH contains a function code
;
;--------------------------------------------------------------------
; J_GET_SERVICE
; =============
;
; Gets a new Service Number
;
; Expects:
;
;	nothing
;
; Returns:
;
;	AL : New Service Number to use
;	     -1 if no service available (J_NO_SERVICE)
;--------------------------------------------------------------------
; J_GET_BASE
; ==========
;
; Gets Segmemts & offset of Janus Memory
;
; Expects:
;
; 	AL :	Janus Service Number
;
; Returns :
;
; 	ES : 	Janus Parameter Segment
; 	DI : 	Janus Parameter Offset (if defined),
; 		else -1
; 	DX :	Janus Buffer Segment
; 	AL :	Status (J_OK, J_NO_SERVICE)
;--------------------------------------------------------------------
; J_ALLOC_MEM
; ===========
;
; Allocates Janus Memory
;
; Expects:
;
;	AL : 	Type of memory to allocate
;	BX :    Number of Bytes to allocate
;
; Returns:
;
;	BX : 	Offset of requested memory if success,
;	AL :    Status (J_OK, J_NO_MEMORY)
;-------------------------------------------------------------------- 
; J_FREE_MEM
; ==========
;
; Releases Janus Memory
;
; Expects:
;
;	AL : 	Type of memory to free
;	BX :    Offset of Memory to free
;
; Returns:
;
; 	Crash if offset/type was wrong (J_GOODBYE, later)
;--------------------------------------------------------------------
; J_SET_PARAM
; ===========
;
; Set the default parameter memory pointer
;
; Expects:
;	
;	AL : 	Janus Service Number to support
;	BX :	Default Offset of Param Memory to install
;
; Returns:
;
;	AL : 	Status (J_OK, J_NO_SERVICE)
;--------------------------------------------------------------------
; J_SET_SERVICE
; =============
;
; Set an address for a far call for that service
;
; Expects:
;	
;	AL : 	Janus Service Number to support
;	ES:DX :	Entry address for FAR call
;
; Returns:
;
;	AL : 	Status (J_OK, J_NO_SERVICE)
;--------------------------------------------------------------------
; J_STOP_SERVICE
; ==============
;
; Prevents AMIGA from using the far call (see above) for
; this function and releases this Service Number.
; No memory is freed up.
; No calls are accepted from either side anymore.
;
; Expects:
;	AL :	Number of Service to stop
;
; Returns:
;
;	AL : 	Status (J_OK, J_NO_SERVICE)
;
;--------------------------------------------------------------------
; J_CALL_AMIGA
; ============
;
; Calls the requested function on AMIGA side.
; Does not wait for the call to complete.
; If J_SET_SERVICE defined, it is internally called on completion.
;
; Expects:
;	AL : 	AMIGA Service to call
;	BX : 	New Parameter Memory offset to use,
;		-1 : Use default offset
;				    		
; Returns:
;	AL : 	Status (J_PENDING, J_FINISHED, J_NO_SERVICE)
;--------------------------------------------------------------------
; J_WAIT_AMIGA
; ============
;
; Waits for a previos issued J_CALL_AMIGA to complete.
; This function is used if no J_SET_SERVICE is defined.
;
; Expects:
;	AL :	Service Number to wait for
;
; Returns:
;	AL :    Status (J_FINISHED, J_NO_SERVICE)
;--------------------------------------------------------------------
; J_CHECK_AMIGA
; =============
;
; Checks completion status of a pending J_CALL_AMIGA
;
; Expects:
;	AL : 	Service Number to check
;
; Returns:
;	AL :	Status (J_PENDING, J_FINISHED, J_NO_SERVICE)
;
;--------------------------------------------------------------------
;
; This is the Interrupt we are using:
;
JANUS		equ	0bh
;
; These are the function codes we know:
;
J_GET_SERVICE	equ	0
J_GET_BASE	equ	1	
J_ALLOC_MEM	equ	2
J_FREE_MEM	equ	3
J_SET_PARAM	equ	4
J_SET_SERVICE	equ	5
J_STOP_SERVICE	equ	6
J_CALL_AMIGA	equ	7
J_WAIT_AMIGA	equ	8
J_CHECK_AMIGA	equ	9

;
; Status Returns:
;
J_NO_SERVICE	equ	0ffh	; no service available
J_PENDING	equ	0	; after J_CALL_AMIGA and J_CHECK_AMIGA
J_FINISHED	equ	1	; after J_CALL_AMIGA and J_CHECK_AMIGA
J_OK		equ	0	; general good return
J_NO_MEMORY	equ	3	; requested memory not available
J_ILL_FNCTN	equ	4	; Illegal function code used in AH	
;

