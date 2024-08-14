        NOLIST
**************************************************************************
** Copyright 1991 CONSULTRON
*
*       mfm.i  -- device code include file 
*
**************************************************************************

        include "exec/types.i"
        include "exec/nodes.i"
        include "exec/lists.i"
        include "exec/libraries.i"
        include "exec/devices.i"
        include "exec/io.i"
        include "exec/alerts.i"
        include "exec/initializers.i"
        include "exec/memory.i"
        include "exec/resident.i"
        include "exec/ables.i"
        include "exec/errors.i"
        include "dos/dos.i"
        include "dos/dosextens.i"
        include "devices/trackdisk.i"

        include "asmsupp.i"

CBM     EQU    1
;DEBUG   EQU    1
;--------------------------------------------------------------
;       device command defintions
;
; Use "trackdisk.i" commands for most of the mfmdisk commands.
;--------------------------------------------------------------
;--------------------------------------------
;       device related data structures
;--------------------------------------------
; maximum number of units in this device
md_NUMUNITS     EQU      4

;------- struct 'C' Equivalent:
;       struct header = {
;       UBYTE   h_track;
;       UBYTE   h_head;
;       UBYTE   h_sec;
;       UBYTE   h_sec_len;
;       UWORD   h_crc16;
;       UWORD   h_datacrc16;
;       APTR    h_data;
;       UBYTE   h_amdm_gap;
;       UBYTE   h_sec_err;
;       UWORD   h_sec_gap;
;       };

        STRUCTURE       header,0
                UBYTE   h_track        ; track num
                UBYTE   h_head         ; head num
                UBYTE   h_sec          ; sector num
                UBYTE   h_sec_len      ; sector length; 0=128; 1=256; 2=512; etc.
            LABEL   head_size      ; size of sector head
                UWORD   h_crc16        ; header crc16 value
                UWORD   h_datacrc16    ; data crc16 value
                APTR    h_data         ; sector data ptr
            LABEL   h_sec_corr      ; = UWORD; alternate for h_amdm_gap for write_track()
                UBYTE   h_amdm_gap     ; sector addr mark to data mark gap (in MFM bytes)
                UBYTE   h_sec_err      ; sector err
                UWORD   h_sec_gap      ; gap between sectors (in MFM bytes)
            LABEL   header_sz

;------- struct 'C' Equivalent:
;       struct TrkStruct = {
;       ULONG   trk_in_buf;
;       UBYTE   slct_trk;
;       UBYTE   slct_head;
;       UBYTE   slct_sec;
;       UBYTE   slct_sec_len;
;       APTR    trk_buf;
;       APTR    trk_start;
;       ULONG   trk_read_size;
;       ULONG   trk_writesz_max;
;       ULONG   trk_writesz;
;       struct  header *head[0];
;       UWORD   num_sec_trk;
;       UWORD   num_trks;
;       UWORD   sec_len;
;       UBYTE   sec_len_shift;
;       UBYTE   sec_off;
;       ULONG   sec_gap_min;
;       UWORD   sec_gap_opti;
;       UWORD   ts_res0;
;       };

        STRUCTURE       TrkStruct,0
                ULONG   trk_in_buf      ; current track in buffer
                UBYTE   slct_trk        ; selected track in buffer
                UBYTE   slct_head       ; selected head
                UBYTE   slct_sec        ; selected sector
                UBYTE   slct_sec_len    ; selected sector len >> 7
                APTR    trk_buf         ; pointer to raw track buffer start
                APTR    trk_start       ; pointer to start raw track buffer if fixed
                ULONG   trk_read_size   ; raw track read buffer size
                ULONG   trk_writesz_max ; raw track write buffer size maximum
                APTR    trk_writesz     ; raw track write buffer size if fixed
                APTR    head            ; pointer to sector locations in
                                        ; current track buffer
                UWORD   num_sec_trk     ; num of sectors in current track
                UWORD   num_trks        ; num of tracks in current drive
                UWORD   sec_len         ; sector length in bytes
                UBYTE   sec_len_shift   ; sector length shift value
                UBYTE   sec_off         ; sector # offset
                ULONG   sec_gap_min     ; minimum sector gap to speed up sector searches
                UWORD   sec_gap_opti    ; optimum sector gap based on num_sec_trk and track size
                UWORD   ts_res0         ; reserved
            LABEL   TrkStruct_sz

