include	title.inc
subttl	INT 10H (Video IO)

.xlist
include external.inc
include equates.inc

public	V0      
public  SET_VIDEO_MODE         
public  SET_CURSOR_TYPE        
public	SET_CURSOR_POSN        
public  RD_CURSOR_POSN         
public  RD_LPEN_POSN           
public  SEL_DISP_PAGE          
public  SCROLL_SCR_UP          
public	SCROLL_SCR_DWN         
public	RD_ATR_CHAR            
public	WR_ATR_CHAR            
public	WR_CHAR                
public	SET_COLOR_PLT          
public	WR_DOT                 
public	RD_DOT                 
public	WR_TELETYPE            
public	RD_VIDEO_STATE         
public	V2                     
public	STRING                 

extrn	video_mode:byte
extrn	disp_cols:word
extrn	dbfr_size:word
extrn	dbfr_start:word
extrn	cursor_position:word
extrn	ega_rows:byte

extrn	cursor_mode:word
extrn	current_page:byte
extrn	crtc_base_adr:word
extrn	video_mode_reg:byte
extrn	crt_color:byte
extrn	disp_bfr_b:near
extrn	disp_bfr_w:near
extrn	beep:near
bell	equ	7
extrn	equip_flag:word
extrn	parm_ptr:dword
extrn	user_cgen_table:dword
extrn	char_gen_table:near
.list

zero	segment	at 0
zero	ends

dataa	segment	at 40h
dataa	ends

disp_bfr	segment	at 0B800h
disp_bfr	ends

eproma	segment	byte public
                                        
	ASSUME CS:EPROMA, DS:DATAA, ES:DISP_BFR                     

public aa_int10
aa_int10 label byte


;******************************************************************************
;                                      
; 			VIDEO             
;     THIS ROUTINE PROVIDES THE INTERFACE TO THE VIDEO DIPLAY.
;          
;               
;     INPUT TO ROUTINE / OUTPUTS FROM ROUTINE:
;              
;     REGISTER AH (WHICH CAN CONTAIN VALUES 0-15) SPECIFIES THE COMMAND
;     TO BE PERFORMED. OTHER REGISTERS SPECIFY PARAMETERS WHICH ARE 
;     BELOW.
;          
;     OTHER REGS DESTROYED :
;      
;
;******************************************************************************

public		VIDEO

VIDEO PROC NEAR
;                    
        STI                            ;RENABLE INTERRUPTS
        PUSH    BP                     ;SAVE THE REGISTERS
        PUSH    SI
        PUSH    DI
        PUSH    ES
        PUSH    DS
        CMP     AH,19                  ;STRING COMMAND ?
        JE      V0A                    ;YES, SKIP CHANGING BP
        MOV     BP,SP                  ;SAVE STACK PTR FOR MODIFIED RETURN
V0A:    PUSH    BX
        PUSH    CX
        PUSH    DX
        CMP     AH,19                  ;IS COMMAND OUT OF RANGE?
        JA      V2                     ;YES - RETURN TO CALLER
                                       ;NO - CONTINUE
        MOV     SI,DATA                ;SETUP DATA SEGMENT
        MOV     DS,SI                   
        CMP     AH,19                  ;STRING COMMAND ?
        JE      V1B                    ;YES, SKIP CHANGING ES
;GET VIDEO SEGMENT
        MOV     SI,EQUIP_FLAG          ;GET EQUIP FLAG
        MOV     DI,DISP_BFR_SEG        ;GET DISP BFR SEG FOR MONOCHROME
        AND     SI,30H                 ;ISOLATE VIDEO SWITCHES
        CMP     SI,30H                  
        JE      V1                     ;JUMP IF MONOCHROME
        ADD     DI,800H                ;B8000 FOR COLOR GRAPHICS CARD
V1:     MOV     ES,DI                  ;SETUP ES TO POINT TO DISP BFR
;PREPARE TO CALL ROUTINE
V1B:    XCHG    AH,AL                  ;SET CMD IN LOW BYTE
        MOV     DI,AX                  ;PUT CMD IN DI LOW BYTE
        XCHG    AH,AL                  ;CMD IN HIGH BYTE AGAIN
        AND     DI,00FFH               ;ZEROUT HIGH BYTE
        SHL     DI,1                   ;MULTIPLY BY 2 TO GET TABLE OFFSET
        CMP     AH,19                  ;STRING COMMAND ?
        JE      V1C                    ;YES, JUMP
        MOV     AH,VIDEO_MODE          ;
V1C:    CLD                            ;SET DIR FLAG FOR AUTOINC
        CALL    WORD PTR CS:[DI+OFFSET V0]   ;CALL THE APPROPRIATE ROUTINE
                                             ;BASED ON THE COMMAND
;
;       RESTORE REGS AND RETURN
;
V2:     POP     DX                     ;RETURN POINT FOR ALL ROUTINES EXCEPT
        POP     CX                     ;FOR THE ONES SHOWN BELOW
MOD_RET1:
        POP     BX                     ;RD_CURSOR_POSN ROUTINE RETURNS HERE
MOD_RET2:
        POP     DS                     ;RD_LPEN_POSN ROUTINE RETURNS HERE
                                       ;RD_VIDEO_STATE ROUTINE RETURNS HERE
        POP     ES
        POP     DI
        POP     SI
        POP     BP
        IRET                           ;RETURN TO CALLER
VIDEO   ENDP

;
;
;       VIDEO ROUTINES - OFFSETS
;

V0      LABEL   WORD
        DW      OFFSET SET_VIDEO_MODE         ;0
        DW      OFFSET SET_CURSOR_TYPE        ;1
        DW      OFFSET SET_CURSOR_POSN        ;2
        DW      OFFSET RD_CURSOR_POSN         ;3
        DW      OFFSET RD_LPEN_POSN           ;4
        DW      OFFSET SEL_DISP_PAGE          ;5
        DW      OFFSET SCROLL_SCR_UP          ;6
        DW      OFFSET SCROLL_SCR_DWN         ;7
        DW      OFFSET RD_ATR_CHAR            ;8
        DW      OFFSET WR_ATR_CHAR            ;9
        DW      OFFSET WR_CHAR                ;10
        DW      OFFSET SET_COLOR_PLT          ;11
        DW      OFFSET WR_DOT                 ;12
        DW      OFFSET RD_DOT                 ;13
        DW      OFFSET WR_TELETYPE            ;14
        DW      OFFSET RD_VIDEO_STATE         ;15
        DW      OFFSET V2_DUMMY               ;16
        DW      OFFSET V2_DUMMY               ;17
        DW      OFFSET V2_DUMMY               ;18
        DW      OFFSET STRING                 ;19


V2_DUMMY:
	ret	

                                                                               
;******************************************************************************
;
; SET VIDEO MODE    
;     THIS ROUTINE INITIALIZES THE VIDEO ACCORDING TO THE SPECIFIED
;     MODE. THE CRT 6845 CONTROLLER (CRTC) REGISTERS ARE SETUP,      
;     THE VIDEO MEMORY VARIABLES ARE INITIALIZED, AND THE DISPLAY BUFFER
;     IS FILLED WITH BLANKS.
;               
;     INPUT TO ROUTINE :
;     AL = VIDEO MODE
;     SI = EQUIPMENT FLAGS
;     DS = DATA SEGMENT
;     ES = DISPLAY BUFFER SEGMENT
;
;     OUTPUT FROM ROUTINE :
;     NONE
;          
;     OTHER REGS DESTROYED :
;     AX,BX,CX,DX,SI,DI,DS
;
;******************************************************************************
       

SET_VIDEO_MODE  PROC    NEAR
        PUSH    DS                     ;SAVE DATA SEGMENT
        CMP     SI,30H                 ;CHECK EQUIP FLAGS
        JE      V10                    ;JUMP IF MONOCHROME
        MOV     DX,03D4H               ;SETUP COLOR CRAPHICS ADAPTER PORT ADDR 
        XOR     CL,CL                  ;PREPARE TO SET MODE REG TO 00 
                                       ;(THIS WILL DISABLE VIDEO)
	cmp	al,6			;see if valid video mode (0-6)
	jbe	V11			;use mode byte from unput
	mov	al,2			;assume 80x25 bw
	cmp	si,20h			;check equipment flag
	je	V11			;yes, 80x25 bw
	mov	al,0			;else, 40x25 bw
        JMP     SHORT V11              
V10:    MOV     DX,03B4H               ;SETUP MONOCHROME DISPLAY POTR ADDR 
        MOV     CL,1                   ;PREPARE TO SET MODE REG TO 01         
                                       ;(THIS WILL SET HI RES AND DISBLE VIDEO)
        MOV     AL,7
V11:    MOV     CRTC_BASE_ADR,DX       ;SETUP CRTC PORT BASE ADDRESS
        MOV     VIDEO_MODE,AL          ;SAVE VIDEO MODE
	mov	ega_rows,24		;number of rows on the screen
        ADD     DX,4                   ;POINT TO MODE REG
        MOV     AL,CL                  ;  
        OUT     DX,AL                  ;OUTPUT TO MODE REG
        SUB     DX,4                   ;POINT TO BASE ADR
        MOV     BL,VIDEO_MODE          ;CALCULATE PARAMETER TABLE OFFSET
        CMP     BL,6                   ;MODE 6 USES SAME PARAMETERS AS 4,5
        JNE     V12
        DEC     BL
V12:    SHR     BL,1
        MOV     AL,PARM_LENGTH         ;GET LENGTH OF TABLE
        MUL     BL                     ;CALCULATE OFFSET IN AX
        XOR     BX,BX                  ;PARM TABLE PTR IS IN ZERO SEG
        MOV     DS,BX
ASSUME DS:ZERO            
        LDS     BX,PARM_PTR
ASSUME DS:EPROMA          
        ADD     BX,AX                  ;ADD TABLE OFFSET
        MOV     SI,BX                  ;SAVE IN SI
        MOV     CX,PARM_LENGTH
        XOR     BL,BL
;
;       INITIALIZE CRTC REGS R0 - R15 WITH PARAMETERS
;
V13:    LODSB                          ;GET PARAMETER
        MOV     AH,AL
        PUSH    DX                     ;SAVE PORT ADDR
        CALL    CRTC_OUTX              ;OUTPUT TO CRTC REGISTER 
        POP     DX                     ;RESTORE PORT ADDR
        INC     BL                     ;POINT TO NEXT REG
        LOOP    V13                    ;DO IT 16 TIMES
;
ASSUME DS:DATAA            
        POP     DS                     ;GET DATA SEGMENT VALUE
        SUB     DI,DI                  
        MOV     AH,VIDEO_MODE
        CALL    CHK_MODE               ;DETERMINE MODE
        MOV     AX,0720H               ;AX CONTAINS BLANK CHAR PLUS ATRIBUTE
        JNZ     V14                    ;
        MOV     CX,2048                ;MONOCROME DISP BFR SIZE IS 2K WORDS
        JMP     SHORT V16
V14:    MOV     CX,8192                ;COLOR GRAPHICS ADAPTER BFR SIZE IS 8K
                                       ;WORDS
V15:    JC      V16
        SUB     AX,AX                  ;FILL WITH ZEROES FOR GRAPHICS MODE
V16:    REP     STOSW                  ;FILL THE BUFFER
;
        XOR     AX,AX
        MOV     CURRENT_PAGE,AL        ;SET CURRENT PAGE TO ZERO
        MOV     DBFR_START,AX          ;SETUP STARTING ADR OF DISP BUFFER
;
        MOV     AL,VIDEO_MODE          ;MODE TO AX
        CBW                            ;ZEROS TO HIGH BYTE
        MOV     DI,AX                  ;USE MODE AS TABLE INDEX 
        MOV     AL,CS:[DI+OFFSET COL_TABLE]
        MOV     DISP_COLS,AX           ;ESTABLISH NO. OF COLUMNS
        MOV     AL,CS:[DI+OFFSET MODE_TABLE]
        MOV     VIDEO_MODE_REG,AL      ;SAVE MODE REG VALUE
        MOV     DX,CRTC_BASE_ADR
        ADD     DX,4                   ;POINT TO MODE REG
        OUT     DX,AL                  ;OUTPUT TO MODE REG (ENABLE VIDEO)
        INC     DX                     ;POINT TO COLOR SELECT REG
        MOV     AL,3FH                 ;VALUE FOR 640 X 200
        CMP     VIDEO_MODE,6           ;IS IT 640 X 200 ?
        JE      V17                    ;YES - OUTPUT VALUE
        MOV     AL,30H                 ;NO - SET VALUE FOR NON 640 X 200
V17:    MOV     CRT_COLOR,AL           ;SAVE THE COLOR VALUE
        OUT     DX,AL                  ;OUTPUT TO COLOR SELECT REG
