;*************************************************************************
; (PC side file)
;
; Autoload.inc -- Structures and defines for Janus memory
;
; Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
;
;*************************************************************************
; The autoload buffer looks like this:
;    - First there is a 16-bit value describing how many 
;      AutoEntry structures follow
;    - Then there are a number of AutoEntry structures
; To find a free AutoEntry structure, check the AppID field.  
; If this is zero, you can use this field
;

AUTOLOAD_ENTRYCOUNT	=    4	; Number of entries in buffer 

AUTOLOAD_APPID_H	=    0
AUTOLOAD_APPID_L	=  123
AUTOLOAD_LOCALID	=    2

ALOAD_TimeOut	  	=   33	; time [s] to wait for autoloaded service
				;  come available
WAIT_TimeOut	  	=  777	; time [s] to wait for requested service
				;  come available


AutoLoad	struc 

   al_entries	dw	0
   al1_AppID	dd	0
   al1_LocalID	dw	0
   al2_AppID	dd	0
   al2_LocalID	dw	0
   al3_AppID	dd	0
   al3_LocalID	dw	0
   al4_AppID	dd	0
   al4_LocalID	dw	0

AutoLoad	ends