DEFSECNUM:      set     9               ; default num of sectors per track
DEFSECNUM_HD:   set     9*2             ; default num of sectors per track; HD drive
DEFHICYLNUM:    set     (80-1)          ; default Hi Cyl num of 3.5 drive; incl HD drive
DEFHICYLNUM_5_25: set   (40-1)          ; default Hi Cyl num of 5.25 drive
DEFINDEX:       set     1               ; default num of indices per track
DEFTRKNUM:      set     160             ; default num of tracks per disk
DEFB_SEC_LEN    set     9               ; default byte size bit #
DEF_SEC_LEN     set     1<<DEFB_SEC_LEN ; default byte size of sector
DEF_SEC_OFF     set     1               ; default sector # offset
DEF_DOSENV_TBL_SZ   set     16          ; DOS environment vector table size
DEF_DOSENV_SZ   set     ((DEF_DOSENV_TBL_SZ+1)*ULONG_SIZE)

;------- struct 'C' Equivalent:
;       struct MFMDev = {
;       struct  Device;
;       APTR    md_SegList;
;       UBYTE   md_Flags;
;       UBYTE   md_pad;
;       APTR    md_Units; *md_NUMUNITS
;       };

        STRUCTURE       MFMDev,DD_SIZE  ; place device structure first
                APTR    md_SegList      ; place lib seglist into here
				APTR    md_AbsExecBase  ; addr of AbsExecBase
                UBYTE   md_Flags        ; place lib flags into here
                UBYTE   md_pad          ; do nothing with this; data alignmentt
                STRUCT  md_Units,(md_NUMUNITS*4) ; place ptr to unit task
            LABEL   MFMDev_sz


;------- struct 'C' Equivalent:
;   struct DCIReq             /* Diskchange Interrupt IORequest struct */
;       {
;       struct IOStdReq     DCIR;
;       struct Interrupt    DCII;
;       };

        STRUCTURE       DCIReq,0           ; Disk Change Interrupt
                STRUCT  DCIR,IOSTD_SIZE    ; IO STD Request structure
                STRUCT  DCII,IS_SIZE       ; Interrupt Structure
            LABEL   DCIReq_sz


;------- struct 'C' Equivalent:
;       struct MFMDevUnit = {
;       struct  Unit;
;       UBYTE   mdu_RetryCnt;
;       UBYTE   mdu_CurrRetryCnt;
;       struct  Device *mdu_Device;
;       UBYTE   mdu_UnitNum;
;       UBYTE   mdu_DFLAGS;
;       UBYTE   mdu_error;
;       UBYTE   mdu_res0;
;       struct  MemEntry *mdu_MemEntL;
;       struct  IOExtIO mdu_TDREQ;
;       struct  MsgPort mdu_port_tr;
;       UWORD   mdu_res1;
;       struct  TrkStrt *mdu_TrkStrt;
;       ULONG   mdu_trk_buf_sz;
;       struct  DCIReq mdu_dci;
;       };

        STRUCTURE       MFMDevUnit,UNIT_SIZE  ; place unit structure first
                UBYTE   mdu_RetryCnt    ; retry cnt
                UBYTE   mdu_CurrRetryCnt ; current retry cnt
                APTR    mdu_Device      ; place unit's device struct ptr
                UBYTE   mdu_UnitNum     ; place unit #
                UBYTE   mdu_DFLAGS      ; Unit Disk Flags
                UBYTE   mdu_error       ; error status since last disk change
                UBYTE   mdu_res0        ; reserved
                APTR    mdu_MemEntL     ; place unit's MemEntry List ptr
                STRUCT  mdu_TDREQ,IOTD_SIZE ; TDRequest structure
                STRUCT  mdu_port_tr,MP_SIZE ; TDRequest reply port 
                UWORD   mdu_res1        ; reserved
                APTR    mdu_TrkStrt     ; pointer to raw track buffer struct
                ULONG   mdu_trk_buf_sz  ; size of track buffer needed
                STRUCT  mdu_dci,DCIReq_sz ; diskchange interrupt req struct
            LABEL   MFMDevUnit_sz

