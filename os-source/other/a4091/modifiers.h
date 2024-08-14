//=============================================================================
// Very first revision of the DMA chip has XTPORT1 and XTPORT2 reversed.
//=============================================================================
//#define REAL_OLD	1

//=============================================================================
// Early revs of the A590 DMA chip would drop a couple of words at the end of
// a read operation.  With FIXING_READS defined the last block of a transfer is
// extended to read the next block too into an internal buffer.  The required
// block is then copied with the CPU to the final destination.  This is pretty
// klugey, but not required on final DMA chips.
//=============================================================================
//#define FIXING_READS	1

//=============================================================================
// The latest rev of the A590 DMA chip does not handle flushing properly.  This
// means that disconnection and reselection cannot be handled because the DMA's
// FIFO cannot be flushed in the middle of a read or write.  Uncomment when the
// DMA chip gets fixed (hopefully no 2091 boards will go out with this chip in)
//
// Augi tells me that the B chip requires terminal count, and was only used in
// A590's (but it might have gotten into a few 2091's).  The product code for
// the B chip was 2, and the product code for the D and later chips is 3.  The
// D and later DO NOT work correctly with terminal count!  Because of this, the
// 590/2091 driver will check the DMAC product code and act accordingly. - REJ
//=============================================================================
//#define DMA_FLUSH_OK	1
#define DMAC_FLUSH_REV	3

//=============================================================================
// Leave this uncommented to build an A590/A2091 SCSI/XT driver
// (Really all but A2090!)
//=============================================================================
#ifndef	IS_A2090
#define A590	1
#endif	//	IS_A2090

// for A590/A2091 ONLY
#ifdef	IS_A2091
#define A590_A2091_ONLY		1
#endif
#ifdef	IS_A590
#define A590_A2091_ONLY		1
#endif

//=============================================================================
// Leave this uncommented to build an A2090 SCSI/ST506 driver (NOT DONE YET!!!)
//=============================================================================
#ifdef	IS_A2090
#define A2090		1
#endif

//=============================================================================
// Is this a kickstart ROM module?  Means that no expansion scsi.device can
// be inited before me, for example.
//=============================================================================
#ifdef	IS_A300
#define IN_KICK		1
#endif
#ifdef	IS_A1000
#define IN_KICK		1
#endif
#ifdef	IS_CDTVCR
#define IN_KICK		1
#endif
#ifdef	IS_SCSIDISK
#define IN_KICK		1
#endif
#ifdef	IS_A4000T
#define IN_KICK		1
#endif
// for making the kludge disk-loadable driver
#ifdef	IS_A3090_DISK
#define IN_KICK		1
#endif

//=============================================================================
// Is this a Zorro-3 board?  (note a3090_disk is also IS_A3090)
//=============================================================================
#ifdef IS_A3090
#define ZORRO_3		1
#endif
// FIX!!!! REMOVE!!! temp for first A4000
//#ifdef IS_A4000T
//#define ZORRO_3		1
//#endif

//=============================================================================
// Do we get our parameters from battmem or jumpers?
//=============================================================================
#ifdef IS_A4000T
#define USE_BATTMEM	1
#endif
#ifdef IS_SCSIDISK
#define USE_BATTMEM	1
#endif
#ifdef IS_A1000
#define USE_BATTMEM	1
#endif

//=============================================================================
// Does this driver support CAM?
//=============================================================================
#ifdef NCR53C710
#define USES_CAM	1
#endif

//=============================================================================
// Offset from base of board to the start of the code in the autoboot roms.  If
// AUTOBOOT_ROM is not defined, then an expansion drawer device will be built.
// Scsidisk and ide need the autoboot code, but don't need to find the board
// (controlled by an#ifdef EXPANSION_DEV in the source)
//=============================================================================

#ifndef IN_KICK
#ifdef IS_A3090
#define NOT_EXECUTE_IN_PLACE 1
#else
#define EXECUTE_IN_PLACE 1
#endif
#endif

#ifndef	IS_DISK
#ifdef IS_A3090
#define AUTOBOOT_ROM	0x0000	// rom starts at beginning of PIC
#else
#define AUTOBOOT_ROM	0x2000
#endif
#endif

#ifndef	IN_KICK
#define EXPANSION_DEV	1
#endif

#ifdef IS_A3090
#define A3090_ROM_OFFSET  0x00000000
#define A3090_DRIVER_OFFSET 0x00002000	// allow 8K/4 bytes for loader
#define A3090_CHIP_OFFSET 0x00800000
#define A3090_VECTOR	  0x00880003
#define A3090_JUMPERS	  0x008C0003
#define NCR_WRITE_OFFSET  0x00040000	// offset for longword writes...
#endif

//=============================================================================
// Does this have to work under 1.3?
//=============================================================================
#ifdef	IS_A590
#define KS_1_3	1
#endif
#ifdef	IS_A2091
#define KS_1_3	1
#endif

//=============================================================================
// Is this a V39 kickstart module?
//=============================================================================
#ifdef	IS_A300
#define V39		1
#endif
#ifdef	IS_A1000
#define V39		1
#endif
#ifdef	IS_SCSIDISK
#define V39		1
#endif
#ifdef	IS_A4000T
#define V39		1
#endif

//=============================================================================
// romboot library (AddDosNode) used to screw up and not make proper bootnodes.
// this is fixed under 1.4 so this skips code in mountstuff.asm
//=============================================================================
#ifndef	KS_1_3
#define ROMBOOT_FIXED	1
#endif

