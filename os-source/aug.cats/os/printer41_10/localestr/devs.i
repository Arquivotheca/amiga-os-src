	IFND LOCALESTR_DEVS_I
LOCALESTR_DEVS_I	SET	1


;-----------------------------------------------------------------------------


* This file was created automatically by CatComp.
* Do NOT edit by hand!
*


	IFND EXEC_TYPES_I
	INCLUDE 'exec/types.i'
	ENDC


;-----------------------------------------------------------------------------



	IFD PRINTER_DEVICE
MSG_PRI_DEV_TROUBLE EQU 0
MSG_PRI_DEV_DOQUERY EQU 1
MSG_PRI_DEV_OFFLINE EQU 2
MSG_PRI_DEV_NOPAPER EQU 3
MSG_PRI_DEV_UNKNOWN EQU 4
MSG_PRI_DEV_MAYBEPAPER EQU 5
	ENDC ; PRINTER_DEVICE


;-----------------------------------------------------------------------------


	IFD STRINGARRAY

   STRUCTURE AppString,0
	LONG   as_ID
	APTR as_Str
   LABEL AppString_SIZEOF


	IFD PRINTER_DEVICE
MSG_PRI_DEV_TROUBLE_STR: DC.B 'Printer Trouble:',10,'%s',0
MSG_PRI_DEV_DOQUERY_STR: DC.B 'Resume|Cancel',0
MSG_PRI_DEV_OFFLINE_STR: DC.B 'Make printer on-line',0
MSG_PRI_DEV_NOPAPER_STR: DC.B 'Out of paper',0
MSG_PRI_DEV_UNKNOWN_STR: DC.B 'Check printer and cabling',0
MSG_PRI_DEV_MAYBEPAPER_STR: DC.B 'Check printer and paper',0
	ENDC ; PRINTER_DEVICE

	CNOP 0,4


AppString:

	IFD PRINTER_DEVICE
AS0:	DC.L MSG_PRI_DEV_TROUBLE,MSG_PRI_DEV_TROUBLE_STR
AS1:	DC.L MSG_PRI_DEV_DOQUERY,MSG_PRI_DEV_DOQUERY_STR
AS2:	DC.L MSG_PRI_DEV_OFFLINE,MSG_PRI_DEV_OFFLINE_STR
AS3:	DC.L MSG_PRI_DEV_NOPAPER,MSG_PRI_DEV_NOPAPER_STR
AS4:	DC.L MSG_PRI_DEV_UNKNOWN,MSG_PRI_DEV_UNKNOWN_STR
AS5:	DC.L MSG_PRI_DEV_MAYBEPAPER,MSG_PRI_DEV_MAYBEPAPER_STR
	ENDC ; PRINTER_DEVICE


	ENDC ; STRINGARRAY


;-----------------------------------------------------------------------------


	ENDC ; LOCALESTR_DEVS_I