;
        AND     DI,0EH                 ;CALCULATE INDEX  TO LENGTH TABLE
        MOV     AX,CS:[DI+OFFSET LEN_TABLE]  ;ESTABLISH DISP BFR SIZE
        MOV     DBFR_SIZE,AX
        XOR     BX,BX
        MOV     CX,8
V18:    MOV     word ptr CURSOR_POSITION[BX],0  ;SET CURSOR POSITION OF ALL PAGES TO 0
        ADD     BX,2
        LOOP    V18
        MOV     CURSOR_MODE,67H        
        RET
SET_VIDEO_MODE ENDP
;
;       COLUMN TABLE
;
COL_TABLE       LABEL    BYTE          
        DB      40                     ;# OF COLS FOR 40X25 BW
        DB      40                     ;# OF COLS FOR 40X25 COLOR
        DB      80                     ;# OF COLS FOR 80X25 BW
        DB      80                     ;# OF COLS FOR 80X25 COLOR
        DB      40                     ;# OF COLS FOR 320X200 COLOR
        DB      40                     ;# OF COLS FOR 320X200 BW
        DB      80                     ;# OF COLS FOR 640X200 BW
        DB      80                     ;# OF COLS FOR MONOCHROME
;
;       MODE TABLE
;
MODE_TABLE      LABEL    BYTE          
        DB      2CH                    ;MODE  FOR 40X25 BW
        DB      28H                    ;MODE  FOR 40X25 COLOR
        DB      2DH                    ;MODE  FOR 80X25 BW
        DB      29H                    ;MODE  FOR 80X25 COLOR
        DB      2AH                    ;MODE  FOR 320X200 COLOR
        DB      2EH                    ;MODE  FOR 320X200 BW
        DB      1EH                    ;MODE  FOR 640X200 BW
        DB      29H                    ;MODE  FOR MONOCHROME
;
;       DISPLAY BFR LENGTH
;
LEN_TABLE       LABEL    WORD
        DW      2048                   ;LENGTH FOR 40X25
        DW      4096                   ;LENGTH FOR 80X25
        DW      16384                  ;LENGTH FOR GRAPHICS MODE
        DW      16384                  ;LENGTH FOR GRAPHICS MODE


                                        
;******************************************************************************
;
; SET CURSOR TYPE   
;     THIS ROUTINE SETS THE CURSOR TYPE BASED ON THE VALUE IN CX
;               
;     INPUT TO ROUTINE :
;     CH = START LINE FOR CURSOR
;     CL = END LINE FOR CURSOR
;
;     OUTPUT FROM ROUTINE :
;     NONE
;          
;     OTHER REGS DESTROYED :
;     AX,BX,DX
;
;******************************************************************************
                                        
SET_CURSOR_TYPE PROC    NEAR
        MOV     AX,CX                  ;GET CURSOR VALUE
        MOV     CURSOR_MODE,AX         ;SAVE IT
        MOV     BL,10                  ;CRTC R10 IS CURSOR START REG
                                       ;CRTC R11 IS CURSOR END REG
        CALL    CRTC_OUT2              ;OUTPUT TO CRTC R10,R11 REGS
        RET
SET_CURSOR_TYPE ENDP



;******************************************************************************
;
; SET CURSOR POSITION
;     THIS ROUTINE POSITIONS THE CURSOR AT THE ROW/COL             
;     COORDINATE SPECIFIED IN DX.
;               
;     INPUT TO ROUTINE :
;     DH = ROW COORDINATE OF CURSOR
;     DL = COLUMN COORDINATE OF CURSOR
;     BH = DISPLAY PAGE OF CURSOR
;
;     OUTPUT FROM ROUTINE :
;     NONE
;          
;     OTHER REGS DESTROYED :
;     AX,BX,DX
;
;******************************************************************************

SET_CURSOR_POSN PROC    NEAR
        MOV     BL,BH                  ;DISPLAY PAGE IN BL
        XOR     BH,BH
        SAL     BX,1                   ;USE DISPLAY PAGE AS INDEX TO TABLE
        MOV     CURSOR_POSITION[BX],DX ;SAVE ROW,COL IN CURSOR TABLE
        SHR     BL,1                   ;GET BACK DISPLAY PAGE
        CMP     BL,CURRENT_PAGE        ;IS DISPLAY PAGE THE CURRENT PAGE?
        JNE     V25                    ;NO - WE ARE DONE, RETURN
        CALL    SET_CURSOR_REG         ;YES - SET THE CURSOR
V25:    RET                            ;                   
SET_CURSOR_POSN ENDP
      


                                        
;******************************************************************************
;
; READ CURSOR POSITION
;     THIS ROUTINE READS THE CURRENT CURSOR POSITION AND
;     RETURNS THE ROW,COL COORDINATES.
;               
;     INPUT TO ROUTINE :
;     BH = DISPLAY PAGE OF CURSOR
;
;     OUTPUT FROM ROUTINE :
;     DH = ROW COORDINATE OF CURSOR    
;     DL = COLUMN COORDINATE OF CURSOR 
;     CX = CURRENT CURSOR MODE         
;          
;     OTHER REGS DESTROYED :
;     BX  
;
;******************************************************************************
       
RD_CURSOR_POSN  PROC    NEAR
        MOV     CX,CURSOR_MODE         ;GET CURSOR MODE
        CALL    GET_CUR_POSN           ;GET THE CURSOR POSITION
        SUB     BP,2                   ;SKIP RESTORING DX,CX REGS
        MOV     SP,BP
        JMP     MOD_RET1               ;MODIFIED RETURN
RD_CURSOR_POSN  ENDP

                                        
;******************************************************************************
;
; READ LIGHT PEN POSITION
;     THIS ROUTINE READS THE LIGHTPEN POSITION AND RETURNS
;     THE LOCATION IF THE LIGHTPEN SWITCH AND TRIGGER ARE SET.
;     IF THE SWITCH OR TRIGGER IS NOT SET NOTHING IS RETURNED.
;
;     INPUT TO ROUTINE :
;     NONE              
;
;     OUTPUT FROM ROUTINE :
;     AH = 0  LIGHTPEN POSN INFORMATION NOT AVAILABLE
;     AH = 1  LIGHTPEN POSN INFORMATION AVAILABLE
;               DH,DL = ROW,COL OF CURRENT LIGHTPEN
;               CH = RASTER POSITION
;               BX = HORIZONTAL PIXEL POSITION (APPROXIMATION)
;
;     OTHER REGS DESTROYED :
;     AL,CL
;
;******************************************************************************

RD_LPEN_POSN    PROC    NEAR
        XOR     AH,AH
        MOV     DX,CRTC_BASE_ADR       ;GET CRTC BASE ADR
        ADD     DX,6                   ;POINT TO STATUS REG
        IN      AL,DX                  ;READ STATUS
        TEST    AL,04H                 ;IS LIGHTPEN SWITCH ON?
        JNZ     V34                    ;NO - RETURN
        TEST    AL,02H                 ;YES - NOW TEST LIGHTPEN TRIGGER
        JZ      V35                    ;JUMP IF NOT SET
        MOV     BL,16                  ;R16 IS LIGHTPEN REG(HIGH)
        CALL    CRTC_IN                ;READ CRTC R16 REG
        MOV     AH,AL                  ;SAVE IN AH
        INC     BL                     ;POINT TO R17
        CALL    CRTC_IN                ;READ CRTC R17 REG
                                       ;AX NOW CONTAINS LIGHTPEN REFRESH ADR
        SUB     AX,DBFR_START          ;CONVERT TO OFFSET RELATIVE TO BEGIN
                                       ;OF PAGE
        XOR     BH,BH
        MOV     BL,VIDEO_MODE          ;GET MODE TO DETERMINE SUBTRACT AMT
        MOV     BL,CS:SUBAMT[BX]       ;DETERMINE HOW MUCH TO SUBRACT  
        SUB     AX,BX                  ;ADJUST THE OFFSET
        JNS     V31                    
        XOR     AX,AX                  ;IF LESS THAN 0, TREAT IT AS 0
V31:    PUSH    AX
        MOV     AH,VIDEO_MODE
        CALL    CHK_MODE               ;CHECK FOR GRAPHICS MODE
        POP     AX
        JNC     V32                    ;JUMP IF GRAPHICS
;
;       ALPHA MODE
;
        DIV     BYTE PTR DISP_COLS     ;CALCULATE ROW,COL
        XCHG    AL,AH
        MOV     DX,AX                  ;DH = ROWS, DL = COLS
        MOV     CL,3                   ;SETUP SHIFT COUNT
        MOV     CH,AH                  ;GET ROWS
        SHL     CH,CL                  ;RASTER VALUE = ROW * 8
        SUB     BH,BH
        MOV     BL,AL                  ;COLUMNS IN BX
        SHL     BX,CL                  ;PIXEL VALUE = COL * 8
        JMP     V33
;
;       GRAPHICS MODE
;
V32:    MOV     CL,40                  ;DIVIDE BY 40 TO GET ROW,COLS
        DIV     CL                     ;AL = ROWS, AH = COLS
        XCHG    AH,AL                  ;AH = ROWS, AL = COLS
        MOV     DX,AX                  ;DH = ROWS, DL = COLS
        MOV     CL,2                   ;SETUP SHIFT COUNT
        SHR     DH,CL                  ;DIVIDE ROWS BY 4
        MOV     CH,AH                  ;ROWS TO CH
        SHL     CH,1                   ;RASTER VALUE = ROWS * 2
        MOV     CL,3                   ;SETUP SHIFT COUNT
        MOV     BL,AL                  ;COLS TO BX
        XOR     BH,BH
        SHL     BX,CL                  ;PIXEL VALUE = COL * 8
        CMP     VIDEO_MODE,6           ;HIGH RESOLUTION ?
        JNE     V33                    ;NO - GO AROUND ADJUSTMENT
                                       ;YES - ADJUSTMENT FOR HIGH RES REQD
        SHL     BX,1                   ;PIXEL VALUE = COL * 16
        SHL     DL,1                   ;MULTIPLY COL TIMES 2
;
V33:    MOV     AH,1                   ;SET BIT TO INDICATE POSN INFO AVAILABLE
V34:    PUSH    DX                     ;SAVE ROW,COL FROM ABOVE
        MOV     DX,CRTC_BASE_ADR       ;GET BASE ADR
        ADD     DX,7                   ;         
        OUT     DX,AL                  ;RESET LIGHTPEN TRIGGER
        POP     DX                     ;RECOVER ROW,COL
V35:    MOV     SP,BP                  ;SKIP RESTORING DX,CX,BX REGS
        JMP     MOD_RET2               ;MODIFIED RETURN
RD_LPEN_POSN    ENDP
;
;       AMOUNT TO BE SUBTACTED FROM ADDR VALUES OF LIGHTPEN REGS
;       DUE TO SYNC PROBLEM.
;
SUBAMT  LABEL   BYTE
        DB      3                      ;AMOUNT TO SUBTRACT FOR MODE 0
        DB      3                      ;                            1
        DB      5                      ;                            2
        DB      5                      ;                            3
        DB      3                      ;                            4
        DB      3                      ;                            5
        DB      3                      ;                            6
        DB      4                      ;                            7
       

                                        
;******************************************************************************
;
; SELECT DISPLAY PAGE
;     THIS ROUTINE SELECTS THE PAGE TO BE DISPLAYED
;               
;     INPUT TO ROUTINE :
;     AL = NEW ACTIVE DISPLAY PAGE
;     DS = DATA SEGMENT
;     ES = DISPLAY BUFFER SEGMENT
;
;     OUTPUT FROM ROUTINE :
;     NONE                                      
;          
;     OTHER REGS DESTROYED :
;     AX,BX,DX
;
;******************************************************************************

SEL_DISP_PAGE   PROC    NEAR           
        XOR     AH,AH
        MOV     CURRENT_PAGE,AL        ;SAVE THE NEW DISPLAY PAGE
        PUSH    AX                     ;SAVE PAGE VALUE
        MUL     DBFR_SIZE              ;START ADDR = PAGE * BFR SIZE
        MOV     DBFR_START,AX          ;SAVE START ADDR
        SHR     AX,1                   ;DIVIDE BY 2 FOR CRTC
        MOV     BL,12                  ;SETUP FOR CRTC REGS R12,R13
        CALL    CRTC_OUT2              ;OUTPUT ADDRESS TO CRTC R12,R13
        POP     BX                     ;GET PAGE VALUE
        ADD     BX,BX                  ;MULTIPLY BY 2 FOR TABLE INDEX 
        MOV     DX,CURSOR_POSITION[BX] ;GET ROW,COL OF CURSOR FROM TABLE
        CALL    SET_CURSOR_REG         ;SET THE CURSOR
        RET     
