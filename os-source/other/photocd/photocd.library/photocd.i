        IFND    LIBRARIES_PHOTOCD_I
LIBRARIES_PHOTOCD_I SET 1

**
**  photocd.i   -   photocd.library assembly language header
**
**  Copyright (C) 1993 Commodore-Amiga, Inc.
**  All Rights Reserved
**

;------------------------------------------------------------------------------

    IFND    EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC

*******************************************************************************
**
**  Resolution codes
**
*******************************************************************************

PHOTOCD_RES_BAD     EQU     0   ; Invalid resolution
PHOTOCD_RES_BASE16  EQU     1   ; Base/16  ( 192 x 128)
PHOTOCD_RES_BASE4   EQU     2   ; Base/4   ( 384 x 256)
PHOTOCD_RES_BASE    EQU     3   ; Base     ( 768 x 512)
PHOTOCD_RES_4BASE   EQU     4   ; 4Base    (1536 x 1024)
PHOTOCD_RES_16BASE  EQU     5   ; 16Base   (3072 x 2048)
PHOTOCD_RES_COUNT   EQU     5   ; Number of resolutions

PHOTOCD_BASE_WIDTH  EQU     768 ; Base resolution width
PHOTOCD_BASE_HEIGHT EQU     512 ; Base resolution height

*******************************************************************************
**
**  Image data format codes
**
*******************************************************************************

PHOTOCD_FORMAT_RGB  EQU     0   ; RGB format
PHOTOCD_FORMAT_YUV  EQU     1   ; YUV format

*******************************************************************************
**
**  Transition codes
**
*******************************************************************************

PHOTOCD_TRANS_CUT           EQU     $00            ; Cut
PHOTOCD_TRANS_CROSS         EQU     $01            ; Cross-fade
PHOTOCD_TRANS_RANDOM        EQU     $02            ; Random fade
PHOTOCD_TRANS_BOTTOM2TOP    EQU     $03            ; Bottom-to-top wipe
PHOTOCD_TRANS_TOP2BOTTOM    EQU     $04            ; Top-to-bottom wipe
PHOTOCD_TRANS_LEFT2RIGHT    EQU     $05            ; Left-to-right wipe
PHOTOCD_TRANS_RIGHT2LEFT    EQU     $06            ; Right-to-left wipe

*******************************************************************************
**
**  Medium codes
**
*******************************************************************************

PHOTOCD_MEDIUM_COLOR_NEG    EQU     $00            ; Color negative
PHOTOCD_MEDIUM_COLOR_REV    EQU     $01            ; Color reversal
PHOTOCD_MEDIUM_COLOR_HARD   EQU     $02            ; Color hard-copy
PHOTOCD_MEDIUM_THERMAL_HARD EQU     $03            ; Thermal hard-copy
PHOTOCD_MEDIUM_BW_NEG       EQU     $04            ; Black & white negative
PHOTOCD_MEDIUM_BW_REV       EQU     $05            ; Black & white reversal
PHOTOCD_MEDIUM_BW_HARD      EQU     $06            ; Black & white hard-copy
PHOTOCD_MEDIUM_INTERNEG     EQU     $07            ; Internegative
PHOTOCD_MEDIUM_SYNTH        EQU     $08            ; Synthetic
PHOTOCD_MEDIUM_UNDEFINED    EQU     $FF            ; Undefined

*******************************************************************************
**
**  Codeset codes
**
*******************************************************************************

PHOTOCD_CODESET_ISO646_38CH EQU     $01            ; ISO 646 (38-character)
PHOTOCD_CODESET_ISO646_65CH EQU     $02            ; ISO 646 7-bit (65-character)
PHOTOCD_CODESET_ISO646      EQU     $03            ; ISO 646 (95-character)
PHOTOCD_CODESET_ISO8859_1   EQU     $04            ; ISO 8859-1
PHOTOCD_CODESET_ISO2022     EQU     $05            ; ISO 2022
PHOTOCD_CODESET_NOT_2375    EQU     $06            ; Not registered according
                                                   ; to ISO 2375
PHOTOCD_CODESET_UNDEFINED   EQU     $FF            ; Undefined

********************************************************************************
**
**  SBA (Scene Balance Algorithm) Codes
**
********************************************************************************


PHOTOCD_SBA_NEUTRAL_COLOR   EQU     $00            ; Neutral SBA on; color SBA on
PHOTOCD_SBA_NONE            EQU     $01            ; Neutral SBA off; color SBA off
PHOTOCD_SBA_NEUTRAL         EQU     $02            ; Neutral SBA on; color SBA off
PHOTOCD_SBA_COLOR           EQU     $03            ; Neutral SBA off; color SBA on

