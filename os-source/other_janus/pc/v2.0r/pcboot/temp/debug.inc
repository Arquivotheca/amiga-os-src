;******************************************************************************
;
; Debug.inc contains flag to enable debug messageses 
;
;******************************************************************************

include		mes.inc			; includes all message offsets

.xlist
;
;--- Assembly conditions ------------------------------------------------------
;
infolevel      		=    	10	; message output level 
video	       		=    	1	; enable video output
serial	       		=    	0	; enable serial output
;
;--- Debug Message Flags ------------------------------------------------------------
;
; Load_irq:
DBG_HandlerVersionMsg	=	1     	; Handler Version Message
DBG_LibVersionMsg	=	1     	; Library Version Message
DBG_WrongVersionMsg	=	1	; Wrong Version Warning
DBG_LoadMsg		=	0	; Handler loaded at %x		    
DBG_CopyMsg		=	0	; Copyright message		    
DBG_BaseMsg		=	0	; Janus Base found at %x
DBG_NoBaseMsg		=	1	; No Janus Base warning
DBG_NoServiceBaseMsg 	=	1	; No Service Base warning
DBG_BufferMsg		=	0	; Buffer mem found at %x
DBG_IntsMsg		=	0	; Int table offset = %x
DBG_ParasMsg		=	0	; Int Paras offset = %x
DBG_JPartBase		=	0	; Janus Partition Info
;
; Irq3Serv:
DBG_HWMsgP		=	0	; Pending HW interrupt								     
DBG_HWMsgR  		=	0	; HW interrupt ready
DBG_HWMsgPC 		=	0	; PC request for janus
DBG_IntCtrlMes		=	0	; Status of interrupt controller
DBG_DosMsgA 		=	0	; Pending INT13
DBG_DosMsgN 		=	0	; INT13	ready
DBG_IdleMsg 		=	0	; DOS idle status
;
; JI_Han:
DBG_IntTblMes		=	0	; interrupt table
DBG_FoundIntMes		=	0	; interrupt number
DBG_NoIntMes		=	0	; no interrupt warning
DBG_ExeIntMes		=	0	; execute interrupt							  
;
; HD I/O:
DBG_HDPart		=	0	; partition messages
DBG_regmsg   		=	0	; show diskIO regs
DBG_diskmsg1		=	0	; access HD
DBG_BlockMove		=	0	; copy parameter
;
; Scroll:
DBG_Scroll 		=	0	; scroll diagnostic
DBG_UpdateMsg		=	0	; PIC update warning
;
; Service 3,4 :
DBG_MemChunk	  	=	0   	; free memory chunk found
DBG_PreLock		=	0	; try to lock semaphore
DBG_PastLock		=	0	; semaphre locked
;
; Service 7,8,9 :
DBG_WaitStatus	  	= 	0	; print status
;
; Service 10,11 :
DBG_ServiceSearch 	=	0	; search thru service list
DBG_MakeService	  	=	0	; all. and init mem for service struc
DBG_AllocParam	  	=  	0	; allocate customers memory
DBG_InitPointers  	=	0	; init pointer to service strucure
DBG_InitWaitPointers  	=	0	; init pointer and wait for amiga
DBG_ServiceAvailable	=	0	; linked to service successfully
DBG_ServiceAdded  	=	0	; service structure added to system
;
; Service 12 :
DBG_CallAmigaService 	=  	0
;
; Service 13,14 :
;DBG_ChannelMsg	  	=	0
DBG_UCountMsg	  	=	0
DBG_RemoveMsg	  	=	0
DBG_ServiceDeleted 	=    	0 
;
; Service D :
DBG_GotServiceInt 	=	1
DBG_AmigaAddMsg	  	=	1
DBG_AmigaCallMsg    	=	1
DBG_AmigaDeleteMsg 	=	1
DBG_HandlerMsg    	=	1
DBG_NextSlotMsg	  	=	0
DBG_ExitSDMsg	  	=	1