SEL_DISP_PAGE   ENDP

                                        
;******************************************************************************
;
; SCROLL SCREEN UP        
;     THIS ROUTINE SCROLLS UP A BLOCK OF CHARACTERS ON THE SCREEN.
;     VACATED LINES ARE FILLED WITH BLANK CHARACTERS.
;               
;     INPUT TO ROUTINE :
;     AH = CURRENT VIDEO MODE
;     AL = # OF ROWS TO SCROLL UP (IF 0  THE ENTIRE BLOCK IS BLANKED)
;     CX = ROW,COL COORDINATE OF THE UPPER LEFT CORNER OF BLOCK
;     DX = ROW,COL COORDINATE OF THE LOWER RIGHT CORNER OF BLOCK
;     BH = ATTRIBUTE OF THE BLANK CHAR TO BE USED FOR THE VACATED LINES
;     DS = DATA SEGMENT
;     ES = DISPLAY BUFFER SEGMENT
;
;     OUTPUT FROM ROUTINE :
;     NONE                                                     
;          
;     OTHER REGS DESTROYED :
;     ALL
;
;******************************************************************************

SCROLL_SCR_UP   PROC    NEAR
	CALL	ADJUST_BL
        CALL    CHK_MODE               ;CHECK THE MODE
        JNC     GRAPH_SCR_UP           ;GRAPHICS MODE HANDLE SEPERATELY
;
;       SCROLL UP ALPHA MODE
;
        PUSH    DX                     ;SAVE LOWER RIGHT COORDINATE
        MOV     DX,CX                  ;GET UPPER LEFT COORDINATE
        CALL    BFR_PG_POSN            ;CONVERT UPPER LEFT COORDINATE TO OFFSET
        MOV     SI,DX                  ;SAVE OFFSET
        MOV     DI,SI                  
        POP     DX                     ;RECOVER LOWER RIGHT COORDINATE
        CALL    SCR_SETUP              ;SETUP THE VARIABLES FOR SCROLL
        ADD     SI,AX                  ;COMPUTE SOURCE ADDR
;
;       COMMON SECTION FOR SCROLL UP/DWN ALPHA MODE
;
SCR_COMMON:
        SUB     DH,BL                  ;DETERMINE # OF ROWS TO MOVE
        TEST    VIDEO_MODE,04H         ;DETERMINE IF VIDEO MUST BE TURNED OFF
        JNZ     V40                    ;SKIP VIDEO OFF FOR MODES 4,5,6,7
        TEST    VIDEO_MODE,02H         
        JZ      V40                    ;SKIP VIDEO OFF FOR MODES 0,1
        CALL    VIDEO_OFF              ;TURN OFF VIDEO FOR MODES 2,3
;
V40:    PUSH    DS                     ;SAVE DATA SEGMENT
        PUSH    ES                     ;SETUP SEG REGS FOR MOVE OPERATION
        POP     DS
        OR      BL,BL                  ;BLANK ENTIRE WINDOW?
        JNZ     V41                    ;NO - GO DO THE MOVE
        ADD     DH,BL                  ;YES - SETUP TO CLEAR ALL ROWS
        JMP     V42                    ;GO TO CLEAR BLOCK
V41:    CALL    MOVE_BLK               ;MOVE THE BLOCK
        MOV     DH,BL                  ;SETUP # OF ROWS TO CLEAR
V42:    MOV     AH,BH                  ;GET THE BLANK CHAR ATTRIBUTE
        MOV     AL,20H                 ;SETUP BLANK CHAR
        CALL    CLEAR_BLK              ;CLEAR THE VACATED ROWS
        POP     DS                     ;RECOVER DATA SEGMENT
        CMP     VIDEO_MODE,7           ;MONOCHROME?
        JNE     V43                    ;NO - NEED TO FIXUP MODE REG
        RET                            ;YES - ALL DONE , RETURN
;
V43:    MOV     DX,CRTC_BASE_ADR       ;GET CRTC BASE ADR
        ADD     DX,4                   ;POINT TO MODE REG
        MOV     AL,VIDEO_MODE_REG      ;GET THE MODE REG VALUE
        OUT     DX,AL                  ;OUPUT TO MODE REG
        RET
;
;       SCROLL UP GRAPHICS MODE
;
GRAPH_SCR_UP:        
        PUSH    DX                     ;SAVE LOWER RIGHT CORNER COORDINATE
        MOV     DX,CX                  ;GET UPPER LEFT COORDINATE
        CALL    GRAPH_POSN             ;CONVERT UPPER LEFT COORDINATE TO OFFSET
        MOV     DI,DX                  ;SAVE OFFSET AS DESTINATION ADDR
        POP     DX                     ;RECOVER LOWER RIGHT COORDINATE
        MOV     BP,80                  ;SETUP ADR INCREMENT FOR MOVE BLK SUBRTN
        SUB     DX,CX                  ;COMPUTE # OF ROW,COLS IN BLOCK
        INC     DH                     ;CORRECT FOR `0' ORIGIN
        INC     DL
        XOR     CH,CH                  ;
        MOV     CL,2                   ;SETUP SHIFT COUNT
        SHL     DH,CL                  ;MULTIPLY # OF ROWS IN BLK BY 4 SINCE
                                       ;EACH CHAR TAKES 4 LINES IN ODD AND   
                                       ;4 LINES IN EVEN.
        SHL     BL,CL                  ;MULTIPLY # OF ROWS TO SCROLL BY 4
        CMP     VIDEO_MODE,6           ;HIGH RESOLUTION?
        JE      V45                    ;YES - SKIP ADJUSTMENT
        SHL     DL,1                   ;NO - MED RESOLUTION ADJUSTMENT
        SHL     DI,1                   ;BECAUSE TWICE AS MANY BYTES PER CHAR
;
V45:    PUSH    ES                     ;SETUP SEG REGS FOR MOVE OPERATION
        POP     DS                     
        OR      BL,BL                  ;BLANK ENTIRE BLOCK?
        JZ      V47                    ;YES - GO CLEAR THE ROWS
                                       ;NO - DO THE MOVE
;
V46:    MOV     AL,BL                  ;GET # OR ROWS*4 TO SCROLL
        MOV     AH,80                  ;GET # OF COLS
        MUL     AH                     ;COMPUTE OFFSET FOR SOURCE
        MOV     SI,DI
        ADD     SI,AX
;
;       COMMON SECTION FOR SCROLL UP/DWN GRAPHICS MODE
;
GRAPH_SCR_CMN:
        SUB     DH,BL                  ;DETERMINE # OF ROWS TO MOVE
        CALL    MOVE_BLK_BYTE          ;MOVE EVEN,ODD ROWS
        MOV     DH,BL
V47:    MOV     AL,BH                  ;GET FILL VALUE
        CALL    CLEAR_BLK_BYTE         ;CLEAR EVEN,ODD ROWS
        RET
SCROLL_SCR_UP   ENDP
       
       

                                        
;******************************************************************************
;
; SCROLL SCREEN DOWN      
;     THIS ROUTINE SCROLLS DOWN A BLOCK OF CHARACTERS ON THE SCREEN.
;     VACATED LINES ARE FILLED WITH BLANK CHARACTERS.
;               
;     INPUT TO ROUTINE :
;     AH = CURRENT VIDEO MODE
;     AL = # OF ROWS TO SCROLL DOWN (IF 0  THE ENTIRE BLOCK IS BLANKED)
;     CX = ROW,COL COORDINATE OF THE UPPER LEFT CORNER OF BLOCK
;     DX = ROW,COL COORDINATE OF THE LOWER RIGHT CORNER OF BLOCK
;     BH = ATTRIBUTE OF THE BLANK CHAR TO BE USED FOR THE VACATED LINES
;     DS = DATA SEGMENT
;     ES = DISPLAY BUFFER SEGMENT
;
;     OUTPUT FROM ROUTINE :
;     NONE                                                      
;          
;     OTHER REGS DESTROYED :
;     ALL
;
;******************************************************************************


SCROLL_SCR_DWN  PROC    NEAR
	CALL	ADJUST_BL
        CALL    CHK_MODE               ;CHECK MODE
        JNC     GRAPH_SCR_DWN          ;GRAPHICS MODE HANDLE SEPERATELY
;
;       SCROLL DOWN ALPHA MODE
;
        STD                            ;SET DIR FLAG FOR AUTODEC       
        PUSH    DX                     ;SAVE LOWER RIGHT COORDINATE
        CALL    BFR_PG_POSN            ;CONVERT LOWER RIGHT COORDINATE TO OFSET
        MOV     SI,DX                  ;SAVE OFFSET
        MOV     DI,SI                  
        POP     DX                     ;RECOVER LOWER RIGHT COORDINATE
;
        CALL    SCR_SETUP              ;SETUP VARIABLES FOR SCROLL
        SUB     SI,AX                  ;COMPUTE SOURCE ADDR
        NEG     BP                     ;SETUP ADDR DECREMENT VALUE
        JMP     SCR_COMMON             ;GO TO COMMON SECTION
;
;       SCROLL DOWN GRAPHICS MODE
;
GRAPH_SCR_DWN:
        STD                            ;SET DIR FLAG FOR AUTODEC
        PUSH    DX                     ;SAVE LOWER RIGHT COORDINATE
        CALL    GRAPH_POSN             ;CONVERT LOWER RIGHT COORDINATE TO OFSET
        MOV     DI,DX                  ;SAVE OFFSET AS DEST ADDR
        POP     DX                     ;RECOVER LOWER RIGHT COORDINATE
;
        MOV     BP,-80                 ;SETUP ADDR DECREMENT FOR MOVE SUBRTN
        SUB     DX,CX                  ;COMPUTE # OF ROWS,COL IN BLOCK
        INC     DH                     ;CORRECT FOR `0' ORIGIN
        INC     DL
        XOR     CH,CH
        MOV     CL,2                   ;SETUP SHIFT COUNT
        SHL     DH,CL                  ;MULTIPLY # OF ROWS IN BLK BY 4
        SHL     BL,CL                  ;MULTIPLY # OF ROWS TO SCROLL BY 4
        CMP     VIDEO_MODE,6           ;HIGH RESOLUTION?
        JE      V50                    ;YES - SKIP ADJUSTMENT
        SHL     DL,1                   ;NO - MED RESOLUTION ADJUSTMENT
        SHL     DI,1                   ;BECAUSE TWICE AS MANY BYTES PER CHAR
        INC     DI
;
V50:    PUSH    ES                     ;SETUP SEG REGS FOR MOVE OPERATION
        POP     DS                     
        ADD     DI,240                 ;POINT TO LAST ROW OF PIXELS
        OR      BL,BL                  ;BLANK ENTIRE BLOCK?
        JZ      V47                    ;YES - GO CLEAR THE ROWS
;
V51:    MOV     AL,BL                  ;GET # OF ROWS*4 TO SCROLL
        MOV     AH,80                  ;GET # OF COLS
        MUL     AH                     ;COMPUTE OFFSET FOR SOURCE
        MOV     SI,DI
        SUB     SI,AX
        JMP     GRAPH_SCR_CMN          ;JUMP TO COMMON SECTION
SCROLL_SCR_DWN  ENDP


                                        
;******************************************************************************
;
; READ ATTRIBUTE AND CHAR
;     THIS ROUTINE READS THE ATTRIBUTE AND CHARACTER AT THE THE 
;     CURRENT CURSOR POSITION.
;               
;     INPUT TO ROUTINE :
;     AH = CURRENT VIDEO MODE
;     BH = DISPLAY PAGE
;     DS = DATA SEGMENT
;     ES = DISPLAY BUFFER SEGMENT
;
;     OUTPUT FROM ROUTINE :
;     AL = CHARACTER READ
;     AH = ATTRIBUTE READ
;
;     OTHER REGS DESTROYED :
;     ALL           
;
;******************************************************************************
       
RD_ATR_CHAR     PROC    NEAR           
        CALL    CHK_MODE               ;CHECK CURRENT MODE
        JNC     RD_GRAPH               ;JUMP IF GRAPHICS MODE  
;
;       READ CHAR ALPHA MODE
;
        SUB     CH,CH                  
        MOV     CL,BH                  ;SAVE DISP PAGE IN CX
        CALL    GET_CUR_POSN           ;GET ROW,COL COORDINATE
        CALL    BFR_POSN               ;CONVERT COORDINATE TO OFFSET
	PUSH	AX			;save crt mode in AH
        PUSH    DX                     ;SAVE OFFSET WITHIN PAGE
        MOV     AX,DBFR_SIZE           ;GET BFR SIZE
        MUL     CX                     ;MULTIPLY BFR SIZE TIMES PAGE #
        POP     BX                     ;GET OFFSET WITHIN PAGE
        ADD     BX,AX                  ;ADD TO PAGE STARTING ADDR
	pop	ax			;restore crt mode in AH
	call	CHK_80_COLOR		;see if 80x25 color
	jnz	V52			;no, go around retrace checking
        CALL    WAIT_HRZ_RETRACE       ;WAIT FOR HORIZONTAL RETRACE LOW
	CALL	WAIT_FOR_RETRACE	;WAIT FOR RETRACE ACTIVE