*******************************************************************************
**
**  Error codes
**
*******************************************************************************

PHOTOCD_ERR_NONE        EQU     0   ; No error
PHOTOCD_ERR_MEMORY      EQU     1   ; Insufficent free memory
PHOTOCD_ERR_NO_DISC     EQU     2   ; No disc present in CD-ROM drive
PHOTOCD_ERR_DOS         EQU     3   ; dos.library error
PHOTOCD_ERR_DATA_FORMAT EQU     4   ; Invalid Photo CD data
PHOTOCD_ERR_NOT_FOUND   EQU     5   ; Specified image or session not found
PHOTOCD_ERR_ARGS        EQU     6   ; Invalid arguments

*******************************************************************************
**
**  Inputs Tags
**
*******************************************************************************

PCD_ErrorCode       EQU (TAG_USER+1)    ; (ULONG *) Pointer to ULONG where
                                        ; error code (one of PHOTOCD_ERR_*)
                                        ; is deposited if an error is
                                        ; encountered. This variable is
                                        ; unchanged if the operation is
                                        ; successful.

PCD_Source          EQU (TAG_USER+2)    ; (STRPTR) Path to Photo CD
                                        ; filesystem structure.

PCD_Disc            EQU (TAG_USER+2)    ; Operate on disc

PCD_Session         EQU (TAG_USER+3)    ; (UBYTE) Operate on session specified
                                        ; by cardinal session number.

PCD_Image           EQU (TAG_USER+4)    ; (UWORD) Operate on image specified
                                        ; by cardinal image number.

PCD_File            EQU (TAG_USER+5)    ; (BPTR) Operate on file handle.

PCD_Lines           EQU (TAG_USER+6)    ; (ULONG *) Number of lines of
                                        ; image buffer memory to allocate.

PCD_Overview        EQU (TAG_USER+7)    ; (BOOL) Use Overview Pack.

PCD_Resolution      EQU (TAG_USER+8)    ; (UBYTE) Resolution on which to
                                        ;  operate. One of PHOTOCD_RES_*.

PCD_Format          EQU (TAG_USER+9)    ; (UBYTE) Image data format on
                                        ; which to operate. One of
                                        ; PHOTOCD_FORMAT_*.

PCD_StartLine       EQU (TAG_USER+10)   ; (UWORD) Ordinal number of first
                                        ; line of image on which to operate.

PCD_EndLine         EQU (TAG_USER+11)   ; (UWORD) Ordinal number of last
                                        ; line of image on which to operate.

********************************************************************************
**
**  Disc Information Tags
**
********************************************************************************

PCDDisc_Signature   EQU (TAG_USER+1)    ; (STRPTR) Disc signature
PCDDisc_Version     EQU (TAG_USER+2)    ; (UBYTE) Specification version number
PCDDisc_Revision    EQU (TAG_USER+3)    ; (UBYTE) Specification revision number
PCDDisc_SerNo       EQU (TAG_USER+4)    ; (STRPTR) Disc serial number
PCDDisc_CreateStamp EQU (TAG_USER+5)    ; (struct DateStamp *) Disc creation
                                        ;   stamp
PCDDisc_ModifyStamp EQU (TAG_USER+6)    ; struct DateStamp *) Disc modification
                                        ;    stamp
PCDDisc_nImages     EQU (TAG_USER+7)    ; (UWORD) Number of images on disc
PCDDisc_IntrlvADPCM EQU (TAG_USER+8)    ; (UBYTE) Number of ADPCM audio sectors
                                        ;   per interleaved group
PCDDisc_IntrlvImage EQU (TAG_USER+9)    ; (UBYTE) Number of image sectors
                                        ;   per interleaved group
PCDDisc_MinRes      EQU (TAG_USER+10)   ; (UBYTE) Lowest image resoluton
                                        ;   on disc. One of PHOTOCD_RES_*.
PCDDisc_MaxRes      EQU (TAG_USER+11)   ; (UBYTE) Highest image resolution
                                        ;   on disc. One of PHOTOCD_RES_*.
PCDDisc_LeadoutStart EQU (TAG_USER+12)  ; (ULONG) Start of final lead-out
                                        ;   area on disc. This is a
                                        ;   cd.device/struct RMSF packed
                                        ;   as a 32-bit unsigned long word.
PCDDisc_nSessions   EQU (TAG_USER+13)   ; (UBYTE) Number of sessions
                                        ;   on disc.

*******************************************************************************
**
**  Session Information Tags
**
*******************************************************************************

PCDSess_nImages     EQU (TAG_USER+32)    ; (UWORD) Number of images
                                         ;  recorded in this session
