* === rexx/rexxenv.i ===================================================
*
* Copyright (c) 1986, 1987-1990 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Include file for environment and global data structure definitions.

         IFND     REXXENV_I
REXXENV_I SET     1

         IFND     REXX_STORAGE_I
         INCLUDE  "rexx/storage.i"
         ENDC
         IFND     DOS_DOS_I
         INCLUDE  "dos/dos.i"
         ENDC

         ; The symbol table anchors the roots of the binary trees.

ST_SIZE  EQU   16                      ; number of root entries
         STRUCTURE SymTable,0
         STRUCT   st_Root,ST_SIZE*4    ; root array
         STRUCT   st_Special,3*4       ; cached nodes for special vars
         APTR     st_LastNode          ; last node assigned
         LABEL    st_SIZEOF            ; size: 80 bytes

* Special variable index constants (offsets to slot)
STRESULT EQU      0
STRC     EQU      4
STSIGL   EQU      8

* Defined fields
st_RESULT EQU     st_Special+STRESULT  ; cached pointer: RESULT
st_RC    EQU      st_Special+STRC      ; cached pointer: RC
st_SIGL  EQU      st_Special+STSIGL    ; cached pointer: SIGL

ST_MASK  EQU      $0F                  ; mask for root index

*         ; The Date structure is used to hold a DOS timestamp.
*
*         STRUCTURE DateTime,0
*         LONG     dt_Days
*         LONG     dt_Mins
*         LONG     dt_Ticks
*         LABEL    dt_SIZEOF            ; size: 12 bytes

TICKRATE EQU      TICKS_PER_SECOND     ; tick rate
TICKSDAY EQU      24*60*60*TICKRATE    ; ticks per day

         ; The Environment structure is the activation record for a REXX
         ; program.  Internal function calls create a new environment.

         STRUCTURE Environment,LN_SIZE
         WORD     ev_Level             ; current nesting level
         APTR     ev_GlobalPtr         ; pointer to global data structure
         APTR     ev_Segment           ; pointer to source Segment Array
         APTR     ev_ArgList           ; argument list clause
         LONG     ev_PC                ; 'Program Counter' register
         LONG     ev_SrcLine           ; current execution source line
         STRUCT   ev_EStack,skSIZEOF   ; execution pipeline
         STRUCT   ev_RStack,skSIZEOF   ; control stack
                                       ; 'inherited' strings
         APTR     ev_Prompt            ; read prompt string
         APTR     ev_LastComm          ; previous Host address
         APTR     ev_COMMAND           ; current Host address
                                       ; 'inherited' values
         ULONG    ev_ExecFlags         ; execution and trace flags
         ULONG    ev_SrcPos            ; source position
         LONG     ev_TraceLim          ; failure level
         LONG     ev_TraceCnt          ; suppression count
         APTR     ev_ST                ; pointer to Symbol Table
         UWORD    ev_NumDigits         ; numeric digits setting
         UWORD    ev_NumFuzz           ; numeric fuzz setting
         UWORD    ev_Indent            ; line number indentation
         UWORD    ev_IntFlags          ; interrupt flags
         LONG     ev_Elapse            ; elapsed time
         STRUCT   ev_Date,ds_SIZEOF    ; DOS timestamp
         LABEL    ev_SIZEOF            ; size: 124 bytes

* Field definitions
EVSUCC   EQU      LN_SUCC              ; successor
EVPRED   EQU      LN_PRED              ; predecessor
EVFLAGS  EQU      LN_TYPE              ; environment flags
EVNAME   EQU      LN_NAME              ; string value
STATUS   EQU      ev_ExecFlags         ; machine status word
TRACEOPT EQU      ev_ExecFlags+3       ; trace options
SRCPOS   EQU      ev_SrcPos            ; current source position
EVINTF   EQU      ev_IntFlags          ; interrupt flags
SYSDAYS  EQU      ev_Date+ds_Days
SYSMINS  EQU      ev_Date+ds_Minute
SYSTICKS EQU      ev_Date+ds_Tick

* Environment flag bit definitions
EFB_SCI     EQU   0                    ; scientific notation?
EFB_TIMER   EQU   1                    ; date/time stamp valid?
EFB_REQ     EQU   2                    ; request command result?
EFB_CACHE   EQU   3                    ; cache clauses?
EFB_RESULT  EQU   5                    ; result requested?
EFB_CMD     EQU   6                    ; a command invocation?
EFB_NEWST   EQU   7                    ; a new symbol table?

* Inherited flags
ENVFMASK    EQU   1<<EFB_CACHE!1<<EFB_REQ!1<<EFB_TIMER!1<<EFB_SCI

* Inheritance counts
ENVSCNT     EQU   3                    ; inherited strings
ENVLCNT     EQU   (ev_SIZEOF-ev_ExecFlags+4)/4

         ; The global data structure for this invocation.  This is at the
         ; head of the RexxTask structure.

         STRUCTURE GlobalNexx,ev_SIZEOF
         APTR     gn_CurrEnv           ; current storage environment
         LONG     gn_TotAlloc          ; total allocated space
         LONG     gn_TotFree           ; total available space
         APTR     gn_TBuff             ; work area (temporary buffer)
         APTR     gn_MsgPkt            ; global message packet
         APTR     gn_SigLabel          ; signalled label
         APTR     gn_LabSeg            ; pointer to Labels Array
         APTR     gn_SrcSeg            ; pointer to source Segment Array
         APTR     gn_LineSeg           ; pointer to Lines Array
         LONG     gn_RSeed             ; seed for random-number generator

         APTR     gn_SrcFile           ; invocation source file
         APTR     gn_FileName          ; resolved filename
         APTR     gn_FileExt           ; file extension
         APTR     gn_HostAddr          ; host address
         APTR     gn_SOURCE            ; "source" string

         APTR     gn_HotList1          ; quick list -- one quantum
         APTR     gn_HotList2          ; quick list -- two quanta
         APTR     gn_HotList3          ; quick list -- three quanta
         APTR     gn_TempValue         ; argstring (temporary)
         LABEL    gn_SIZEOF            ; size: 200 bytes

         ; Run-time definitions