;------- state bit for unit flags
;        BITDEF  0 & 1 are already used. Do not use
        BITDEF  MDU,STOPPED,2           ; Unit STOPPED
        BITDEF  MDU,DISK_CHANGED,3      ; Disk Changed since last state update
                    ; This bit activated only by disk change interrupt routine
                    ; If this bit is set, the following TWO bits are invalid 
                    ; until updated.
        BITDEF  MDU,WRITE_PROT,4        ; Disk Write protected
        BITDEF  MDU,NO_DISK,5           ; No Disk in drive
        BITDEF  MDU,NO_TRK_MEM,6        ; No Track Buffer memory allocated
        BITDEF  MDU,REMTASK,7           ; remove Unit Task

;------- state bit for unit disk flags
        BITDEF  MDU,TRK_SEEK,0          ; track seek only.  DO NO READS
        BITDEF  MDU,TRK_MOD,1           ; track modified
        BITDEF  MDU,TRK_SYNCED,2        ; track synced to index pulse
        BITDEF  MDU,TRK_FORMAT,3        ; track to be formatted
        BITDEF  MDU,TRK_WRONG,4         ; track wrong in Address mark
        BITDEF  MDU,TRK_FIXED,5         ; track fixed; addr and data marks and gaps corrected
        BITDEF  MDU,DOUBLE_STEP,6       ; tracks to be double-stepped
        BITDEF  MDU,SINGLE_SIDED,7      ; disk is single-sided flag


;------- struct 'C' Equivalent:
;       struct UnitMemEntry = {
;       struct  List unit_memlist;
;       UWORD   mel_numents;
;       struct  Unit   *mel_Unit;
;       ULONG   mels_Unit;
;       struct  TrkStrt   *mel_TrkStrt;
;       ULONG   mels_TrkStrt;
;       struct  Task   *mel_task;
;       ULONG   mels_task;
;       UBYTE   *mel_taskstack;
;       ULONG   mels_taskstack;
;       UBYTE   *mel_def_DosEnvec;
;       ULONG   mels_def_DosEnvec;
;       struct  header   *mel_head;
;       ULONG   mels_head;
;       UBYTE   *mel_trkbuf_mem;
;       ULONG   mels_trkbuf_mem;
;       };

        STRUCTURE       UnitMemEntry,LN_SIZE    ; place List structure first
                UWORD   mel_numents             ; # of entries in MemList
                APTR    mel_Unit                ; Unit struct ptr
                ULONG   mels_Unit               ; Unit struct size
                APTR    mel_TrkStrt             ; TrkStrt struct ptr
                ULONG   mels_TrkStrt            ; TrkStrt struct size
                APTR    mel_task                ; task struct ptr
                ULONG   mels_task               ; task struct size
                APTR    mel_taskstack           ; task stack ptr
                ULONG   mels_taskstack          ; task stack size
                APTR    mel_def_DosEnvec        ; default DosEnvec struct ptr
                ULONG   mels_def_DosEnvec       ; default DosEnvec struct size
                APTR    mel_head                ; head struct ptr
                ULONG   mels_head               ; head struct size
                APTR    mel_trkbuf_mem          ; raw track buffer mem ptr
                ULONG   mels_trkbuf_mem         ; raw track buffer mem size
            LABEL   UnitMemEntry_sz

;------- state bit for IORequest flags
        BITDEF  IO,IMMED,7
        BITDEF  IO,ABORT,6
;
; stack size and priority for the IO tasks and ports we will create
UNITTASKSTACKSIZE set      $200
DEVPRI            set      0
UNITTASKPRI       set      0
UNITPORTPRI       set      0

MFMNAME macro
                dc.b    'mfm.device',0
                endm

;------ New MFM System 34 format constants
MFM_SYNC_A1A1   set     $44894489       ; MFM pattern for Address/Data ID mark LW1