PCDSess_CDDAStart   EQU (TAG_USER+33)    ; (ULONG) Start of first CD-DA
                                         ;   track in this session.
                                         ;   This is a cd.device/struct RMSF
                                         ;   packed as a 32-bit unsigned
                                         ;   long word.
PCDSess_LeadoutStart EQU (TAG_USER+34)   ; (ULONG) Start of lead-out area
                                         ;  of this session. This is a
                                         ;  cd.device/struct RMSF packed as
                                         ;  a 32-bit unsigned long word.
PCDSess_WrtrVndr    EQU (TAG_USER+35)    ; (STRPTR) Vendor identification
                                         ;  of writer device used to write
                                         ;  this session.
PCDSess_WrtrProd    EQU (TAG_USER+36)    ; (STRPTR) Product identification
                                         ;  of writer device used to write
                                         ;  this session.
PCDSess_WrtrVer     EQU (TAG_USER+37)    ; (UBYTE) Version number (also
                                         ;  known as major version number)
                                         ;  of writer device used to write
                                         ;  this session.
PCDSess_WrtrRev     EQU (TAG_USER+38)    ; (UBYTE) Revision number (also
                                         ;  known as minor version number)
                                         ;   of writer device used to write
                                         ;   this session.
PCDSess_WrtrDate    EQU (TAG_USER+39)    ; (struct DateStamp *) Revision
                                         ;  date of firmware of writer
                                         ;  device used to write
                                         ;  this session.
PCDSess_WrtrSerNo   EQU (TAG_USER+40)    ; (STRPTR) Serial number of
                                         ;  writer device used to write
                                         ;  this session.
PCDSess_CreateStamp EQU (TAG_USER+41)    ; (struct DateStamp *) Session
                                         ;  creation stamp

********************************************************************************
**
**  Image Information Tags
**
********************************************************************************


PCDImg_4BaseHCT     EQU (TAG_USER+64)    ; (UBYTE) 4Base Huffman Code Table.
                                         ;  In the range 1..4, corresponding
                                         ;  to the four 4Base Huffman Code
                                         ;  Table classes specified by
                                         ;  the Photo CD standard.
PCDImg_IPE          EQU (TAG_USER+65)    ; (BOOL) TRUE if an Image Pack
                                         ;  Extension (IPE) is present.
                                         ;  FALSE if IPE is not present.
PCDImg_ResOrder     EQU (TAG_USER+66)    ; (UBYTE) Resolution order (highest
                                         ;  resolution available for this image). One of
                                         ;  One of PHOTOCD_RES_*.
PCDImg_Rotation     EQU (TAG_USER+67)    ; (WORD) Degrees of counter-clockwise
                                         ;  rotation required to display image
                                         ;  in intended orientation.
PCDImg_4BaseStop    EQU (TAG_USER+68)    ; (UWORD) Sector offset of first
                                         ;  sector after 4Base ICD.
PCDImg_16BaseStop   EQU (TAG_USER+69)    ; (UWORD) Sector offset of first
                                         ;  sector after 16Base ICD.
PCDImg_IPEStop      EQU (TAG_USER+70)    ; (UWORD) Sector offset of first
                                         ;  sector after IPE.
PCDImg_IntrlvADPCM  EQU (TAG_USER+71)    ; (UBYTE) Number of ADPCM audio
                                         ;  sectors per interleaved group
PCDImg_IntrlvImage  EQU (TAG_USER+72)    ; (UBYTE) Number of image sectors
                                         ;  per interleaved group
PCDImg_PrefFast     EQU (TAG_USER+73)    ; (BOOL) TRUE if fast loading
                                         ;  is preferred. FALSE if high
                                         ;  resolution is preferred.
PCDImg_PrefRes      EQU (TAG_USER+74)    ; (UBYTE) Preferred resolution
                                         ;  for single-speed (150KB/s)
                                         ;  reader. One of PHOTOCD_RES_*.
PCDImg_MagX         EQU (TAG_USER+75)    ; (UWORD) X-axis coordinate
                                         ;  of center of magnification area.
PCDImg_MagY         EQU (TAG_USER+76)    ; (UWORD) Y-axis coordinate of
                                         ;  center of magnification area.
PCDImg_MagFactor    EQU (TAG_USER+77)    ; (UWORD) Linear magnification factor.
PCDImg_DispOffX     EQU (TAG_USER+78)    ; (UWORD) X-axis coordinate on
                                         ; display area for center of image.
PCDImg_DispOffY     EQU (TAG_USER+79)    ; (UWORD) Y-axis coordinate on
                                         ;  display area for center of image.
