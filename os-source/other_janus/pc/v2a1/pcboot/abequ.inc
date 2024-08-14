; ABEqu.inc	-	Equates for AutoBoot handler
;
AB_ACTIVATE	=	0	;Make autoboot active
AB_INT13MSG	=	0	;Display parms when int13 called


;***************************************************************************
;* Debug switches for Autoboot File Open routine first_time
;***************************************************************************
DBG_AB_FILEOPENMSG  	=	1 ;When opening ab file print returns from
				  ;call and wait
DBG_AB_OPENFAILMSG	=	1 ;Display message is autoboot file open fails
DBG_AB_FTEXITMSG	=	1 ;File opened ok, routine exits
  
