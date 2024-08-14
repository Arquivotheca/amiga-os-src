;***************************************************************************
;* ABEqu.inc		-	Equates for AutoBoot handler
;***************************************************************************
				    
AB_ACTIVATE		=      	1	;Make autoboot active
AB_INT13MSG		=	0	;Display parms when int13 called


;***************************************************************************
;* Debug switches for Autoboot File Open routine first_time
;***************************************************************************

DBG_AB_FILEOPENMSG   	=	0 	;Prints return value from DoJanus
DBG_AB_READPARMS	=	0 	
DBG_AB_COPENFAILMSG   	=	1 	;Error on open of autoboot control file
DBG_AB_OPENFAILMSG	=	1 	;Error on open of autoboot file
DBG_AB_FTEXITMSG	=	0 	;File opened ok, routine exits
				  	
DBG_AB_CNV_IN	 	=	0 	;Entry to cnv_do
DBG_AB_CNV_OUT		=	0 	;Exit from cnv_do 
