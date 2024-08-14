	STRUCTURE mydata,0
		APTR    JIOBase         ;pointer to janus IO registers
		APTR	Glib		;pointer to graphics library
		APTR    Ilib            ;pointer to intuition library
		APTR    Dlib            ;pointer to DOS library
		APTR    PrefWindow      ;this programs window
		APTR    MPort           ;this programs message port
		ULONG   MSigs           ;signal bit from message port
		ULONG	FinishedFlag	;flag, time to exit the program
		UWORD	FinishCode	;what to do when finished
		BPTR    PrefsFile       ;file handle for prefs file
		UBYTE   SerialState     ;state of serial enable/disable
		UBYTE   MonoState       ;state of mono enable/disable
		UBYTE   ColorState      ;state of color enable/disable
		UBYTE   RamBank         ;position of 64k RAM bank
		LABEL   mydata_SIZEOF

PREFSIZE        EQU     4               ;size of preferences data
DEFSER          EQU     0               ;default serial to off
DEFMON          EQU     1               ;default mono to on
DEFCOL          EQU     0               ;default color to off
DEFRAM          EQU     2               ;default RAM to $D0000-$DFFFF