;V52:    IN      AL,DX                  ;WAIT FOR HORIZONTAL RETRACE HIGH
;        AND     AL,1
;        JZ      V52
V52:	MOV     AX,ES:[BX]             ;READ ATTRIBUTE AND CHAR
        RET
;
;       READ CHAR GRAPHICS MODE
;
RD_GRAPH:
        MOV     DX,CURSOR_POSITION     ;GET CURSOR POSITION
        CALL    GRAPH_POSN             ;CONVERT ROW,COL TO OFFSET
        MOV     DI,DX                  ;SAVE OFFSET
        SUB     SP,8                   ;USER STACK AREA TO TEMPORARILY
                                       ;STORE PIXEL BYTES      
        MOV     SI,SP
        PUSH    SI
        MOV     CX,4                   ;SETUP LOOP COUNT
        PUSH    ES                     ;SET DS TO POINT TO DISP BFR SEG
        POP     DS
        CMP     AH,6                   ;HIGH RESOLUTION?
        JE      V56                    ;YES - GO HANDLE SEPERATELY
;
;       MEDIUM RESOLUTION
;
        SHL     DI,1 
V55:    MOV     AX,[DI]                ;GET EVEN 2-BYTE PIXEL
        XCHG    AH,AL
        CALL    PXEL_TO_DOT            ;CONVERT IT TO DOT PATTERN
        MOV     BL,AL                  ;SAVE EVEN DOT PATTERN
        MOV     AX,[DI+2000H]          ;GET ODD 2-BYTE PIXEL
        XCHG    AH,AL
        CALL    PXEL_TO_DOT            ;CONVERT IT TO DOT PATTERN
        MOV     BH,AL                  ;BX NOW HAS THE DOT PATTERN FOR ODD,EVEN
        MOV     SS:[SI],BX             ;STORE IN STACK AREA
        ADD     SI,2                   ;POINT TO NEXT STACK WORD
        ADD     DI,80                  ;POINT TO NEXT PIXEL
        LOOP    V55                    ;DO IT 4 TIMES
        JMP     V57                    ;GO COMPARE PIXEL BYTES AGAINST TABLE
;
;       HIGH RESOLUTION
;
V56:    MOV     AL,[DI]                ;GET EVEN PIXEL BYTE
        MOV     AH,[DI+2000H]          ;GET ODD PIXEL BYTE
        MOV     SS:[SI],AX             ;PUT IT IN STACK AREA
        ADD     SI,2                   ;POINT TO NEXT WORD IN STACK
        ADD     DI,80                  ;POINT TO NEXT PIXEL BYTE
        LOOP    V56                    ;DO IT 4 TIMES
;
;
;       COMPARE STACK AREA(WHICH CONTAINS DOT PATTERN) WITH
;       CHARACTER GEN TABLE
;
V57:    POP     DI                     ;POINT TO BEGINING OF STACK AREA
        MOV     SI,OFFSET CHAR_GEN_TABLE   ;POINT TO CHARACTER GEN TABLE
        XOR     AL,AL
        PUSH    CS
        POP     DS                     ;SETUP DS TO POINT TO CODE SEGMENT
V58:    PUSH    SS                     ;SETUP ES TO POINT TO STACK SEGMENT
        POP     ES
        MOV     BL,128                 ;SETUP LOOP COUNT
V59:    MOV     CX,4                   ;SETUP # OR WORDS TO COMPARE(STRING OP)
        PUSH    SI                     ;SAVE POINTERS
        PUSH    DI
        REPE    CMPSW                  ;DO THE COMPARE
        POP     DI                     ;RECOVER POINTERS
        POP     SI
        JZ      V60                    ;DID MATCH OCCUR ?
                                       ;YES - FIXUP STACK AND RETURN
        ADD     SI,8                   ;NO - COMPARE NEXT ENTRY
        INC     AL                     ;NEXT CHAR
        JZ      V60                    ;DO ALL THE 255 CHARS
        DEC     BL
        JNZ     V59
;
;       CHECK IF THERE IS A USER SUPPLIED CHAR GEN TABLE
;
        PUSH    AX                     ;SAVE REGS
        XOR     AX,AX                  ;SETUP TO POINT TO ZERO SEGMENT
        MOV     DS,AX
        ASSUME DS:ZERO
        LES     SI,USER_CGEN_TABLE     ;GET POINTERS
        PUSH    ES
        POP     DS
        MOV     AX,DS                  ;IS POINTER 0
        OR      AX,SI
        POP     AX
        JNZ     V58                    ;IF POINTER IS 0 THEN RETURN
                                       ;ELSE COMAPARE AGAINST USER TABLE  
V60:    ADD     SP,8                   ;FIXUP STACK PTR AND RETURN
        RET
RD_ATR_CHAR     ENDP


                                        
;******************************************************************************
;
; WRITE CHARACTER
; WRITE ATTRIBUTE AND CHARACTER
;     THIS ROUTINE WRITES THE CHARACTER OR CHARACTER AND ATTRIBUTE
;     AT THE CURRENT CURSOR POSITION
;               
;     INPUT TO ROUTINE :
;     AH = CURRENT VIDEO MODE
;     AL = CHARACTER TO WRITE
;     BL = ATTRIBUTE OF CHAR 
;     BH = DISPLAY PAGE
;     CX = COUNT OF CHARACTERS TO WRITE
;     DS = DATA SEGMENT
;     ES = DISPLAY BUFFER SEGMENT
;
;     OUTPUT FROM ROUTINE :
;     NONE                                    
;          
;     OTHER REGS DESTROYED :
;     ALL            
;
;******************************************************************************

        ASSUME CS:EPROMA,DS:DATAA,ES:DISP_BFR
WR_CHAR         PROC    NEAR
        XOR     BP,BP                  ;CLEAR BP TO INDICATE CHAR ONLY 
        JMP     SHORT V65              ;GO TO COMMON SECTION
;
WR_ATR_CHAR     PROC    NEAR
        MOV     BP,1                   ;SET BP TO INDICATE CHAR AND ATTRIBUTE
;
;       COMMON FOR CHAR, CHAR AND ATTRIBUTE
;
V65:    CALL    CHK_MODE               ;GRAPHICS MODE?           
        JNC     WR_GRAPH               ;YES - GO HANDLE SEPERATELY
;
;       WRITE CHAR ALPHA MODE
;
	push	ax			;save crt mode in AH
        PUSH    CX                     ;SAVE COUNT
        MOV     AH,BL                  
        PUSH    AX                     ;SAVE ATTRIBUTE AND CHAR
        SUB     CH,CH                  ;SAVE DISP PAGE IN CX
        MOV     CL,BH
        CALL    GET_CUR_POSN           ;GET ROW,COL COORDINATE OF CURSOR
        CALL    BFR_POSN               ;CONVERT COORDINATE TO OFFSET
        PUSH    DX                     ;SAVE OFFSET WITHIN PAGE
        MOV     AX,DBFR_SIZE           ;GET BFR SIZE
        MUL     CX                     ;MULTIPLY BFR SIZE TIMES PAGE #
        POP     DI                     ;GET OFFSET WITHIN PAGE
        ADD     DI,AX                  ;ADD TO PAGE STARTING ADDR
        POP     BX                     ;RECOVER ATR & CHAR
        POP     CX                     ;RECOVER COUNT
	pop	ax			;restore crt mode in AH
        OR      BP,BP                  ;CHAR ONLY ?
        JZ      V67                    ;YES - GO OUTPUT CHAR ONLY
;
V66:	call	CHK_80_COLOR		;see if 80x25 color
	jnz	V66B			;no, go around retrace checking
V66A:	CALL    WAIT_HV_RETRACE       ;WAIT HORIZONTAL RETRACE LOW
	jnz	V66C			;vertical retrace, don't check horizontal retrace
	CALL	WAIT_FOR_RETRACE	;WAIT FOR RETRACE ACTIVE
;V66A:   IN      AL,DX                  ;WAIT HORIZONTAL RETRACE HIGH
;        AND     AL,1
;        JZ      V66A
V66C:	MOV     AX,BX                  ;GET ATTRIBUTE/CHAR
        STOSW                          ;OUTPUT ATTRIBUTE AND CHAR TO BFR
;        STI                            ;RENABLE INTRERRUPTS
        LOOP    V66A                    ;KEEP DOING TILL COUNT=0
        RET
V66B:
        MOV     AX,BX                  ;GET ATTRIBUTE/CHAR
        REP	STOSW			;OUTPUT ATTRIBUTE AND CHAR TO BFR
        RET
;
V67:	call	CHK_80_COLOR		;see if 80x25 color
	jnz	V67B			;no, go around retrace checking
V67A:	CALL    WAIT_HV_RETRACE       ;WAIT HORIZONTAL RETRACE LOW
	jnz	V67D			;vertical retrace, don't check horizontal retrace
	CALL	WAIT_FOR_RETRACE	;WAIT FOR RETRACE ACTIVE
;V68:    IN      AL,DX                  ;WAIT HORIZONTAL RETRACE HIGH
;        AND     AL,1
;        JZ      V68
V67D:	MOV     AX,BX                  ;GET CHAR
        STOSB                          ;OUTPUT CHAR ONLY TO DISP BFR
;        STI                            ;RENABLE INTERRUPTS
        INC     DI                     ;SKIP ATTRIBUTE BYTE
        LOOP    V67A                    ;KEEP DOING TILL COUNT=0
        RET
V67B:
        MOV     AX,BX                  ;GET CHAR
V67C:
        STOSB                          ;OUTPUT CHAR ONLY TO DISP BFR
        INC     DI                     ;SKIP ATTRIBUTE BYTE
        LOOP    V67C                    ;KEEP DOING TILL COUNT=0
        RET
;
;       WRITE CHAR GRAPHICS MODE
;
WR_GRAPH:
        MOV     DX,CURSOR_POSITION     ;GET THE ROW,COL OF CURRENT CURSOR
        CALL    GRAPH_POSN             ;CONVERT ROW,COL TO OFFSET
        MOV     DI,DX                  ;SAVE OFFSET
        CMP     AH,6                   ;HIGH RESOLUTION MODE?
        JE      V70                    ;YES - SKIP ADJUSTMENT
        SHL     DI,1                   ;NO -MED RES ADJUSTMENT - OFFSET*2
;
V70:    MOV     SI,OFFSET CHAR_GEN_TABLE ;GET ADDR OF CHAR GEN TABLE
        MOV     DX,CS                  ;ESTABLISH SEGMENT
        MOV     DS,DX
        OR      AL,AL                  ;IS BIT 7 ON?
        JNS     V71                    ;NO -  FIRST HALF
                                       ;YES - SECOND HALF
        AND     AL,07FH                ;STRIP BIT 7     
        XOR     DX,DX
        MOV     DS,DX                  ;POINT TO ZERO SEGMENT
        ASSUME DS:ZERO
        LDS     SI,USER_CGEN_TABLE     ;GET OFFSET AND SEGMENT OF USER SUPPLIED
                                       ;TABLE
        ASSUME DS:DATAA
V71:    MOV     BH,AH                  ;SAVE MODE
        MOV     AH,8                   ;MULTIPLY*8 TO INDEX INTO CHAR GEN TABLE
        MUL     AH                     
        ADD     SI,AX                  ;ADD TO BASE ADDR
        CMP     BH,6                   ;HIGH RESOLUTION MODE
        JE      V74                    ;GO HANDLE HIGH RES SEPERATELY
;
;       MEDIUM RESOLUTION
;
        MOV     DL,BL                  ;SAVE COLOR BYTE BIT 7 FOR TEST LATER
        AND     BL,03H                 ;ISOLATE BITS 0-1
;
V72:    PUSH    SI                     ;SAVE POINTERS
        PUSH    DI
        MOV     BH,4                   ;SETUP LOOP COUNT
;
V73:    CALL    WRITE_PXEL             ;WRITE EVEN BYTES
        ADD     DI,2000H               ;POINT TO ODD
        CALL    WRITE_PXEL             ;WRITE ODD BYTES
        SUB     DI,2000H-80            ;NEXT LINE
        DEC     BH                     ;ALL DONE?
        JNZ     V73                    ;NO - KEEP DOING
        POP     DI                     ;YES - RECOVER POINTERS
        POP     SI
        ADD     DI,2                   ;POINT TO NEXT CHAR
        LOOP    V72                    ;LOOP TILL ALL CHARS ARE WRITTEN
        RET
