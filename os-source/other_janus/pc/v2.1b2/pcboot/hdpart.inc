;****************************************************************************
; File: HDpart.inc
;
; Structures for accessing the partition block of DOS harddisk drives
;
;****************************************************************************
; HD Partition structure	
;****************************************************************************


HDPart0 	struc

BootFlag	db	?
HeadStart	db	?
SecStart	db	?
CylStart	db	?
SysFlag 	db	?
HeadEnd 	db	?
SecEnd		db	?
CylEnd		db	?
SecOffLow	dw	?
SecOffHigh	dw	?
SecNumLow	dw	?
SecNumHigh	dw	?

HDPart0 	ends

MaxPart		=    	04	; maximum number of partitions on harddisk
sectorsize	=    	200h	; 512 BYTE/SECTOR

;****************************************************************************
