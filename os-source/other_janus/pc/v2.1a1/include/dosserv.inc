;************************************************************************
;* (PC side file)
;*
;* DOSServ.h - DOSServ specific data structures
;*
;* 11-19-90 - Bill Koester - Created this file
;************************************************************************/


DOSSERV_APPLICATION_ID	equ	1
DOSSERV_LOCAL_ID	equ	3

;*************************/
;* DOS request structure */
;*************************/

DOSServReq	struc
	dsr_Function	dw	0
	dsr_Lock	db	0
	dsr_Pad		db	0
	dsr_Buffer_Seg	dw	0
	dsr_Buffer_Off	dw	0
	dsr_Buffer_Size	dw	0
	dsr_Err		dw	0
	dsr_Arg1_h	dw	0
	dsr_Arg1_l	dw	0
	dsr_Arg2_h	dw	0
	dsr_Arg2_l	dw	0
	dsr_Arg3_h	dw	0
	dsr_Arg3_l	dw	0
DOSServReq	ends

;***********************************/
;* Function codes for dsr_Function */
;***********************************/

DSR_FUNC_OPEN_OLD		equ	1
DSR_FUNC_OPEN_NEW		equ	2
DSR_FUNC_OPEN_READ_WRITE	equ	3
DSR_FUNC_CLOSE			equ	4
DSR_FUNC_READ			equ	5
DSR_FUNC_WRITE			equ	6
DSR_FUNC_SEEK_BEGINING		equ	7
DSR_FUNC_SEEK_END		equ	8
DSR_FUNC_SEEK_CURRENT		equ	9

;***************************/
;* Error codes for dsr_Err */
;***************************/

DSR_ERR_OK        	 	equ	0
DSR_ERR_UNKNOWN_FUNCTION 	equ	1
DSR_ERR_TOO_MANY_FILES 		equ	2
DSR_ERR_OPEN_ERROR		equ	3
DSR_ERR_FILE_NOT_OPEN		equ	4