;
;       HIGH RESOLUTION
;
V74:    PUSH    SI                     ;SAVE POINTERS
        PUSH    DI
        MOV     BH,4                   ;SETUP LOOP COUNT 
;
V75:    LODSW                          ;GET THE DOT PATTERN
        OR      BL,BL                  ;IS XOR BIT ON?
        JNS     V76                    ;NO - SKIP THE XOR'ING
        XOR     AL,ES:[DI]             ;YES - XOR WITH EXISTING PATTERN
        XOR     AH,ES:[DI+2000H]
;
V76:    MOV     ES:[DI],AL             ;STORE EVEN BYTE
        MOV     ES:[DI+2000H],AH       ;STORE ODD BYTE
        ADD     DI,80                  ;POIMT TO NEXT LINE
        DEC     BH
        JNZ     V75                    
        POP     DI
        POP     SI
        INC     DI                     ;POINT TO NEXT CHAR
        LOOP    V74                    ;KEEP DOING TILL ALL CHARS WRITTEN
        RET
WR_ATR_CHAR     ENDP
WR_CHAR         ENDP


                                        
;******************************************************************************
;
; SET COLOR PALLETTE
;     THIS ROUTINE SETS UP THE COLOR FOR MEDIUM RESOLUTION GRAPHICS.
;               
;     INPUT TO ROUTINE :
;     BH = COLOR ID
;              O  BACKGROUND COLOR SET BASED ON BL BITS 0-4
;              1  PALETTE SELECTION SET BASED ON BL BIT 0 : O GREEN,RED,YELLOW
;                                                           1 BLUE,CYAN,MAGENTA
;     BL = BACKGROUND COLOR CODE/ COLOR SET SELECT
;
;     OUTPUT FROM ROUTINE :
;     NONE
;          
;     OTHER REGS DESTROYED :
;     AX,BX,DX   
;       
;******************************************************************************
       
SET_COLOR_PLT   PROC    NEAR
        MOV     AL,CRT_COLOR           ;GET THE CURRENT COLOR SETTING
        CMP     BH,0                   ;TEST FOR BACKGROUND
        JE      V81                    ;JUMP IF BACKGROUND
;       COLOR SET SELECT
        TEST    BL,01                  ;TEST IF COLOR SELECT SET
        JZ      SHORT V80
        OR      AL,20H                 ;SET COLOR SELECT BIT
        JMP     SHORT V82
V80:    AND     AL,0DFH                ;RESET COLOR SELECT BIT
        JMP     V82
;       BACKROUND
V81:    AND     BL,01FH                ;ISOLATE BACGROUND COLOR BITS
        AND     AL,0E0H                ;ZEROUT THE CURRENT BACKGROUND COLOR
        ADD     AL,BL                  ;MERGE THE NEW BACKCROUND COLOR
;
V82:    MOV     CRT_COLOR,AL           ;SAVE VALUE
        MOV     DX,CRTC_BASE_ADR       ;GET BASE ADDR
        ADD     DX,5                   ;POINT TO COLOR SELECT REG
        OUT     DX,AL                  ;OUTPUT TO COLOR SELECT REG
        RET
SET_COLOR_PLT   ENDP



                                        
;******************************************************************************
;                                      
; WRITE DOT         
;     THIS ROUTINE WRITES A DOT AT THE SPECIFIED ROW,COL     
;     COORDINATE. THE COORDINATE IS EXPRESSED IN PIXEL VALUES.
;               
;     INPUT TO ROUTINE :
;     AL = DOT VALUE TO BE WRITTEN (BIT 7 =1 CAUSES VALUE TO BE
;                                   XORRED WITH EXISTING VALUE)
;     DX = ROW COORDINATE (EXPRESSED IN PIXEL VALUES 0 - 199)
;     CX = COL COORDINATE (EXPRESSED IN PIXEL VALUES 0 - 639)
;     DS = DATA SEGMENT
;     ES = DISPLAY BUFFER SEGMENT
;
;     OUTPUT FROM ROUTINE :
;     NONE 
;          
;     OTHER REGS DESTROYED :
;     XX,XX,XX,XX
;  
;******************************************************************************
       
WR_DOT  PROC    NEAR
        PUSH    AX                     ;SAVE REGS
        CALL    DOT_POSN               ;GET THE LOCATION IN DISP BFR        
                                       ;GET THE MASK AND SHIFT AMOUNT
        MOV     DL,AL                  ;MAKE A COPY OF AL (BIT 7)FOR TEST LATER
        AND     AL,AH                  ;ISOLATE THE REQUIRED BITS
        SHL     AL,CL                  ;ALIGN IT
        SHL     AH,CL                  ;SHIFT THE MASK
        XOR     AH,0FFH                ;FLIP THE MASK
        MOV     BL,ES:[DI]             ;GET THE EXISTING BYTE
        OR      DL,DL                  ;IS BIT 7 ON?
        JNS     V85                    ;NO - SKIP THE XORRING
        XOR     AL,BL                  ;YES - XOR WITH EXISTING VALUE
        JMP     V86
V85:    AND     BL,AH                  ;ZEROUT THE AFFECTED BITS
        OR      AL,BL                  ;OR IN THE NEW VALUE
V86:    MOV     ES:[DI],AL             ;STORE IN DISP BFR
        POP     AX                     ;RESTORE REGS
        RET
WR_DOT  ENDP


                                        
;******************************************************************************
;
; READ DOT          
;     THIS ROUTINE READS THE DOT VALUE AT THE SPECIFIED ROW,COL
;     COORDINATE. THE COORDINATE IS EXPRESSED IN PIXEL VALUES.
;               
;     INPUT TO ROUTINE :
;     DX = ROW COORDINATE (EXPRESSED IN PIXEL VALUES 0 - 199)
;     CX = COL COORDINATE (EXPRESSED IN PIXEL VALUES 0 - 639)
;     DS = DATA SEGMENT
;     ES = DISPLAY BUFFER SEGMENT
;
;     OUTPUT FROM ROUTINE :
;     AL = DOT VALUE
;          
;     OTHER REGS DESTROYED :
;     DI,CX,XX
;
;******************************************************************************

RD_DOT  PROC    NEAR
        CALL    DOT_POSN               ;GET THE LOCATION IN DISP BFR
        MOV     AL,ES:[DI]             ;GET MASKS AND SHIFT AMOUNT
        SHR     AL,CL                  ;RIGHT JUSTIFY REQUIRED BITS
        AND     AL,AH                  ;ISOLATE THEM
        RET
RD_DOT  ENDP


                                        
;******************************************************************************
;
; WRITE TELETYPE    
;     THIS ROUTINE PROVIDES A TELETYPE INTERFACE. THE INPUT CHARACTER IS
;     WRITTEN AT THE CURRENT CURSOR POSITION AND THE CURSOR IS MOVED TO 
;     THE NEXT POSITION. IF THE CURSOR REACHES THE END OF COLUMN, THE      
;     THE COLUMN IS SET TO ZERO AND THE ROW IS INCREMENTED(START OF NEXT
;     LINE). IF THE ROW REACHES THE END OF ROW , THE SCREEN IS SCROLLED
;     UP ONE LINE AND THE CURSOR IS POSITIONED ON THE LAST ROW,FIRST COLUMN.
;               
;     INPUT TO ROUTINE :
;     AH = CURRENT VIDEO MODE
;     AL = CHARACTER TO WRITE
;     BL = FOREGROUND COLOR (VALID IN GRAPHICS MODE)
;     BH = DISPLAY PAGE
;
;     OUTPUT FROM ROUTINE :
;     NONE
;          
;     OTHER REGS DESTROYED :
;     XX,XX,XX 
;
;******************************************************************************
       
WR_TELETYPE     PROC    NEAR
        MOV     BH,CURRENT_PAGE
        PUSH    AX                     ;SAVE THE CHAR
        MOV     AH,3                   ;GET THE CURRENT CURSOR POSITION
        INT     10H                    ;CALL THE VIDEO ROUTINE TO GET CUR POSN
        POP     AX                     ;RECOVER CHAR
HANDLE_SPECIAL:
        PUSH    AX                     ;SAVE CHAR AGAIN
;
;       CHECK FOR SPECIAL CHARACTERS THAT NEED SPECIAL HANDLING
;
        CMP     AL,07                  ;CHECK FOR BEEP
        JNE     V90                    ;JUMP IF NOT BEEP
;
;       BEEP CHAR
        MOV     BL,0C0H                ;SETUP THE TIME DURATION FOR BEEP
        CALL    BEEP                   ;DO THE BEEP
        POP     AX                     ;RECOVER CHAR
        RET
;
V90:    CMP     AL,8                   ;CHECK FOR BACKSPACE
        JNE     V91                    ;JUMP IF NOT BACKSPACE
;
;       BACKSPACE CHAR
        OR      DL,DL                  ;BEGINING OF LINE ?
        JZ      V96                    ;YES - CANT DO ANYTHING
        DEC     DL                     ;NO - BACKUP CURSOR
        JMP     V96                    ;GO SET THE CURSOR  
;
V91:    CMP     AL,0AH                 ;CHECK FOR LINE FEED
        JE      V93                    ;BRANCH IF LINE FEED
        CMP     AL,0DH                 ;CHECK FOR CARRAIGE RETURN
        JNE     V92                    ;BRANCH IF NOT CARRIAGE RETURN
;
;       CARRIAGE RETURN
        MOV     DL,0                   ;SET COLUMN TO 0
        JMP     V96                    ;GO SET THE CURSOR
;
V92:    MOV     CX,1                   ;SET UP COUNT OF 1
        MOV     AH,10                  ;SETUP FOR WRITE CHAR ONLY
        INT     10H                    ;CALL VIDEO ROUTINE TO WRITE THE CHAR-AL
UPDATE_CURSOR:
        INC     DL                     ;INCREMENT COLUMN
        CMP     DL,BYTE PTR DISP_COLS  ;END OF COLUMNS
        JNZ     V96
        MOV     DL,0                   ;NEXT LINE
V93:    CMP     DH,24                  ;END OF ROWS?
        JNZ     V95                    ;NO - GO INC AND SET CURSOR
                                       ;YES - SCROLLING IS NECESSARY
        MOV     AH,2
        INT     10H                    ;CALL THE VIDEO ROUTINE TO SET THE
                                       ;CURSOR POSN
        MOV     AH,VIDEO_MODE          
        CALL    CHK_MODE               ;GRAPHICS MODE?
        JNC     V94                    ;BRANCH IF GRAPHICS
        MOV     AH,8                   ;SETUP TO READ ATR,CHAR AT CURRENT CURS
        INT     10H                    ;CALL VIDEO ROUTINE TO READ ATR,CHAR
        MOV     BH,AH                  ;SETUP ATTRIBUTE
V94:    XOR     CX,CX                  ;UPPER LEFT CORNER (0,0)
        MOV     DH,24                  ;LOWER RIGHT CORNER
        MOV     DL,BYTE PTR DISP_COLS
        DEC     DL                     ;CORRECT FOR 0 ORIGIN
        MOV     AL,01                  ;SETUP FOR 1 LINE SCROLL
        MOV     AH,06                  ;SETUP SCROLL COMMAND
        INT     10H                    ;CALL THE VIDEO ROUTINE TO DO THE SCROLL
        POP     AX                     ;RESTORE CHAR
        RET
;
V95:    INC     DH                     ;INC ROW
V96:    MOV     AH,2                   ;SETUP TO SET CURSOR POSITION
        INT     10H                    ;CALL VIDEO ROUTINE TO SET CURSOR POSN
        POP     AX                     ;RESTORE CHAR
        RET
WR_TELETYPE     ENDP


                                        
;******************************************************************************
;
; READ CURRENT VIDEO STATE
;     THIS ROUTINE RETURNS THE CURRENT VIDEO STATE.
;               
;     INPUT TO ROUTINE :
;     NONE
;
;     OUTPUT FROM ROUTINE:
;     AL = CURRENT VIDEO MODE SETTING
;     AH = # OF DISPLAY COLUMNS ON THE SCREEN
;     BH = CURRENT ACTIVE DISPLAY PAGE
;       
;     OTHER REGS DESTROYED :
;     NONE     
;
;******************************************************************************
       
RD_VIDEO_STATE  PROC    NEAR
        MOV     AL,VIDEO_MODE          ;GET CURRRENT MODE
        MOV     BH,CURRENT_PAGE        ;GET CURRENT PAGE
        MOV     AH,BYTE PTR DISP_COLS  ;GET # OF DISP COLUMNS
        POP     DX                     ;THROW AWAY CALLERS RETURN ADDRESS
        POP     DX                     ;RESTORE DX
        POP     CX                     ;RESTORE CX
        MOV     SP,BP                  ;SKIP RESTORING BX
        JMP     MOD_RET2               ;MODIFIED RETURN