MFM_SYNC_A1FE   set     $44895554       ; MFM pattern for Address ID mark LW2
CRC_A1A1A1FE    set     $B230           ; CRC preset for pattern

MFM_SYNC_A1FB   set     $44895545       ; MFM pattern for Data ID mark LW2
CRC_A1A1A1FB    set     $E295           ; CRC preset for pattern

CRC_DATA_ZEROS  set     $51449454       ; CRC ($DA6E) MFM pattern for data = zeros

MFM_SYNC_C2C2   set     $52245224       ; Index sync mark LW1
MFM_SYNC_C2FC   set     $52245552       ; Index sync mark LW2

MFM_4E4E        set     $92549254       ; MFM NULL LW
MFM_0000        set     $AAAAAAAA       ; MFM ZERO LW

MFM_DB6D        set     $51454551       ; MFM WORST-CASE PATTERN?


MFM_BSZ         set     2     ; MFM byte size = 2 data bytes  

**********************************************************************
* SZ_DEF MACRO -- Creates three defines:
*   NAME_SZ  = Size in bytes
*   NAME_LSZ = Size in MFM longwords (bytes / 2)
*   NAME_MSZ = Size in MFM bytes (bytes * 2)
**********************************************************************/
SZ_DEF  macro
\1_SZ       set     (\2)        ; actual data size
\1_LSZ      set     ((\1_SZ)>>1)   ; MFM longword data size
\1_MSZ      set     ((\1_SZ)<<1)   ; MFM byte data size (add one clock bit for each data bit)
        endm

    SZ_DEF  IM_SYNC,4  ; data byte size of Index Mark
    SZ_DEF  AM_SYNC,4  ; data byte size of Address Mark
    SZ_DEF  DM_SYNC,4  ; data byte size of Data Mark
    SZ_DEF  ADDR,(6+AM_SYNC_SZ)  ; data byte size of sector address
    SZ_DEF  CRC,2                ; CRC byte size
    SZ_DEF  DATA_CRC,(2+DM_SYNC_SZ)  ; data byte size of sector data CRC

;------ MFM System 34 format gap constants
    SZ_DEF  IP_IM_NULL,90   ; Index Pulse to Index Mark NULL data bytes
    SZ_DEF  IP_IM_ZERO,12   ; Index Pulse to Index Mark ZERO data bytes
    SZ_DEF  IM_AM_NULL,50   ; Index Mark to Address Mark NULL data bytes
    SZ_DEF  IM_AM_ZERO,12   ; Index Mark to Address Mark ZERO data bytes
    SZ_DEF  AM_DM_NULL,22   ; Address Mark to Data Mark NULL data bytes
    SZ_DEF  AM_DM_ZERO,12   ; Address Mark to Data Mark ZERO data bytes
    SZ_DEF  DM_AM_NULL,80   ; Data Mark to Address Mark NULL data bytes
    SZ_DEF  DM_AM_ZERO,12   ; Data Mark to Address Mark ZERO data bytes

    SZ_DEF  I_GAP,(IP_IM_NULL_SZ+IP_IM_ZERO_SZ+IM_SYNC_SZ+IM_AM_NULL_SZ)
                                ; data byte Size of Index Mark + gaps

    SZ_DEF  I_AM_GAP,(I_GAP_SZ+IM_AM_ZERO_SZ+AM_SYNC_SZ)
                        ; data byte Size of Index Pulse including 1st Address Mark

    SZ_DEF  DM_AM_GAP,(DM_AM_NULL_SZ+DM_AM_ZERO_SZ+AM_SYNC_SZ)
                        ; data byte Size of gap between last sector data CRC and Address Mark

    SZ_DEF  SECTOR_GAP,((DM_AM_ZERO_SZ+ADDR_SZ+AM_DM_NULL_SZ)+(AM_DM_ZERO_SZ+DATA_CRC_SZ+DM_AM_NULL_SZ))
                        ; data size between data portions of sectors; gap optimal
    SZ_DEF  DM_AM_NULL_MIN,50   ; Data Mark to Address Mark NULL data bytes (Minimum)
    SZ_DEF  SECTOR_GAP_MIN,((DM_AM_ZERO_SZ+ADDR_SZ+AM_DM_NULL_SZ)+(AM_DM_ZERO_SZ+DATA_CRC_SZ+DM_AM_NULL_MIN_SZ))
                        ; minimum data size between data portions of sectors
    SZ_DEF  SEC_GAP_DELTA,15    ; threshold delta = gap optimal - gap minimum

    SZ_DEF  DATA_DFT,584    ; Default data size including syncs and gaps
    SZ_DEF  ADDR_DFT,44     ; Default address size including syncs and gaps
    SZ_DEF  SEC_DFT,(ADDR_DFT_SZ+DATA_DFT_SZ)   ; Default sector size


    SZ_DEF  TRACKRAW,((250000/8)/5)
        ; ((Bits per second [MFM] / bits per byte) / revs per sec )
        ;   * MFM factor [1 MFM bit + 1 data bit]
                                ; Maximum allowable buffer (32K) 
    SZ_DEF  SPEEDVAR_MIN,190    ; ~ -3% minimum speed variance allowed 
    SZ_DEF  SPEEDVAR_MAX,300    ; ~ +5% maximum speed variance allowed 
    SZ_DEF  TRACKMAX,(TRACKRAW_SZ+SPEEDVAR_MAX_SZ)
    SZ_DEF  TRACKMIN,TRACKRAW_SZ
    SZ_DEF  TRACKBUFF,(TRACKMAX_SZ+I_AM_GAP_SZ)

