
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/interrupts.h>
#include <exec/libraries.h>
#include <exec/tasks.h>
#include <exec/io.h>

//************************************************************************
//***
//***  Miscellaneous Definitions
//***
//************************************************************************

#define STACK_SIZE      8192            // task stack size
#define SKIP_BYTES      2048            // size of skip buffer

#define BLOCK_SIZE      2048            // sector size
#define BLOCK_SHIFT     11              // bits used in bytes per sector

#define PERFORMIO       -42             // for calling ourself
#define UNIT_PATTERN    'UNIT'          // used for unit number

#define CDROMPAGESIZE   16*4096         // 64k for 16 buffers

#define PBXSIZE         16              // Number of buffers in PBX

#define ROM_STATUS      0x000           // Status byte (EDC, Sync inserted, etc...)
#define ROM_LSNPOS      0x004           // Header converted to LSN format
#define ROM_HEADER      0x00C           // Actual MSF BCD header
#define ROM_DATA        0x010           // Start of CD-ROM data
#define ROM_C2P0        0xC04           // Start of C2P0 data

#define SECSTSB_C2P0        5           // Set if any C2 bits are set in the sector
#define SECSTSB_EDC1        6           // EDC error detected in mode 2 format
#define SECSTSB_EDC2        7           // EDC error detected in mode 1 format
#define SECSTSB_SHORTSECTOR 8           // Short sector (unexpected sync)
#define SECSTSB_SYNCINSERT  9           // Sync inserted
#define SECSTSB_CORRECTED   14          // ECC has been performed on this sector
#define SECSTSB_INVALID     15          // New sector flag
#define SECSTSF_CNT         0x0000001F  // - Count mask
#define SECSTSF_C2P0        0x00000020
#define SECSTSF_EDC1        0x00000040
#define SECSTSF_EDC2        0x00000080
#define SECSTSF_SHORTSECTOR 0x00000100
#define SECSTSF_SYNCINSERT  0x00000200
#define SECSTSF_CORRECTED   0x00004000
#define SECSTSF_INVALID     0x00008000

#define SECSTS_ERROR        (SECSTSF_C2P0|SECSTSF_EDC1|SECSTSF_EDC2|SECSTSF_SHORTSECTOR)



//************************************************************************
//***
//***  CDTV Device Driver Base Data Segment
//***
//************************************************************************

struct CDDB {
        struct Library      db_Library;
        UWORD               db_Align1;              // ALIGNLONG
        UBYTE              *db_CDROMPage;           // Hardware buffer pointers
        UBYTE              *db_CDCOMRXPage;
        UBYTE              *db_CDCOMTXPage;
        UBYTE              *db_CDSUBPage;

        struct CDInfo       db_Info;                // Info structure (must remain here for CDTV compatibility)
        UWORD               db_Align2;              // ALIGNLONG
        struct Task         db_Task;                // This task               
        struct Interrupt    db_StatIntr;            // Device Interrupt server 
        struct Interrupt    db_XLIntr;              // CDXL interrupt server   

        struct MsgPort      db_ClassACmdPort;       // Port for non-disk access type commands 
        struct MsgPort      db_ClassDCmdPort;       // Port for disk access type commands     
        APTR                db_ClassDReq;           // active class-2 command

        APTR                db_InitTask;            // Initialization task pointer
        UBYTE               db_InitSignal;          // Initialization task signal

        UBYTE               db_ComRXInx;            // Drive communications receive page index
        UBYTE               db_ComTXInx;            // Drive communications transmit page index
        UBYTE               db_SubInx;              // Subcode page index
        UBYTE               db_CDCOMRXCMP;          // Drive communications receive page shadow
        UBYTE               db_ReceivingCmd;        // Receiving a packet from the drive
        UBYTE               db_ComRXPacket;         // Index to packet currently being received
        UBYTE               db_ReadingTOC;          // TOC is being read flag
        UBYTE               db_Disabled;            // Interrupts disabled flag
        UBYTE               db_NoHardware;          // Flag is set if drive is not present
        UBYTE               db_PhotoCD;             // Flag is set if this is a PhotoCD disk