RD_VIDEO_STATE  ENDP


                                        
;******************************************************************************
;
; CHECK MODE        
;     THE ROUTINE CHECKS THE MODE CONTAINED IN AH AND SETS THE 
;     CARRY, ZERO FLAGS ACCORDINGLY.
;               
;     INPUT TO ROUTINE :
;     AH =  VIDEO MODE (0-7)
;
;     OUTPUT FROM ROUTINE :
;     CARRY = 1  IF AH = 0,1,2,3,7 (ALPHA MODE)
;             0  IF AH = 4,5,6 (GRAPHICS)
;     ZERO  = 1  IF AH = 7
;
;     OTHER REGS DESTROYED :
;     NONE
;
;******************************************************************************

CHK_MODE        PROC    NEAR
        CMP     AH,4                   ;IS IT 0,1,2,3?
        JNC     V100                   ;NO - GO AROUND RETURN
        RET                            ;YES - JUST RETURN
V100:   CMP     AH,7                   ;SET ZERO FLG IF 7
        CMC                            ;RESET CARRY
        RET
CHK_MODE        ENDP

;******************************************************************************
;
; CHECK IF 80X25 BW OR 80X25 COLOR ON COLOR CARD
;     THE ROUTINE CHECKS THE MODE CONTAINED IN AH AND SETS THE 
;     ZERO FLAGS ACCORDINGLY.
;               
;     INPUT TO ROUTINE :
;     AH =  VIDEO MODE (0-7)
;
;     OUTPUT FROM ROUTINE :
;     ZERO = 1  IF AH = 2,3 (80X25 COLOR CARD)
;            0  IF AH = 0,1,7 (ELSE)
;
;     OTHER REGS DESTROYED :
;     NONE
;
;******************************************************************************

CHK_80_COLOR	PROC    NEAR
        CMP     AH,2                   ;IS IT 80X25 BW ON COLOR CARD?
        JE      V101                   ;YES, REYURN
	CMP     AH,3                   ;IS IT 80X25 COLOR ON COLOR CARD?
V101:	RET
CHK_80_COLOR	ENDP


                                        
;******************************************************************************
;
; CTRC OUTPUT (1 OR 2 BYTES)
;     THESE ROUTINES OUTPUT  TO THE CRTC(6845) REGISTER(S).
;     CRTC_OUT2 - OUTPUTS AH TO THE CRTC(6845) REGISTER SPECIFIED IN BL
;                 OUTPUTS AL TO THE CRTC(6845) REGISTER (BL)+1
;       
;     CRTC_OUT -  OUTPUTS AH TO THE CRTC(6845) REGISTER SPECIFIED IN BL
;               
;     INPUT TO ROUTINE :
;     AH,AL = VALUES TO BE OUTPUT (AL VALID ONLY FOR CRTC_OUT2)
;     BL = 6845 REGISTER NUMBER
;
;     OUTPUT FROM ROUTINE :
;     NONE
;          
;     OTHER REGS DESTROYED :
;     CRTC_OUT2 - AX,BL,DX
;     CRTC_OUT  - AL,DX
;******************************************************************************
       
CRTC_OUT2       PROC    NEAR           
        MOV     DX,CRTC_BASE_ADR       ;GET CRTC BASE ADDR
        XCHG    AL,BL                  ;AL=REG NUMBER, BL=DATA(LSB)
        OUT     DX,AL                  ;OUTPUT REG NUMBER
	jmp	short $+2
        XCHG    AL,AH                  ;AL=  DATA(MSB), AH= REG NUMBER
        INC     DX                     ;POINT TO DATA PORT
        OUT     DX,AL                  ;OUTPUT DATA (MSB)
        XCHG    AH,BL                  ;AH=DATA(LSB), BL=REG NUMBER
        INC     BL                     ;NEXT REG
CRTC_OUT        PROC    NEAR           
        MOV     DX,CRTC_BASE_ADR       ;GET BASE ADDR
CRTC_OUTX:
        MOV     AL,BL
        OUT     DX,AL                  ;OUTPUT REG NUMBER
	jmp	short $+2
        MOV     AL,AH                  ;
        INC     DX                     ;POINT TO DATA PORT
        OUT     DX,AL                  ;OUTPUT DATA(LSB)
        RET
CRTC_OUT        ENDP
CRTC_OUT2       ENDP


                                        
;******************************************************************************
;
; CRTC INPUT        
;     THIS ROUTINE READS THE  CRTC(6845) REGISTER SPECIFIED IN BL.
;               
;     INPUT TO ROUTINE :
;     BL = NUMBER OF THE REGISTER TO BE READ
;
;     OUTPUT FROM ROUTINE :
;     AL = REGISTER VALUE READ
;          
;     OTHER REGS DESTROYED :
;     DX
;
;******************************************************************************
       
CRTC_IN         PROC    NEAR
        MOV     DX,CRTC_BASE_ADR       ;GET BASE ADDR
        MOV     AL,BL
        OUT     DX,AL                  ;OUTPUT REG NUMBER
	jmp	short $+2
        INC     DX                     ;POINT TO DATA PORT
        IN      AL,DX                  ;READ THE REG
        RET
CRTC_IN         ENDP


                                        
;******************************************************************************
;
; BUFFER POSITION   
;     THIS ROUTINE CALCULATES THE DISPLAY BUFFER POSITION(OFFSET)
;     OF A CHARACTER,GIVEN THE ROW,COL COORDINATE.
;               
;     INPUT TO ROUTINE :
;     DH = ROW COORDINATE OF CHAR
;     DL = COL COORDINATE OF CHAR      
;
;     OUTPUT FROM ROUTINE :
;     DX = OFFSET OF THE CHAR IN THE DISPLAY BUFFER
;          
;     OTHER REGS DESTROYED :
;     NONE
;
;******************************************************************************

BFR_POSN        PROC    NEAR           
        PUSH    AX                     ;SAVE REGS
        MOV     AL,DH                  ;GET ROW COORDINATE        
        XOR     DH,DH
        MUL     BYTE PTR DISP_COLS     ;MULTIPLY ROWS TIMES NO. OF DISP COLUMNS
        ADD     DX,AX                  ;ADD IN COLUMN COORDINATE
        ADD     DX,DX                  ;ADJUST FOR ATTRIBUTE BYTE
        POP     AX                     ;RESTORE REG
        RET
BFR_POSN        ENDP


;
;     THIS ROUTINE IS THE SAME AS THE ABOVE(BFR_POSN) ROUTINE
;     EXCEPT THAT THE CURRENT PAGE START ADDRESS IS ADDED
;     TO THE CALCULATED OFFSET.
;

BFR_PG_POSN     PROC    NEAR           
        CALL    BFR_POSN               ;GET THE OFFSET
        ADD     DX,DBFR_START          ;ADD IN THE CURRENT PAGE START ADDR
        RET
BFR_PG_POSN     ENDP



                                        
;******************************************************************************
;
; GRAPHICS MODE BUFFER POSITION
;     THIS ROUTINE CALCULATES THE DISPLAY BUFFER POSTION(OFFSET) OF A CHAR,    
;     GIVEN THE ROW,COL COORDINATE. IT IS ASSUMED THAT GRAPHICS MODE
;     IS IN EFFECT.
;
;     INPUT TO ROUTINE :
;     DH = ROW COORDINATE OF CHARACTER
;     DL = COL COORDINATE OF CHARACTER
;
;     OUTPUT FROM ROUTINE :
;     DX = OFFSET OF THE CHAR IN THE DISPLAY BUFFER
;          
;     OTHER REGS DESTROYED :
;     NONE
;
;******************************************************************************
       
GRAPH_POSN      PROC    NEAR                 
        PUSH    AX                     ;SAVE REGS
        MOV     AL,DH
        XOR     DH,DH                  ;ROWS TO AL
        MUL     BYTE PTR DISP_COLS     ;ROWS  TIMES NO. OF DISP COLUMNS
        ADD     AX,AX                  ;TAKES 4 ROWS/CHAR IN GRAPHICS MODE
        ADD     AX,AX
        ADD     DX,AX                  ;ADD IN COLUMN VALUE
        POP     AX                     ;RESTORE REGS
        RET
GRAPH_POSN      ENDP            


; ---------------------------------

ADJUST_BL	PROC	NEAR
	PUSH	AX
	MOV	BL,AL
	OR	AL,AL
	JZ	BL_RDY
	MOV	AL,DH
	SUB	AL,CH
	INC	AL
	CMP	AL,BL
	JNE	BL_RDY
	XOR	BL,BL
BL_RDY:	POP	AX
	RET
ADJUST_BL	ENDP
                                        
;******************************************************************************
;
; GET CURSOR POSITION
;     THIS ROUTINE GETS THE ROW,COL COORDINATE OF THE CURSOR,
;     GIVEN THE DISPLAY PAGE NUMBER.
;               
;     INPUT TO ROUTINE :
;     BH = DISPLAY PAGE
;
;     OUTPUT FROM ROUTINE :
;     DH,DL = ROW,COL COORDINATE OF THE CURSOR
;          
;     OTHER REGS DESTROYED :
;     BX
;
;******************************************************************************
                                        
GET_CUR_POSN    PROC    NEAR
        MOV     BL,BH                  ;GET DISPLAY PAGE
        SUB     BH,BH
        SAL     BX,1                   ;USE DISPLAY PAGE AS INDEX INTO TABLE
        MOV     DX,CURSOR_POSITION[BX] ;GET ROW,COL FROM CURSOR POSITION TABLE
        RET
GET_CUR_POSN    ENDP



;******************************************************************************
;
; SET CURSOR REGISTER 
;     THIS ROUTINE SETS THE CURSOR POSITION BASED ON THE ROW,COL
;     COORDINATE. THE OFFSET IN THE DISP BFR IS CALCULATED AND 
;     ADJUSTED AND THEN OUTPUT TO THE CRTC(6845) CURSOR ADDRESS REGS R14,R15.
;
;     INPUT TO ROUTINE :
;     DH = ROW COORDINATE OF CURSOR
;     DL = COL COORDINATE OF CURSOR
;
;     OUTPUT FROM ROUTINE :
;     NONE
;          
;     OTHER REGS DESTROYED :
;     AX,BX,DX
;
;******************************************************************************
       
SET_CURSOR_REG  PROC    NEAR
        CALL    BFR_PG_POSN            ;CALCULATE THE OFFSET IN DISP BFR
        SAR     DX,1                   ;ADJUST THE VALUE FOR ATTRIBUTE BYTE
        MOV     AX,DX
        MOV     BL,14                  ;POINT TO R14,R15 CURSOR ADDR REGS
        CALL    CRTC_OUT2              ;OUTPUT OFFSET TO R14,R15 REGS
        RET
SET_CURSOR_REG  ENDP


                                        
;******************************************************************************
;
; SCROLL SETUP      
;     THIS ROUTINE SETS UP VARIABLES FOR THE SCROLL ROUTINES.
;               
;     INPUT TO ROUTINE :
;     CX = ROW,COL COORDINATE OF UPPER LEFT CORNER
;     DX = ROW,COL COORDINATE OF LOWER RIGHT CORNER
;     BL = # OF ROWS TO SCROLL
;     OUTPUT FROM ROUTINE :
;     DX = # OF ROWS,COL IN BLOCK
;     BP = # OF CHARACTERS IN A ROW TIMES 2
;     AX = OFFSET OF SOURCE ADDRESS
;     OTHER REGS DESTROYED :
;      
;
;******************************************************************************
                                      