//=============================================================================
// A590/A2091/A2090 con only DMA to 24-bit space, must use programmed IO above.
//=============================================================================
#ifdef	IS_A590
#define DMA_24_BIT	1
#endif
#ifdef	IS_A2091
#define DMA_24_BIT	1
#endif
#ifdef	IS_A2090
#define DMA_24_BIT	1
#endif

//=============================================================================
// Do we want the Get Geometry code enabled...
//=============================================================================
#define	GET_GEOMETRY	1

//=============================================================================
// Max sector size supported by the Z3->Z2 copying code (buffer size)
//=============================================================================
#define	Z2_BUFFERSIZE	2048

//=============================================================================
// NCR 53c710 address in A4000T
//=============================================================================
#ifdef IS_A4000T
#define NCR_ADDRESS	 0x00dd0040
#define NCR_WRITE_OFFSET 0x00000080	// offset for long writes - 128 bytes
#define NCR_DIP_ADDR	 // ?????
#define A3090_CHIP_OFFSET 0
#endif

//=============================================================================
// Various IDE things...
//=============================================================================

//=============================================================================
// The ide driver might as well run as one task, since it doesn't reselect
// anyways.  This can also be used for SCSI if we want.
//=============================================================================
#ifdef IS_IDE
#define ONE_TASK	1
#endif

//=============================================================================
// WD Caviar 2120 (120Meg) AT IDE drives return the interrupt before the data is
// ready.  The Caviar 140 (40Meg) is ok.  Actually, about 90% of all AT drives
// have this problem to one degree or another.
//=============================================================================
#define WD_AT_KLUDGE	1

//=============================================================================
// The Conner CP2024 2.5" AT drive (20MB) comes up in translate mode (615/4/17)
// and refuses to listen to InitializeDriveParameters commands.  It says it's
// 653/2/32.  This violates the CAM-ATA spec.  We need to kludge for this drive
// by explicitly recognizing it.
//=============================================================================
#define CP2024_KLUDGE	1

//=============================================================================
// Define this if you want removable ide media to be identified as tape drives.
// Leave it undefined if removable ide media is to be identified as a disk.
//=============================================================================
#define IDE_TAPES_SUPPORTED	1

//=============================================================================
// Randy Hilton says we must do an enable interrupt, though it seems most
// drives come up with them enabled (except the PrairieTek 342).
// The CDTVCR can't access that register
//=============================================================================
#ifndef IS_CDTVCR
#define USE_ENABLE_INTS	1
#endif

//=============================================================================
// Randy Hilton says we must do an Initialize Drive Parameters for the same
// parameters it has told us in it's identify drive response.  According to
// the CAM-ATA spec, this should NOT be needed, but Randy insists.  This
// breaks the WD 2120 (120MB) drive (it refuses to allow read-multiple after
// this command, even if a Set Multiple is done).
//=============================================================================
#define USE_INIT_PARAM	1

//=============================================================================
// The A2091 does not support xt devices anymore.  Uncomment for an A590 driver.
//=============================================================================
#ifdef	IS_A590
#define XT_SUPPORTED	1
#define XT_OR_AT_IDE	1
#endif

#ifdef	IS_IDE
#define AT_SUPPORTED	1
#define XT_OR_AT_IDE	1
#ifndef	A1000
#define FOR_68000_ONLY	1
#endif
#endif

#ifndef	IS_IDE
#define SCSI_SUPPORTED	1
#endif

//=============================================================================
// SCSI hardware selection - 590 may have A part nowadays.  Should we
// adjust dynamically??  FIX!
//=============================================================================
#ifdef IS_SCSIDISK
#define CLOCK_14MHZ	1
#define WD33C93A	1
#endif
#ifdef IS_A590
#define CLOCK_7MHZ	1
#define WD33C93		1
#endif
#ifdef IS_A2091
// FIX!  is this always true?  I hear it isn't!!!!!!!! FIX!!!!!!
#define CLOCK_7MHZ	1
#define WD33C93A	1
#endif

//=============================================================================
// Constants used throughout and also used for tuning purposes
//=============================================================================
//#define TRUE		-1
//#define FALSE		0

#define LONG_SPINUP_DELAY 15	// 15 second spinup time for slow drives

// Make sure DEVSTACK is a multiple of 4!
#define DEVSTACK	1000			// size of device task stacks
#define HD_PRI		10			// priority of startup task
#define INTNUM		3 	// (INTB_PORTS)	4703 interrupt we service
//#define INTNUM	13
#define INTPRI		20			// priority of our int server

#ifdef IS_IDE
#define CMD_BLKS	2			// I don't think I need more?
#endif
#ifndef IS_IDE
#define CMD_BLKS	10			// number of cmd blks to allocate
//#define CMD_BLKS	2			// number of cmd blks to allocate
#endif

#ifdef SCSI_SUPPORTED
#define MAX_LUN		7
#define MAX_ADDR	7
#endif
#ifndef SCSI_SUPPORTED
#define MAX_LUN		0			// i.e. IDE only
#define MAX_ADDR	1
#endif
#define MAX_XT_ADDR	1			// A590 only

#define MAX_CMD		28			// highest IO command #

// enable this definition to do programmed I/O for everything
//#define NODMA 		1

// enable this definition to force longward access to SASR for writes
//#define LONGWORD_SASR	1

//enable this definition to build a 1.3 compatible bonus-ROM version of the driver
//#define BONUS		1


