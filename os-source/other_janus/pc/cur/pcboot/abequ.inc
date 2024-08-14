;***************************************************************************
;* ABEqu.inc		-	Equates for AutoBoot handler
;***************************************************************************
				    
AB_ACTIVATE		=      	1	;Make autoboot active
AB_INT13MSG		=	0	;Display parms when int13 called


;***************************************************************************
;* Debug switches for Autoboot File Open routine first_time
;***************************************************************************

DBG_AB_FILEOPENMSG   	=	0	;Prints return value from DoJanus
					;after aboot.ctrl file open
DBG_AB_READPARMS	=	0 	
DBG_AB_COPENFAILMSG   	=	0 	;Error on open of autoboot control file
DBG_AB_OPENFAILMSG	=	0 	;Error on open of autoboot file
DBG_AB_FTEXITMSG	=	0 	;File opened ok, routine exits
				  	
DBG_AB_CNV_IN	 	=	0 	;Entry to cnv_do
DBG_AB_CNV_OUT		=	0 	;Exit from cnv_do 

DBG_AB_NOPCDISKMSG	=	0	;First_Time pcdisk not found.
DBG_AB_PCDISKMSG	=	0	;First_Time pcdisk found.

AB_CTRLOPENERRMSG	=	0	;Error return from pcdisk on
					;open of aboot.ctrl