SCR_SETUP       PROC    NEAR
        SUB     DX,CX                  ;CALCULATE # OF ROWS IN BLK
        ADD     DX,0101H               ;COMPENSATE FOR `O' ORIGIN
        MOV     BP,DISP_COLS           ;GET # OF DISP COLUMNS
        SHL     BP,1
        MOV     AL,BL                  ;GET # OF LINES TO SCROLL
        MUL     BYTE PTR DISP_COLS     ;CALULATE OFFSET TO SOURCE ADDR
        SHL     AX,1                   ;COMPENSATE FOR ATTRIBUTE BYTE
        RET
SCR_SETUP       ENDP



;******************************************************************************
;
; WAIT HORIZONTAL RETRACE
;     THIS ROUTINE WAITS FOR  HORIZONTAL RETRACE (LOW) CONDITION,
;     THEN TURNS OFF INTERRUPTS, AND RETURNS.                       
;               
;     INPUT TO ROUTINE :
;     NONE 
;
;     OUTPUT FROM ROUTINE :
;     NONE
;          
;     OTHER REGS DESTROYED :
;     AX,DX
;
;******************************************************************************

WAIT_HRZ_RETRACE  PROC    NEAR         
        MOV     DX,CRTC_BASE_ADR       ;GET BASE ADDR
        ADD     DX,6                   ;POINT TO STATUS PORT
V110:	STI				;INTERRUPT ON
	NOP
	CLI				;INTERRUPT OFF
	IN      AL,DX                  ;GET STATUS
        AND     AL,1                   ;ISOLATE HORZ RETRACE BIT 0
        JNZ     V110                   ;IS IT LOW?  NO - KEEP CHECKING
;        CLI                            ;YES - HOLD OFF INTERRUPTS
        RET                            ;RETURN
WAIT_HRZ_RETRACE ENDP

;******************************************************************************
;
; WAIT FOR VERTICAL RETRACE ACTIVE OR HORIZONTAL RETRACE LOW
;               
;     INPUT TO ROUTINE :
;     NONE 
;
;     OUTPUT FROM ROUTINE :
;        ZF = 1 IF VERTICAL RETRACE ACTIVE
;	      0 IF HORIZONTAL RETRACE LOW
;          
;     OTHER REGS DESTROYED :
;     AX,DX
;
;******************************************************************************

WAIT_HV_RETRACE  PROC    NEAR         
        MOV     DX,CRTC_BASE_ADR       ;GET BASE ADDR
        ADD     DX,6                   ;POINT TO STATUS PORT
V112:	STI				;INTERRUPT ON
	NOP
	CLI				;INTERRUPT OFF
	IN      AL,DX                  ;GET STATUS
        TEST    AL,8                   ;VERTICAL RETRACE ACTIVE?
        JNZ     V113                   ;YES, RETURN
        AND     AL,1                   ;ISOLATE HORZ RETRACE BIT 0
        JNZ     V112                   ;IS IT LOW?  NO - KEEP CHECKING
;        CLI                            ;YES - HOLD OFF INTERRUPTS
V113:	RET                            ;RETURN
WAIT_HV_RETRACE ENDP

;******************************************************************************
;
; WAIT FOR RETRACE (EITHER VERTICAL OR HORIZONTAL)
;     THIS ROUTINE WAITS FOR  RETRACE (high) CONDITION, AND RETURNS.                       
;               
;     INPUT TO ROUTINE :
;     NONE 
;
;     OUTPUT FROM ROUTINE :
;     NONE
;          
;     OTHER REGS DESTROYED :
;     AX,DX
;
;******************************************************************************

WAIT_FOR_RETRACE  PROC    NEAR         
        MOV     DX,CRTC_BASE_ADR       ;GET BASE ADDR
        ADD     DX,6                   ;POINT TO STATUS PORT
V111:   IN      AL,DX                  ;GET STATUS
        AND     AL,9                   ;ISOLATE HORZ & VER RETRACE BIT
        JZ      V111                   ;IS IT LOW?  YES - KEEP CHECKING
        RET                            ;RETURN
WAIT_FOR_RETRACE ENDP




                                        
;******************************************************************************
;
; VIDEO OFF         
;     THIS ROUTINE TURNS OFF THE VIDEO DURING VERTICAL RETRACE
;               
;     INPUT TO ROUTINE :
;     NONE 
;
;     OUTPUT FROM ROUTINE :
;     NONE
;          
;     OTHER REGS DESTROYED :
;     AX
;
;******************************************************************************
       
VIDEO_OFF       PROC    NEAR
        PUSH    DX                     ;SAVE REG
        MOV     DX,CRTC_BASE_ADR       ;GET BASE ADDR
        ADD     DX,6                   ;POINT TO STATUS
V115:   IN      AL,DX                  ;GET STATUS
        AND     AL,8                   ;WAIT FOR VERTICAL RETRACE
        JZ      V115
        MOV     AL,25H                 ;PREPARE TO WRITE MODE CONTROL REG
        SUB     DX,2                   ;POINT TO MODE CONTROL REG
        OUT     DX,AL                  ;OUTPUT TO MODE CTRL - TURN  OFF VIDEO
        POP     DX                     ;RESTORE REGS
        RET
VIDEO_OFF       ENDP



                                        
;******************************************************************************
;
; MOVE BLOCK        
;     MOVES  A FRAGMENTED BLOCK OF MEMORY  
;
;     INPUT TO ROUTINE :
;     DI = STARTING DESTINATION ADDRESS
;     SI = STARTING SOURCE ADDRESS
;     DL = # OF WORDS TO MOVE EACH ITERATION
;     DH = # OF ITERATIONS
;     BP = ADDRESS INCREMENT/DECREMENT AFTER EACH ITERATION  
;          (POSITIVE IF FORWARD, NEGATIVE IF BACKWARDS)
;     DIR FLAG = 0  MOVE FORWARD
;                1  MOVE BACKWARD
;
;     OUTPUT FROM ROUTINE :
;     NONE
;     NOTE: ON RETURN SI = LAST ROW SOURCE ADDR PLUS BP
;                     DI = LAST ROW DEST ADDR PLUS BP
;          
;     OTHER REGS DESTROYED :
;     CX,DH        
;
;******************************************************************************
                                      
        ASSUME ES:DATAA
MOVE_BLK        PROC    NEAR
        XOR     CH,CH                  ;ZERO TO HIGH BYTE OF COUNT
V120:   MOV     CL,DL                  ;SETUP COUNT
        PUSH    DI                     ;SAVE THE VALUES
        PUSH    SI
        REP     MOVSW                  ;DO THE MOVE
        POP     SI                     ;RECOVER THE VALUES
        POP     DI
        ADD     SI,BP                  ;ADD IN THE ADDRESS INCREMENT/DECREMENT
        ADD     DI,BP                  ;
        DEC     DH                     ;DONE WITH ALL ITERATIONS?
        JNZ     V120                   ;KEEP DOING UNTIL ALL DONE
        RET
MOVE_BLK        ENDP



                                        
;******************************************************************************
;
; CLEAR BLOCK       
;     CLEARS A FRAGMENTED BLOCK OF MEMORY. 
;
;     INPUT TO ROUTINE :
;     AX = CLEAR VALUE
;     DI = STARTING DESTINATION ADDRESS
;     DL = # OF WORDS TO CLEAR WITH CLEAR VALUE EACH ITERATION
;     DH = # OF ITERATIONS
;     BP = ADDRESS INCREMENT/DECREMENT AFTER EACH ITERATION  
;          (POSITIVE IF FORWARD, NEGATIVE IF BACKWARDS)
;     DIR FLAG = 0  CLEAR FORWARD
;                1  CLEAR BACKWARD
;
;     OUTPUT FROM ROUTINE :
;     NONE
;     NOTE: ON RETURN DI = LAST ROW DEST ADDR PLUS BP
;          
;     OTHER REGS DESTROYED :
;     CX,DH        
;
;******************************************************************************
                                      
CLEAR_BLK       PROC    NEAR
        XOR     CH,CH                  ;ZERO TO HIGH BYTE OF COUNT
V122:   MOV     CL,DL                  ;SETUP COUNT
        PUSH    DI                     ;SAVE THE VALUE
        REP     STOSW                  ;DO THE STORE 
        POP     DI                     ;RECOVER THE VALUE 
        ADD     DI,BP                  ;ADD IN THE ADDRESS INCREMENT/DECREMENT
        DEC     DH                     ;DONE WITH ALL ITERATIONS?
        JNZ     V122                   ;KEEP DOING UNTIL ALL DONE
        RET
CLEAR_BLK       ENDP




;******************************************************************************
;
; MOVE BLOCK (BYTES)
;     MOVES  ODD/EVEN FEILDS IN MEMORY      
;
;     INPUT TO ROUTINE :
;     DI = STARTING DESTINATION ADDRESS
;     SI = STARTING SOURCE ADDRESS
;     DL = # OF BYTES TO MOVE EACH ITERATION
;     DH = # OF ITERATIONS
;     BP = ADDRESS INCREMENT/DECREMENT AFTER EACH ITERATION  
;          (POSITIVE IF FORWARD, NEGATIVE IF BACKWARDS)
;     DIR FLAG = 0  MOVE FORWARD
;                1  MOVE BACKWARD
;
;     OUTPUT FROM ROUTINE :
;     NONE
;     NOTE: ON RETURN SI = LAST EVEN SOURCE ADDR PLUS BP
;                     DI = LAST EVEN DEST ADDR PLUS BP
;          
;     OTHER REGS DESTROYED :
;     CX,DH        
;
;******************************************************************************
                                      
MOVE_BLK_BYTE   PROC    NEAR
        PUSH    BP                     ;SAVE INC/DEC VALUE
        XOR     CH,CH                  ;ZERO TO HIGH BYTE OF COUNT
        SUB     BP,2000H               ;BACKUP INC/DEC VALUE IN BP
V125:   MOV     CL,DL                  ;SETUP COUNT
        PUSH    DI                     ;SAVE THE VALUES
        PUSH    SI
        REP     MOVSB                  ;MOVE THE EVEN FIELD
        POP     SI                     ;RECOVER  THE VALUES
        POP     DI                     
        MOV     CL,DL                  ;SETUP COUNT AGAIN
        ADD     SI,2000H               ;POINT TO ODD FIELD
        ADD     DI,2000H
        PUSH    DI                     ;SAVE VALUES
        PUSH    SI
        REP     MOVSB                  ;MOVE THE ODD FIELD
        POP     SI                     ;RECOVER THE VALUES
        POP     DI
        ADD     SI,BP                  ;BACKUP TO NEXT EVEN FIELD             
        ADD     DI,BP                  ;
        DEC     DH                     ;DONE WITH ALL ITERATIONS?
        JNZ     V125                   ;KEEP DOING UNTIL ALL DONE
        POP     BP                     ;RECOVER INC/DEC VALUE
        RET
MOVE_BLK_BYTE   ENDP



                                        
;******************************************************************************
;
; CLEAR BLOCK (BYTES)
;     CLEARS ODD/EVEN FIELDS IN MEMORY.     
;
;     INPUT TO ROUTINE :
;     AL = CLEAR VALUE
;     DI = STARTING DESTINATION ADDRESS
;     DL = # OF BYTES TO CLEAR WITH CLEAR VALUE EACH ITERATION
;     DH = # OF ITERATIONS
;     BP = ADDRESS INCREMENT/DECREMENT AFTER EACH ITERATION  
;          (POSITIVE IF FORWARD, NEGATIVE IF BACKWARDS)
;     DIR FLAG = 0  CLEAR FORWARD
;                1  CLEAR BACKWARD
;
;     OUTPUT FROM ROUTINE :
;     NONE
;     NOTE: ON RETURN DI = LAST EVEN DEST ADDR PLUS BP
;          
;     OTHER REGS DESTROYED :
;     CX,DH        
;
;******************************************************************************
                                      
CLEAR_BLK_BYTE  PROC    NEAR
        XOR     CH,CH                  ;ZERO TO HIGH BYTE OF COUNT
        SUB     BP,2000H               ;BACKUP INC/DEC VALUE IN BP
V127:   MOV     CL,DL                  ;SETUP COUNT
        PUSH    DI                     ;SAVE VALUE 
        REP     STOSB                  ;CLEAR THE EVEN FIELD
        POP     DI                     ;RECOVER VALUE
        ADD     DI,2000H               ;POINT TO ODD FIELD
        PUSH    DI                     ;SAVE VALUE 
        MOV     CL,DL
        REP     STOSB                  ;CLEAR THE ODD FIELD
        POP     DI                     ;RECOVER VALUE
        ADD     DI,BP                  ;BACKUP TO NEXT EVEN FIELD             
        DEC     DH                     ;DONE WITH ALL ITERATIONS?
        JNZ     V127                   ;KEEP DOING UNTIL ALL DONE
        RET
CLEAR_BLK_BYTE  ENDP
       

                                        
;******************************************************************************
;                                      
; PIXEL TO DOT CONVERSION
;     THIS ROUTINE CONVERTS A MEDIUM RESOLUTION 2-BYTE COLOR PIXEL
;     INTO A ONE BYTE DOT(ON/OFF) PATTERN.
;               
;     INPUT TO ROUTINE :
;     AX = 2-BYTE COLOR PIXEL
;
;     OUTPUT FROM ROUTINE :
;     AL = DOT PATTERN
;          
;     OTHER REGS DESTROYED :
;     AH,BX
;
;******************************************************************************
       
PXEL_TO_DOT     PROC    NEAR           
        PUSH    CX                     ;SAVE REG
        MOV     CX,0100H               ;SETUP `OR' PATTERN
        MOV     DX,0003H               ;SETUP 2 BIT MASK
