; TimeServ Equates
;
TIMESERV_APPLICATION_ID	=	1
APPLICATION_ID_H	=	0
APPLICATION_ID_L	=	1
TIMESERV_LOCAL_ID	=	1
;
TimeServReq	struc

tsr_Year	dw	0
tsr_Month	db	0
tsr_Day		db	0
tsr_Hour	db	0
tsr_Minutes	db	0
tsr_Seconds	db	0
tsr_String	db  27	dup(?)
tsr_Err		db	0

TimeServReq	ends 
;