* Tracing modes ...
TRACE_O     EQU   $00                  ; 'Off' ... no tracing
TRACE_C     EQU   $01                  ; 'Commands'
TRACE_E     EQU   $02                  ; 'Errors' -- non-zero error codes
TRACE_I     EQU   $03                  ; 'Intermediates'
TRACE_L     EQU   $04                  ; 'Labels'
TRACE_R     EQU   $05                  ; 'Results'
TRACE_N     EQU   $06                  ; 'Normal' -- negative error codes
TRACE_A     EQU   $07                  ; 'All' -- all clauses traced
TRACE_S     EQU   $08                  ; 'Scan' -- syntax checking only
TRACE_B     EQU   $09                  ; 'Back' ... background operation
EXB_ACTIVE  EQU   6                    ; interactive debugging
EXB_NOCOMM  EQU   7                    ; suppress host commands

* Dynamic control flag bits ...
EXB_SKIP    EQU   8                    ; skip next statement
EXB_RETURN  EQU   9                    ; return processing ...
EXB_CLEAR   EQU   10                   ; clear 'skip' flag
EXB_EXIT    EQU   11                   ; exit from interpreter
EXB_MOVE    EQU   13                   ; moving to a new location
EXB_COND    EQU   14                   ; conditional statement?
EXB_NEXTC   EQU   15                   ; next statement conditional?
EXB_SIGNAL  EQU   16                   ; interrupt flag

* State flags
EXB_CACHE   EQU   17                   ; clause caching
EXB_EXTRACE EQU   22                   ; external trace flag bit
EXB_DEBUG   EQU   23                   ; debug source?

EXB_PAUSE   EQU   24                   ; pause for interactive input?
EXB_TRACE   EQU   25                   ; clause to be traced?
EXB_FLUSH   EQU   26                   ; flush pipeline

* Flag definitions
EXF_SKIP    EQU   1<<EXB_SKIP
EXF_RETURN  EQU   1<<EXB_RETURN
EXF_CLEAR   EQU   1<<EXB_CLEAR
EXF_EXIT    EQU   1<<EXB_EXIT
EXF_MOVE    EQU   1<<EXB_MOVE
EXF_COND    EQU   1<<EXB_COND
EXF_SIGNAL  EQU   1<<EXB_SIGNAL
EXF_CACHE   EQU   1<<EXB_CACHE
EXF_EXTRACE EQU   1<<EXB_EXTRACE
EXF_DEBUG   EQU   1<<EXB_DEBUG
EXF_PAUSE   EQU   1<<EXB_PAUSE
EXF_TRACE   EQU   1<<EXB_TRACE
EXF_FLUSH   EQU   1<<EXB_FLUSH

TRACEMASK   EQU   $0F                  ; tracing option mask
EXECMASK    EQU   $FFFE00FF            ; inherited state mask
SIGMASK     EQU   EXF_PAUSE!EXF_SIGNAL!EXF_CLEAR!EXF_COND!EXF_SKIP

* Interrupt flag bits
INB_BREAK_C EQU   0                    ; control-C
INB_BREAK_D EQU   1                    ; control-D
INB_BREAK_E EQU   2                    ; control-E
INB_BREAK_F EQU   3                    ; control-F
INB_ERROR   EQU   4                    ; command error
INB_HALT    EQU   5                    ; global halt
INB_IOERR   EQU   6                    ; I/O error
INB_NOVALUE EQU   7                    ; uninitialized variable
INB_SYNTAX  EQU   8                    ; syntax error
INB_FAILURE EQU   9                    ; command failure

* String attributes associated with interrupt labels
LOADNOT     EQU   NSB_NUMBER           ; load error if not set
LOADSET     EQU   NSB_BINARY           ; load error if set
SETRC       EQU   NSB_FLOAT            ; set 'RC' variable

* Trace action codes
ACT_SYNTAX  EQU   $01                  ; syntax (execution) error
ACT_ERROR   EQU   $02                  ; host command error
ACT_LINE    EQU   $03                  ; source line
ACT_INTF    EQU   $04                  ; intermediate: function result
ACT_INTL    EQU   $05                  ; intermediate: literal value
ACT_INTO    EQU   $06                  ; intermediate: dyadic operation
ACT_INTP    EQU   $07                  ; intermediate: prefix operation
ACT_INTV    EQU   $08                  ; intermediate: symbol value
ACT_INTC    EQU   $09                  ; intermediate: expanded compound name
ACT_RESULT  EQU   $0A                  ; result: expression
ACT_PHOLD   EQU   $0B                  ; result: "placeholder"
ACT_ASSIGN  EQU   $0C                  ; result: assignment

* Miscellaneous tracing constants
TRINDMIN EQU      3                    ; minimum line number indentation
TRINDMAX EQU      5                    ; maximum indentation
TRINDLIM EQU      1000                 ; crossover lines

         ENDC