;
V130:   TEST    AX,DX                  ;CHECK FOR BACKGROUND COLOR
        JZ      V131                   ;SKIP THE `OR' IF BACKGROUND
        OR      CL,CH                  ;OR IN THE CORRESPONDING BIT
V131:   SHL     DX,1                   ;SHIFT MASK FOR NEXT 2 BITS
        SHL     DX,1
        ADD     CH,CH                  ;SHIFT CORRESPONDING `OR' BIT
        JNZ     V130                   ;ALL BITS DONE?
        MOV     AL,CL                  ;YES - MOVE DOT PATTERN INTO AL
        POP     CX                     ;RESTORE REG
        RET
PXEL_TO_DOT     ENDP


                                        
;******************************************************************************
;
; WRITE PIXEL       
;     THIS ROUTINE CONVERTS A DOT PATTERN INTO A 2 BYTE COLOR
;     PIXEL AND WRITES THE PIXEL BYTES INTO DISPLAY BUFFER
;               
;     INPUT TO ROUTINE :
;     BL = COLOR CODE
;     DI = OFFSET INTO DISPLAY BUFFER
;     SI = OFFSET TO DOT PATTERN BYTE
;
;     OUTPUT FROM ROUTINE :
;     NONE 
;          
;     OTHER REGS DESTROYED :
;     AX 
;
;******************************************************************************

WRITE_PXEL      PROC    NEAR           
        LODSB                          ;GET DOT PATTERN
        CALL    DOT_TO_PXEL            ;CONVERT TO 2-BYTE COLOR PIXEL
        OR      DL,DL                  ;IS XOR BIT ON?
        JNS     V135                   ;NO - SKIP THE XOR'ING
        XOR     AH,ES:[DI]             ;YES - XOR WITH EXISTING BYTE
        XOR     AL,ES:[DI+1]           
;
V135:   MOV     ES:[DI],AH             ;STORE INTO BUFFER
        MOV     ES:[DI+1],AL
        RET
WRITE_PXEL      ENDP



                                        
;******************************************************************************
;
; DOT TO PIXEL CONVERSION
;     THIS ROUTINE CONVERTS A ONE BYTE DOT PATTERN TO A 2 -BYTE
;     COLOR PIXEL.
;               
;     INPUT TO ROUTINE :
;     AL = DOT PATTERN
;     BL(BITS 0:1) = COLOR CODE
;
;     OUTPUT FROM ROUTINE :
;     AX = 2 BYTE COLOR PIXEL
;          
;     OTHER REGS DESTROYED :
;     NONE
;
;******************************************************************************
       
DOT_TO_PXEL     PROC    NEAR
        PUSH    DX                     ;SAVE REGS
        PUSH    BX
        XOR     BH,BH
        MOV     DL,CS:[BX+OFFSET COLOR_BYTE]   ;GET COLOR PATTERN
                                               ;THERE ARE 4 REPLICATIONS OF
                                               ;COLOR SO FAR
        MOV     DH,DL                  ;DUPLICATE FOR SECOND BYTE
        MOV     AH,AL                  ;GET DOT PATTERN
        SHR     AH,1                   ;MOVE HIGH NIBBLE TO LOW NIBBLE
        SHR     AH,1
        SHR     AH,1
        SHR     AH,1
        AND     AX,0F0FH               ;ISOLATE LOW NIBBLES
        MOV     BL,AL                  ;USE IT TO INDEX TABLE
        MOV     AL,CS:[BX+OFFSET DOUBLE_BYTE] ;TABLE LOOKUP FOR DOUBLING BITS
        MOV     BL,AH                  
        MOV     AH,CS:[BX+OFFSET DOUBLE_BYTE] ;TABLE LOOKUP FOR DOUBLING BITS
        AND     AX,DX                  ;CONVERT TO COLOR
        POP     BX                     ;RESTORE REGS
        POP     DX
        RET
DOT_TO_PXEL     ENDP

;
;       COLOR BYTE TABLE
;

COLOR_BYTE      LABEL    BYTE
                DB       00H,55H,0AAH,0FFH

;
;       DOUBLE BYTE TABLE
;

DOUBLE_BYTE     LABEL    BYTE          
                DB       00H,03H,0CH,0FH,30H,33H,3CH,3FH               
                DB       0C0H,0C3H,0CCH,0CFH,0F0H,0F3H,0FCH,0FFH



                                        
;******************************************************************************
;
; DOT POSITION      
;     THIS ROUTINE DETERMINES THE DISPLAY BUFFER POSITION(OFFSET)
;     OF A DOT, GIVEN ITS ROW,COL COORDINATE IN GRAPHIC TERMS.
;     THE ROUTINE ALSO RETURNS A MASK, SHIFT AMOUNT WHICH CAN
;     BE USED TO EXTRACT/CHANGE BITS WITHIN A BYTE.
;               
;     INPUT TO ROUTINE :
;     DX = ROW COORDINATE (0-199)
;     CX = COL COORDINATE (0 - 639)
;
;     OUTPUT FROM ROUTINE :
;     DI = DISP BFR OFFSET OF THE BYTE
;     AH = MASK TO ISOLATE BITS WITHIN BYTE
;     CL = SHIFT AMOUNT
;          
;     OTHER REGS DESTROYED :
;     BX
;
;******************************************************************************
       
DOT_POSN        PROC    NEAR
        PUSH    DX                     ;SAVE REGS
        PUSH    AX
        PUSH    DX
        SHR     DL,1                   ;GET RID OF ODD/EVEN BIT
        MOV     AL,80
        MUL     DL                     ;MULTIPLY ROW TIMES 80
        MOV     DI,AX
        POP     DX                     ;RECOVER ROW COORDINATE
        POP     AX                     ;RESTORE REG
        TEST    DL,1                   ;IS IT ODD OR EVEN
        JZ      V140                   ;SKIP ADJUSTMENT IF EVEN
        ADD     DI,2000H               ;ADD 2000H FOR ODD BYTE

;
V140:   MOV     DX,CX                  ;GET COL COORDINATE
        SHR     DX,1                   ;DIVIDE COL BY 4
        SHR     DX,1
        CMP     VIDEO_MODE,6           ;HIGH RESOLUTION MODE?
        JE      V141                   ;MED RESOLUTION
        MOV     AH,3                   ;SETUP MASK BYTE
        AND     CL,03H                 ;COMPUTE SHIFT AMOUNT
        XOR     CL,03H
        SHL     CL,1
        JMP     V143
                                       ;HIGH RESOLUTION
V141:   MOV     AH,1                   ;SETUP MASK BYTE
        MOV     BH,07H                 ;SETUP # OF BITS TO STRIP IN COL VALUE
        SHR     DX,1                   ;ADJUST OFFSET
;
V142:   AND     CL,BH                  ;COMPUTE SHIFT AMOUNT
        XOR     CL,BH
V143:   ADD     DI,DX                  ;ADD IN COLUMN VALUE FOR OFFSET
        POP     DX                     ;RESTORE REGS
        RET
DOT_POSN        ENDP


;******************************************************************************
;
;     WRITE STRING OF CHARACTERS
;
;     BH = PAGE, CX =STRING LENGTH, ES = STRING SEGMENT, BP =STRING ADDRESS
;     DX = CURSOR POSITION
;     AL = TYPE OF MOVE
;
;     AL = 0   WRITE STRING OF CHARACTERS. DON'T MOVE CURSOR. BL = ATTRIBUTE
;     AL = 1   WRITE STRING OF CHARACTERS. MOVE CURSOR. BL = ATTRIBUTE
;     AL = 2   WRITE STRING OF CHARACTER, ATTRIBUTE.  DON'T MOVE CURSOR
;     AL = 3   WRITE STRING OF CHARACTER, ATTRIBUTE.  MOVE CURSOR
;
;     NOTE: CR, LF, BS, AND BELL ARE TREATED AS COMMANDS, NOT CHARACTERS
;
;******************************************************************************

STRING PROC NEAR

;SAVE OLD CURSOR POSITION, SET NEW POSITION      

        MOV     DI,DX                  ;SAVE NEW CURSOR POSITION
        CALL    GET_CURSOR_POSITION           
        PUSH    DX                     ;SAVE CURRENT CURSOR POSITION
        MOV     DX,DI                  ;RECOVER OLD CURSOR           
        CALL    SET_CURSOR

;BRANCH TO APPROPRIATE ROUTINE

        CMP     CX,0                   ;0 LENGTH ?
        JE      V300                   ;YES, JUMP
        CMP     AL,0
        JE      V301
        CMP     AL,1
        JE      V303
        CMP     AL,2
        JE      V305
        CMP     AL,3
        JE      V307

;RESTORE CURSOR             

V300:   POP     DX                     ;GET OLD CURSOR
        CALL    SET_CURSOR 
        RET
;
;	AH = 0 
;

V301:   CALL    GET_CHARACTER
        JC      V302                   ;JUMP IF SPECIAL CHARACTER
        CALL    WRITE_CHARACTER
V302:   LOOP    V301
        POP     DX                     ;RECOVER OLD CURSOR
        CALL    SET_CURSOR             ;RESTORE CURSOR POSITION
        RET
;
;	AH =1 
;

V303:   CALL    GET_CHARACTER
        JC      V304                   ;JUMP IF SPECIAL CHARACTER
        CALL    WRITE_CHARACTER
V304:   LOOP    V303
        POP     DX                     ;THROW AWAY OLD CURSOR POSITION
        RET

;
;	AH = 2
;

V305:   CALL    GET_CHARACTER
        MOV     BL,ES:[BP]             ;GET ATTRIBUTE
        JC      V306                   ;JUMP IF SPECIAL CHARACTER
        CALL    WRITE_CHARACTER
        INC     BP                     ;MOVE TO NEXT CHARACTER
V306:   LOOP    V305
        POP     DX                     ;RECOVER OLD CURSOR
        CALL    SET_CURSOR             ;RESTORE CURSOR
        RET

;
;	AH = 3
;

V307:   CALL    GET_CHARACTER
        MOV     BL,ES:[BP]             ;GET ATTRIBUTE
        JC      V308                   ;JUMP IF SPECIAL CHARACTER
        CALL    WRITE_CHARACTER
        INC     BP                     ;MOVE TO NEXT CHARACTER
V308:   LOOP    V307
        POP     DX                     ;THROW AWAY OLD CURSOR POSITION
        RET

;
;SUBROUTINE TO GET CHARACTER AND HANDLE SPECIAL CHARACTERS
;
;GET THE CHARACTER AND CHECK FOR SPECIAL CHARACTER

GET_CHARACTER PROC NEAR

        MOV     AH,VIDEO_MODE          ;CURRENT MODE TO AH
        MOV     AL,ES:[BP]             ;GET THE CHARACTER
        INC     BP                     ;POINT TO NEXT CHARACTER
        CMP     AL,CR                  ;CARRIAGE RETURN ?
        JE      V309                   ;YES, HANDLE SEPARATELY
        CMP     AL,BS                  ;BACKSPACE ?
        JE      V309                   ;YES, HANDLE SEPARATELY
        CMP     AL,LF                  ;LINE FEED ?
        JE      V309                   ;YES, HANDLE SEPARATELY
        CMP     AL,BELL                ;BELL ? 
        JE      V309                   ;YES, HANDLE SEPARATELY
        CLC                            ;CLEAR CARRY
        RET

;HANDLE SPECIAL CHARACTER 

V309:   PUSH    BX
        PUSH    CX
        CALL    HANDLE_SPECIAL         
        POP     CX
        POP     BX
        CALL    GET_CURSOR_POSITION
        STC                            ;SET CARRY
        RET
GET_CHARACTER ENDP

;
;SUBROUTINE TO WRITE CHARACTER
;WRITE CHARACTER
;

WRITE_CHARACTER PROC NEAR

        PUSH    CX
        MOV     CX,1                   ;CHARACTER COUNT
        MOV     AH,9                   
        INT     10H                    ;WRITE CHARACTER AND ATTRIBUTE
        POP     CX 

;MOVE CURSOR

        PUSH    CX
        PUSH    BX
        MOV     AX,OFFSET V310         ;PUT RETURN ADDRESS ON THE STACK
        PUSH    AX
        CALL    UPDATE_CURSOR          ;MOVE CURSOR
V310:   POP     BX
        POP     CX
        RET
WRITE_CHARACTER ENDP


;
;SUBROUTINE TO GET CURSOR POSITION
;
GET_CURSOR_POSITION PROC NEAR
;
        XCHG    BH,BL
        MOV     SI,BX
        XCHG    BH,BL
        SHL     SI,1
        AND     SI,0FEH
        MOV     DX,[SI+OFFSET CURSOR_POSITION]
        RET
GET_CURSOR_POSITION ENDP

;
;SUBROUTINE TO SET CURSOR
;

SET_CURSOR PROC NEAR
        PUSH    AX
        MOV     AH,2
        INT     10H
        POP     AX
        RET
SET_CURSOR ENDP

;
STRING ENDP

eproma	ends
	end