        UBYTE               db_Packet[16];          // Drive transmit packet

        UBYTE               db_Align4;              // ALIGNLONG
        ULONG               db_TOCNext;             // Pointer to next session
        ULONG               db_MultiSession;        // Multi-session disk possibly present?
        UBYTE               db_MSLastTrackTemp;     // Temp variables when doing multi-session
        UBYTE               db_Align5[3];           // ALIGNLONG            
        ULONG               db_MSLeadOutTemp;
        UBYTE               db_Remap;               // Should sectors be remapped?
        UBYTE               db_Align6[3];           // ALIGNLONG           
        ULONG               db_RemapStart;          // Starting sector to remap
        ULONG               db_RemapStop;           // Stopping sector to remap
        UWORD               db_VolDscType;          // Volume descriptor type read in when attemping sector relocation
        struct IOStdReq     db_IOR;                 // Fake IO Request for TOC
                            
        UBYTE               db_PacketIndex;         // Index ORed with command byte
        UBYTE               db_FlickerLight;        // Light flicker flag

        UBYTE               db_IgnoreResponse;      // If non-zero, ignore responses with this command byte
        UBYTE               db_PackSize;            // Copy of CDCOMCMP
        UBYTE               db_PacketAddress;       // Index of last drive command receive packet
        UBYTE               db_PlayStatus;

        UWORD               db_CMD;                 // Primary drive communications packet
        UWORD               db_Align7;              // ALIGNLONG
        ULONG               db_ARG1;
        ULONG               db_ARG2;
        ULONG               db_ARG3;
        ULONG               db_ARG4;
        ULONG               db_ARG5;

        ULONG               db_PlayStart;           // Start and stop position of a play in progress
        ULONG               db_PlayStop;
        ULONG               db_LastQPos;            // Last Q-Code position reported (BCD MSF)
        UWORD               db_LastQState;          // State drive was in when last Q-Code packet was reported

        UWORD               db_Align8;
        struct CDXL         db_XferNode;            // Transfer node for CD_READ command
        UWORD               db_RetryCount;          // Read retry counter
        APTR                db_XferEntry;           // Current transfer entry

        UBYTE               db_BufferCount;         // Expected buffer count from status long
        UBYTE               db_ReadError;
        UBYTE               db_ECC;                 // ECC enabled
        UBYTE               db_Reading;             // Are we trying to read data
        ULONG               db_SeekAdjustment;      // Play sector adjustment
        ULONG               db_SectorIndex;         // Index to data within a sector
        ULONG               db_ReadStart;           // Place where read began
        ULONG               db_ListEnd;             // Dummy end-of-list pointer (NULL)

        UBYTE               db_OpenState;           // Drive door open state
        UBYTE               db_Align9;              // ALIGNLONG
        ULONG               db_ChgCount;            // Disk change counter
        UWORD               db_CurrentSpeed;        // Current frame rate of drive

        UWORD               db_Attenuation;         // Attenuation data             
        UWORD               db_TargetAttenuation;
        UWORD               db_FadeStepSize;

        UBYTE               db_AutoQ;               // Automatic Q-Code command
        UBYTE               db_AutoFrame;           // Automatic Frame interrupt
        UBYTE               db_AutoFade;            // Automatic attenuation fading

        UBYTE               db_Align10;             // ALIGNLONG
        struct List         db_ChgList;             // Interrupt lists
        struct List         db_FrameList;

        union CDTOC         db_TOC[101];            // Table of contents
        UBYTE               db_QCode[16];           // Q-Code packet

        ULONG               db_NULL;
        };



#define SIGB_CMDDONE 20
#define SIGF_CMDDONE 0x00100000
#define SIGB_PBX     24
#define SIGF_PBX     0x01000000