;*** High Density drive track sizes
    SZ_DEF  TRACKRAW_HD,(((250000/8)/5)*2)    ; HD disk track size
    SZ_DEF  TRACKMAX_HD,(TRACKRAW_HD_SZ+SPEEDVAR_MAX_SZ)
    SZ_DEF  TRACKMIN_HD,(TRACKRAW_HD_SZ)
    SZ_DEF  TRACKBUFF_HD,(TRACKMAX_HD_SZ+I_AM_GAP_SZ)

TRK00           set     0

;------ No track in buffer
NOTRACK         set    -1
;------ No sector -- Index
NOSECTOR        set    -1

;------ New MFM disk error codes
MDERR_OutofTracks       equ     36      ;Out of physical tracks
MDERR_InvParam          equ     37      ;Invalid parameter (MD_SETPARMS)
MDERR_IndexNotSync      equ     38      ;Index mark not synced to index pulse
MDERR_WrongTrack        equ     39      ;Drive head on wrong track from where it
                                        ; should be.

;------ New MFM commands
MD_SETPARMS     equ     29     ;set physical disk params
MD_LASTCOMM     equ     (MD_SETPARMS+1)  ; last mfm device command

;------ New device flags for using during OpenDevice() of mfm.device
        BITDEF  MD,SETPARMS,2   ;set physical disk params on open

RETRYCNT_NORM    set 3   ; retry limit for normal drives
RETRYCNT_5_25    set 20   ; retry limit for 5 1/4" drives


    ifd DEBUG
        XREF    KPrintF
**** MACRO DEF
KPRINTF     macro           ; KPrintF() macro   1=format string 2,3,4,5= values
        movem.l d0/d1/a0/a1,-(sp)
        lea.l   \1,A0           ; get ptr to format string
        ifgt    NARG-1
            lea.l   KPF_array,A1    ; get ptr to values array
            move.l  \2,0(A1)        ; pass value into array
        endc
        ifgt    NARG-2
            move.l  \3,4(A1)        ; pass value into array
        endc
        ifgt    NARG-3
            move.l  \4,8(A1)        ; pass value into array
        endc
        ifgt    NARG-4
            move.l  \5,12(A1)       ; pass value into array
        endc

        jsr     KPrintF
        movem.l (sp)+,d0/d1/a0/a1
    endm
;        ifnd KPF_array
            section ,DATA
KPF_array   ds.l    4
;        endc

    elseif
KPRINTF     macro           ; KPrintF() macro   Blank MACRO; do nothing
    endm
    endc
;        LIST
