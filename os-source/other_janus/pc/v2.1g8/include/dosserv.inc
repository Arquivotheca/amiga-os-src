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
	dsr_Err_h	dw	0
	dsr_Err_l	dw	0
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
DSR_FUNC_SEEK_EXTEND		equ    10
DSR_FUNC_CREATE_DIR	 equ      11	
DSR_FUNC_LOCK		 equ      12
DSR_FUNC_UNLOCK		 equ      13
DSR_FUNC_EXAMINE	 equ      14
DSR_FUNC_EXNEXT	equ		15
DSR_FUNC_GETCURRENTDIR	equ	16
DSR_FUNC_SETCURRENTDIR	equ	17
DSR_FUNC_DELETEFILE	equ	18
DSR_FUNC_DUPLOCK	equ	19
DSR_FUNC_PARENTDIR	equ	20
DSR_FUNC_RENAME	equ		21
DSR_FUNC_SETPROTECTION	equ	22
DSR_FUNC_PARSEPATTERN	equ	23
DSR_FUNC_MATCHPATTERN	equ	24
DSR_FUNC_ENDCURRENTDIR	equ	25
DSR_FUNC_SETFILEDATE	equ	26

;***************************/
;* Error codes for dsr_Err */
;***************************/

DSR_ERR_OK        	 	equ	0
DSR_ERR_UNKNOWN_FUNCTION 	equ	1
DSR_ERR_TOO_MANY_FILES 		equ	2
DSR_ERR_OPEN_ERROR		equ	3
DSR_ERR_FILE_NOT_OPEN		equ	4
DSR_ERR_SEEK_ERROR		equ	5
DSR_ERR_TOO_MANY_LOCKS 	equ		6