PCDImg_Transition   EQU (TAG_USER+80)    ; (UBYTE) Transition descriptor.
                                         ;  One of PHOTOCD_TRANS_*.
PCDImg_Signature    EQU (TAG_USER+81)    ; (STRPTR) Image Pack Information
                                         ; signature.
PCDImg_SpecVer      EQU (TAG_USER+82)    ; (UBYTE) Version number (also
                                         ; known as major version number)
                                         ; of Photo CD specification to
                                         ; which this image conforms.
PCDImg_SpecRev      EQU (TAG_USER+83)    ; (UBYTE) Revision number (also
                                         ;  known as minor version number)
                                         ;  of Photo CD specification to
                                         ;  which this image conforms.
PCDImg_PIWVer       EQU (TAG_USER+84)    ; (UBYTE) Version number (also
                                         ;  known as major version number)
                                         ;  of imaging workstation software
                                         ;  used to process this image.
PCDImg_PIWRev       EQU (TAG_USER+85)    ; (UBYTE) Revision number (also
                                         ;  known as minor version number)
                                         ;  of imaging workstation software
                                         ;  used to process this image.
PCDImg_16BaseMag    EQU (TAG_USER+86)    ; (UWORD) Magnification factor
                                         ;  applied to 16Base source image.
PCDImg_ScanStamp    EQU (TAG_USER+87)    ; (struct DateStamp *) Image
                                         ;  scanning stamp.
PCDImg_ModifyStamp  EQU (TAG_USER+88)    ; (struct DateStamp *) Last image
                                         ;  modification stamp.
PCDImg_Medium       EQU (TAG_USER+89)    ; (UBYTE) Medium of source image.
                                         ;  One of PHOTOCD_MEDIUM_*.
PCDImg_ProdType     EQU (TAG_USER+90)    ; (STRPTR) Product type of
                                         ;  original image.
PCDImg_ScnrVndr     EQU (TAG_USER+91)    ; (STRPTR) Identity of scanner vendor.
PCDImg_ScnrProd     EQU (TAG_USER+92)    ; (STRPTR) Identity of scanner product.
PCDImg_ScnrVer      EQU (TAG_USER+93)    ; (UBYTE) Version number (also known as
                                         ; major version number) of scanner
                                         ; firmware.
PCDImg_ScnrRev      EQU (TAG_USER+94)    ; (UBYTE) Revision number (also known as
                                         ; minor version number) of scanner
                                         ; firmware.
PCDImg_ScnrDate     EQU (TAG_USER+95)    ; (struct DateStamp *) Scanner firmware
                                         ; revision date.
PCDImg_ScnrSerNo    EQU (TAG_USER+96)    ; (STRPTR) Serial number of scanner.
PCDImg_ScnrPixel    EQU (TAG_USER+97)    ; (UWORD) Pixel size of scanner.
                                         ; Fixed-point representation with two
                                         ; fractional digits.
PCDImg_PIWMfgr      EQU (TAG_USER+98)    ; (STRPTR) Imaging workstation
                                         ; manufacturer.
PCDImg_PhtfinCharSet EQU (TAG_USER+99)   ; (UBYTE) Photofinisher's name character
                                         ; set. One of PHOTOCD_CHARSET_*.
PCDImg_PhtfinEscape EQU (TAG_USER+100)   ; (STRPTR) Photofinisher's name escape
                                         ; sequences (for ISO 2022 character set).
PCDImg_PhtfinName   EQU (TAG_USER+101)   ; (STRPTR) Photofinisher's name.
PCDImg_SBAVer       EQU (TAG_USER+102)   ; (UBYTE) Version number (also known as
                                         ; major version number) of
                                         ; scene balance algorithm.
PCDImg_SBARev       EQU (TAG_USER+103)   ; (UBYTE) Revision number (also known as
                                         ; minor version number) of
                                         ; scene balance algorithm.
PCDImg_SBACommand   EQU (TAG_USER+104)   ; (UBYTE) Scene balance algorithm
                                         ; command. One of PHOTOCD_SBA_*.
PCDImg_SBAData      EQU (TAG_USER+105)   ; (UBYTE *) Scene balance algorithm
                                         ;  data.
PHOTOCD_SBA_DATASIZE EQU 100             ; Size of PCDImg_SBAData data
PCDImg_Copyright    EQU (TAG_USER+106)   ; (STRPTR) Filename of text file
                                         ; containing copyright/use rights
                                         ; text.

*******************************************************************************
**
**  Image Constants
**
*******************************************************************************

PHOTOCD_BYTES_PER_PIXEL EQU 3           ; Bytes per pixel for RGB/YUV data

    ENDC
