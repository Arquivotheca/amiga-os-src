;************bill variables **********

public 		AB_bill_int13
AB_bill_int13	dd	1	;Storage for old int 13 vector

public		AB_Cyl
public		AB_Head
public		AB_Sec
public		AB_Secs
public		AB_SecsDone
AB_Cyl		dw	0  	;
AB_Head		dw	0
AB_Sec		dw	0
AB_Secs		dw	0
AB_SecsDone	dw	0

public		AB_jparmseg
public		AB_jparmoff
public		AB_jbuffseg
public		AB_jbuffoff
AB_jparmseg	dw	0  	;Pointer to janus structures, returned by
AB_jparmoff	dw	0	;GetBase(pcdisk)
AB_jbuffseg	dw	0
AB_jbuffoff	dw	0

public		AB_dataseg
public		AB_dataoff
AB_dataseg   	dw	0
AB_dataoff 	dw	0

public		AB_FileOpen
public		AB_NumHeads
public		AB_TrackSecs
public		AB_Cyls
public		AB_File
public		AB_TFile
AB_FileOpen	dw	0	;File open flag, 0 = not open
AB_NumHeads	dw	4	;# heads for fake device = 4
AB_TrackSecs	dw	17	;Tracks / sec for fake device
AB_Cyls		dw	168	;Cylinders
AB_File		dw	0ffh	;File # returned by pcdisk
AB_TFile	dw	0ffh	;Temp file handle for aboot.ctrl

public		AB_billmsg0
public		AB_billmsg1
public		AB_billmsg2
public		AB_billmsg3
public		AB_billmsg4
public		AB_billmsg5
AB_billmsg0	db	"AB INT 13 Func = ",0
AB_billmsg1	db	" Drive = ",0
AB_billmsg2	db	" Cyl = ",0
AB_billmsg3	db	" Head = ",0
AB_billmsg4	db	" Sec = ",0
AB_billmsg5	db	" #Secs = ",0

public		AB_READPARMS
public		AB_COPENFAILMSG
public		AB_OPENFAILMSG
public		AB_FTEXITMSG
public		AB_FileName
public		AB_okflag
AB_READPARMS	db	"Reading AutoBoot Parms.",0
AB_COPENFAILMSG	db	"AutoBoot Control File SYS:sidecar/ABoot.CTRL Open Error!",0
AB_OPENFAILMSG	db	"AutoBoot File Open Error!",0
AB_FTEXITMSG	db	"First_time exits.",0
AB_FileName	db	"SYS:sidecar/aboot.ctrl",0,0,"end"
AB_okflag	dw	1


