	IFND	OBJ_I
OBJ_I	equ	1	

TYPE_NULL		EQU $00 a NULL object, does nothing (also free dict entry)
TYPE_INT		EQU $01 a 32 bit signed integer
TYPE_REAL		EQU $02 a single precision float
TYPE_NAME		EQU $03 a name to be looked up in dictionaries
TYPE_BOOL		EQU $04 a boolean with 0=FALSE or 1=TRUE
TYPE_STRING		EQU $05 array of uchars (don't confuse with 'C' strings)
TYPE_IMMED		EQU $06 ;;name (only used by the scanner)
TYPE_DICT		EQU $07 a dictionary
TYPE_PACKED		EQU $08 a packed array of postscript objects
TYPE_ARRAY		EQU $09 a normal array of postscript objects (or proc)
TYPE_MARK		EQU $0A a special stack marker for counttomark etc.
TYPE_FILE		EQU $0B actually a stream object or executable file
TYPE_OPERATOR	EQU $0C direct pointer to operator function entry point
TYPE_FONTID		EQU $0D a font FID object
TYPE_SAVE		EQU $0E a save level

; DPS extended types follow...
TYPE_LOCK		EQU $0F context locking semaphore
TYPE_GSTATE		EQU $10 virtual memory save state
TYPE_COND		EQU	$11 condition

	; each object is literal or executable and has access attributes

TYPE_MASK 		EQU	$1F	;; mask off the type field
ATTR_MASK 		EQU	$80	;; mask off the attribute field
ACCESS_MASK 	EQU	$60	;; mask off the access field
ATTR_LITERAL 	EQU	$00	;; gets pushed on the operand stack
ATTR_EXECUTE 	EQU	$80	;; get pushed on the execution stack

ACCESS_UNLIMITED EQU	$00	;; can do anything to this object
ACCESS_READ_ONLY EQU 	$20	;; can only read this object
ACCESS_EXE_ONLY	EQU	$40	;; can only execute this object
ACCESS_NONE 	EQU	$60	;; no access at all to this object

SAVE_MASK		equ	$1F	;; save level 00..31
__FREE_BIT__	equ	$20	;;
TRACKED			equ	$40	;; obj value is allocated and in tracked VM
ALLOCATED		equ	$80	;; obj value is allocated (vs. static)

	STRUCTURE g_obj,0
		UBYTE	go_type
		UBYTE	go_tag
		UWORD	go_len
		ULONG	go_val
	LABEL go_SIZEOF

	STRUCTURE ps_obj,0
		UBYTE	pso_type
		UBYTE	pso_tag
		UWORD	pso_len
			LABEL	pso_nullval		;	/* null,type=0,tag=0,len=0 */
			LABEL	pso_intval		;	/* integer,type=1,tag=0,len=0 */
			LABEL	pso_realval		;	/* real,type=2,tag=0,len=0 */
			LABEL	pso_nameval		;	/* name,type=3,tag=namelength,len=hashvalue */
			LABEL	pso_boolval		;	/* boolean,type=4,tag=0,len=0  */
			LABEL	pso_stringval	;	/* string,type=5,tag=0,len=stringlength */
			LABEL	pso_immediate	;	/* immediately evaluated name  (scanner only) */
			LABEL	pso_dictval		;	/* dictionary,type=7,tag=DICT_TYPE,len=maxentries */
			LABEL	pso_packval		;	/* packed array, type=8,tag=0,len=total size */
			LABEL	pso_arrayval	;	/* array,type=9,tag=0,len=maxentries */
			LABEL	pso_markval		;	/* mark,type=10,tag=0,len=0 */
			LABEL	pso_streamval	;	/* file,type=11,tag=0,len=0 */
			LABEL	pso_operval		;	/* operator,type=12,tag=0,len=0 */
			LABEL	pso_fontval		;	/* fontID,type=13,tag=0,len=0 */
			LABEL	pso_gstateval	;	/* gstate,type=14,tag=0,len=0 */
			LABEL	pso_lockval		;	/* lock,type=15,tag=0,len=0 */
			LABEL	pso_saveval		;	/* save,type=16,tag=0,len=0 */
			LABEL	pso_guardval	;	/* special stack guard entry */
		ULONG	pso_value
	LABEL pso_SIZEOF

	ENDC
