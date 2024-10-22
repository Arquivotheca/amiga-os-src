/*****************************************************************************
*
*	$Id: SixteenBitCopper.c,v 39.2 92/11/20 17:42:00 Jim2 Exp $
*
******************************************************************************
*
*	$Log:	SixteenBitCopper.c,v $
 * Revision 39.2  92/11/20  17:42:00  Jim2
 * In copdis16 correct the parameter order of the second invokation
 * of dissamble_copper_instruction.  Also preincrement, not postincrement
 * Actual so the copied instruction is correct for the address.
 * When addresses are to be surpressed don't even print the
 * address of the instructions.
 * 
 * Revision 39.1  92/11/18  10:45:38  Jim2
 * First Release - works with remote wack.
 *
*
******************************************************************************/

#ifndef VERSION
    #ifdef AA_CHIPS
	#include "copdis_rev.h"
    #endif
    #ifdef AAA_CHIPS
	#include "copdisAAA_rev.h"
    #endif
#endif

/*****************************************************************************
*Credits:
*	Original code by Karl Lehenbauer, April 1989
*	Improved by Spencer
*	Intermediate copper disassembly by Jim2
******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include "v:include/graphics/GFX.H"
#include "v:include/graphics/gfxbase.h"
#include "v:include/graphics/Copper.h"
#include "v:include/graphics/View.h"

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include "CopDis.h"

#include "SixteenBitCopper.h"

#define READREG  0x8000
#define LONG_REG 0x4000

#define DISK     0x0F00
#define BLITTER  0x0E00
#define COPPER   0x0D00
#define AUDIO    0x0C00
#define BITPLANE 0x0B00
#define SPRITE   0x0A00

#define ADDRESS 0x0002
#define COLOR   0x0001

#define ADD_UNKNOWN  ADDRESS
#define ADD_DISK     (DISK | ADDRESS)
#define ADD_BLITTER  (BLITTER | ADDRESS)
#define ADD_COPPER   (COPPER | ADDRESS)
#define ADD_AUDIO    (AUDIO | ADDRESS)
#define ADD_BITPLANE (BITPLANE | ADDRESS)
#define ADD_SPRITE   (SPRITE | ADDRESS)

#define ADD32_UNKNOWN  (ADD_UNKNOWN | LONG_REG)
#define ADD32_DISK     (ADD_DISK | LONG_REG)
#define ADD32_BLITTER  (ADD_BLITTER | LONG_REG)
#define ADD32_COPPER   (ADD_COPPER | LONG_REG)
#define ADD32_AUDIO    (ADD_AUDIO | LONG_REG)
#define ADD32_BITPLANE (ADD_BITPLANE | LONG_REG)
#define ADD32_SPRITE   (ADD_SPRITE | LONG_REG)

#define COLOR32 (COLOR | LONG_REG)

#define GET_COPINSTRUCTION(This,Here) CopDisReadBlock((APTR) (This), (APTR) (Here), sizeof (struct copinstruction))
#define GET_COPINS(This,Here) CopDisReadBlock((APTR) (This), (APTR) (Here), sizeof (struct CopIns))


#ifdef AAA_CHIPS
    #define DATA_SIZE ULONG
    #include "AAAFormatFunctions.h"
#else
    #define DATA_SIZE UWORD
    #ifdef AA_CHIPS
	#include "AAFormatFunctions.h"
	static struct Colors
    	{
    	    UBYTE Red, Green, Blue;
    	} ColorReg[32][2];
    #endif
#endif



STATIC VOID ParseNothing (DATA_SIZE This);
STATIC VOID DataStrobe (DATA_SIZE This);


#define AAAColor16 ParseNothing
#define AAAColor32 ParseNothing


/*****************************************************************************
*
*When a copper list involves moving to a register it is often useful to
*express the data as a series of constants rather than a hexidecimal number.
*A data type is provided for the functions that make this translation.
*Additionally, any such functions need to be forward declared so they may
*be placed in the initialized data structure.
*
******************************************************************************/


typedef VOID (DataFormat (DATA_SIZE));


/****** copregstruct *********************************************************
*
*This structure is used to gather information about the destination of a
*copper move instruction.
*
******************************************************************************/

struct copregstruct
{
    char *regname;
    char *longname;
    DataFormat *Format;
    UWORD flags;
};

static struct copregstruct copregisters[] =
{
#ifdef AA_CHIPS
    #include "AAChipRegisters.H"

#else
    #ifdef AAA_CHIPS
	#include "AAAChipRegisters.H"
    #else
	{"ILLEGAL REGISTER", "",ParseNothing, 0}						/* 0x000 */
	#define MAX_CHIP_REGISTER 0
	#define REGISTER_MASK 0x0000
    #endif
#endif

};



/****** ParseNothing *********************************************************
*
*   NAME
*	ParseNothing - Does nothing with the data passed to it.
*
*   SYNOPSIS
*	ParseNothing (This)
*
*	VOID ParseNothing (DATA_SIZE);
*
*   FUNCTION
*	This function is a placeholder for those register that either should
*	not, or do not yet have a translator for their data.
*
*   INPUTS
*	This	 - Register value to be parsed.
*
*   RESULT
*	Prints a string containing interpretation of the register value.
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
STATIC VOID ParseNothing (DATA_SIZE This)
{
	CopDisVPrintf ("");
} /* ParseNothing */


/****** DataStrobe ***********************************************************
*
*   NAME
*	DataStrobe - Prints the text > Data Strobe<.
*
*   SYNOPSIS
*	DataStrobe (This)
*
*	VOID DataStrobe (DATA_SIZE);
*
*   FUNCTION
*	This function is utilized by those registers are simple data strobes.
*	Using the function creates some useful output for these register for
*	the novice user who cannot recognize the register names.
*
*   INPUTS
*	This	 - Register value to be parsed.
*
*   RESULT
*	Prints a string containing interpretation of the register value.
*
*   BUGS
*
*   LOCALS
*	NONE
*
******************************************************************************/
STATIC VOID DataStrobe (DATA_SIZE This)
{
	CopDisVPrintf (" DataStrobe");
} /* DataStrobe */





/****** find_copper_register_info ********************************************
*
*   NAME
*	find_copper_register_info - Finds the record for the copper register.
*
*   SYNOPSIS
*	RegisterInfo = find_copper_register_info (reg)
*
*	struct copregstruct * find_copper_register_info (UWORD)
*
*   FUNCTION
*	Uses the register address to index an array containing register
*	specific information to be used in the instruction disassembly.
*
*   INPUTS
*	reg - Address of register for which the information is to be found.
*
*   RESULT
*	Pointer to a structure that describes the register.
*
*   BUGS
*
******************************************************************************/
STATIC struct copregstruct *find_copper_register_info(UWORD reg)
{
    reg &= REGISTER_MASK;
#ifdef AAA_Chips
    if (reg > 0x01FF)
	return (&(copregisters[((((reg > MAX_CHIP_REGISTERS) ? MAX_CHIP_REGISTER + 2 : reg) - 0x0200) >> 2) + 0x0100]));
#endif
    return (&(copregisters[((reg > MAX_CHIP_REGISTER) ? MAX_CHIP_REGISTER + 2 : reg) >> 1]));
} /* find_copper_register */


/****** FixColor *************************************************************
*
*   NAME
*	FixColor - create a color map entry.
*
*   SYNOPSIS
*	FixColor (Index, Data, Frame)
*
*	struct copregstruct * find_copper_register_info (UWORD)
*
*   FUNCTION
*	Uses the register address to index an array containing register
*	specific information to be used in the instruction disassembly.
*
*   INPUTS
*	reg - Address of register for which the information is to be found.
*
*   RESULT
*	Pointer to a structure that describes the register.
*
*   BUGS
*
******************************************************************************/
STATIC VOID FixColors (UWORD Index, UWORD Data, UWORD Field)
{
#ifdef AA_CHIPS
    if (LocState (Field))
    {
	ColorReg[Index][Field].Red = (ColorReg[Index][Field].Red & 0xF0) + ((Data & 0x0F00) >> 8);
	ColorReg[Index][Field].Green = (ColorReg[Index][Field].Green & 0xF0) + ((Data & 0x00F0) >> 4);
	ColorReg[Index][Field].Blue = (ColorReg[Index][Field].Blue & 0xF0) + (Data & 0x000F);
	if (Data & 0xF000)
	    CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
    }
    else
    {
	ColorReg[Index][Field].Red = ((Data & 0x0F00) >> 4) + ((Data & 0x0F00) >> 8);
	ColorReg[Index][Field].Green =  (Data & 0x00F0) + ((Data & 0x00F0) >> 4);
	ColorReg[Index][Field].Blue = ((Data & 0x000F) << 4) + (Data & 0x000F);
	if (Data & 0x7000)
	    CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
	if (Data & 0x8000)
	    CopDisVPrintf (" Trans");
    }
#endif
#ifdef AAA_CHIPS
    if ((Data) & 0x7000)
	CopDisVPrintf ("\n\t\t\t****Unused bits in data nonzero\t");
    if ((Data) & 0x8000)
	CopDisVPrintf (" Trans");
#endif
} /* FixColor */

/****** disassemble_copper_instruction ***************************************
*
*   NAME
*	disassemble_copper_instruction - Translates copper instruction into english.
*
*   SYNOPSIS
*	Next = disassemble_copper_instruction(ip, comm, data, nocolours,
*		LastColour, ShowOpcode, ShowConst)
*
*	struct copinstruction * disassemble_copper_instruction
*		(struct copinstruction *, BOOL, BOOL, BOOL, BOOL *, BOOL, BOOL);
*
*   FUNCTION
*	Take a copper machine instruction and produce a copper 'assembly'
*	instruction.
*
*	The basic format of the disassembly consists of address, machine code,
*	opcode, operands and comments.
*
*	The copper 'assembly' consists of the following four opcodes:  WAIT, SKIP,
*	MOVE and MOVEL.
*
*	There are at least two and at most five operands for WAIT and SKIP opcodes.
*	The first two operand, these are always present, are the horizontal and
*	vertical positions for the WAIT, or SKIP.  If the WAIT also requires the
*	blitter be finished the operand >bfd< is added.  Finally, if the masking
*	bits are not all set to one the masks are given.
*
*	The operands for MOVE and MOVEL consist of the data to be written followed
*	by the register name.  The data is not an address a comment will break down
*	the data to the constants that make up the data.  A long description of the
*	register completes the comment.
*
*	There is an option available to suppress any address dependent information.
*	This suppression is accomplished by setting all adresses to zero.
*	This includes the instruction address information.
*
*	Portions of the disassembly may not be printed due to the setting of
*	flags.  The most interesting of these is the nocolours flag.  When this
*	is set true no moves to the color registers are printed.  Instead a
*	single line is printed to mark the place of these writes.
*
*   INPUTS
*	ip         - Pointer to copper instruction to be disassembled.
*	Actual     - Address where the copper instruction is stored.  This can be
*		     an address in a remote machine.
*	comm       - TRUE the long description for the register is to be printed.
*	data       - FALSE the address of the data is to be printed.
*	nocolours  - TRUE don't print moves to the color registers.
*	LastColour - TRUE the last instruction was a supressed color register.
*	ShowOpcode - TRUE print the Copper machine code.
*	ShowConst  - TRUE print the translation of the register data.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	HighOrderOpcode - Move machine code for the first write to an address
*			  register pair.
*	Addr		- Data to be written to the first register of an address
*			  register pair.
*	regp	 	- Pointer to record current register.
*
******************************************************************************/
STATIC struct copinstruction * disassemble_copper_instruction(struct copinstruction *ip, struct copinstruction *Actual, BOOL comm, BOOL data, BOOL nocolours, BOOL *WasColour, BOOL ShowOpcode, BOOL ShowConst)
{
    UWORD HighOrderOpcode;
    ULONG Addr, ColorRegIndex;
    struct copregstruct *regp;

    if (ip->ir1 & 1)
    {
						/* true if it's a branch or skip instruction */
	if (!data)
	    CopDisVPrintf("%08lx  ", Actual);
	if (ShowOpcode)
	    CopDisVPrintf("%04lx %04lx\t\t", ip->ir1, ip->ir2);
	CopDisVPrintf("%s\t%svpos=%lx,hpos=%lx",
			((ip->ir2 & 1) ? "SKIP" : "WAIT"),
						/* reverse sense of blitter finish wait disable to show blitter
		 				* finish wait -- that way we occasionally say "blitter" instead
		 				* of almost always saying "no blitter"
		 				*/
			((ip->ir2 & 0x8000) ? "" : "bf,"),
			((ip->ir1 >> 8) & 0xff),
			((ip->ir1 >> 1) & 0x7f));

						/* if enable masks are not basically "all", print them */
	if ((ip->ir2 & 0x7FFE) != 0x7FFE)
	    CopDisVPrintf(",vcomp=%02lx,hcomp=%02lx",((ip->ir2 &0x7FFE)>>8), ((ip->ir2 &0x7FFE) >> 1));
	CopDisVPrintf("\n");
	*WasColour = FALSE;
    }
    else
    {
						/* it's a move instruction */

						/* calculate register ID and look it up */
	regp = find_copper_register_info(ip->ir1);
	if (!((nocolours) && (regp->flags & COLOR)))
	{
	    *WasColour = FALSE;
	    if (!data)
		CopDisVPrintf("%08lx  ", Actual);
	    if (ShowOpcode)
		CopDisVPrintf ("%04lx %04lx", ip-> ir1,(((data) && (regp-> flags & ADDRESS)) ? 0 : ip->ir2));
	    if (!((regp->flags & ADDRESS) || (regp->flags & LONG_REG)))
	    {
		if (ShowOpcode)
						/* If machine code is printed leave room for the machine code of
						*  a MOVEL
						*/
		    CopDisVPrintf("\t\t");
		CopDisVPrintf("MOVE\t%04lx,%s%s\t",
				(((data) && (regp-> flags & ADDRESS)) ? 0 : ip->ir2),
				regp-> regname,
				((ip->ir1 & 0xFE01) ? "\n\t\t\t***Reserved bits non-zero" : ""));
		if (ShowConst)
						/* For the color register write, we really want to se a break
						*  down of the RGB value written.  Also for AA assemble the eight
						*  bit value from the two writes.
						*/
		    if (regp->flags & COLOR)
		    {
			ColorRegIndex = (ip->ir1 & REGISTER_MASK - 0x180) >> 1;
			FixColors (ColorRegIndex , ip->ir2, 0);
#ifdef AA_CHIPS
			CopDisVPrintf (" Red=%02lx Green=%02lx Blue=%02lx", ColorReg[ColorRegIndex][0].Red, ColorReg[ColorRegIndex][0].Green, ColorReg[ColorRegIndex][0].Blue);
#endif
#ifdef AAA_CHIPS
			CopDisVPrintf (" Red=%02lx Green=%02lx Blue=%02lx", ((ip->ir2) & 0x0F00) >> 4, (ip->ir2) & 0x00F0, ((ip->ir2) & 0x000F) << 4);
#endif
		    }
		    else
		    	(regp-> Format) (ip->ir2);
	    }
	    else

						/* The MOVE is to an address register.  These really should appear
						*  in pairs.  If they do the four words will be disassembled as
						*  a MOVEL.  In order for this to happen the first move must be
						*  of the high order address bits.  If either of these conditions
						*  is not met an error is generated and the code is disassembed as
						*  a MOVE.
						*/
	    {
		if ((ip-> ir1 & 0x2) == 0)
		{
						/* The address was one for a high order address bits. */
		    Addr = (data ? 0 : ip-> ir2);
		    HighOrderOpcode = ip-> ir1;
		    GET_COPINSTRUCTION (++Actual, ip);
		    if (ip-> ir1 != HighOrderOpcode + 2)
		    {
						/* The next instruction is not a move to the next address. */

			if (ShowOpcode)
			    CopDisVPrintf ("\t\t");
			CopDisVPrintf ("MOVE\t%04lx,%sh\n\t\t\t***Missing low order address%s\t",
                			Addr,
					regp-> regname,
					((HighOrderOpcode & 0xFE00) ? "\n\t\t\t***Reserved bits non-zero" : ""));
						/* Backtrack the instruction pointer since the new instruction has
						*  not yet been disassembled.
						*/
			GET_COPINSTRUCTION(--Actual, ip);
		    }
		    else
		    {
						/* Yes, a valid MOVEL. */

			Addr = (Addr << 16) + (data ? 0 : ip-> ir2);
			if (ShowOpcode)
			    CopDisVPrintf (" %04lx %04lx\t", ip-> ir1,(data ? 0 : ip-> ir2));
			CopDisVPrintf("MOVEL\t%08lx,%s%s\t",
					Addr,
					regp-> regname,
					(((HighOrderOpcode & 0xFE00) || (ip-> ir1 & 0xFE00)) ? "\n\t\t\t***Reserved bits non-zero" : ""));
		    }
		}
		else
		{
						/* The address was for low order address bits without high order. */
		    if (ShowOpcode)
			CopDisVPrintf ("\t\t");
		    CopDisVPrintf ("MOVE\t%04lx,%sl\n\t\t\t***Missing high order address%s\t",
					(data ? 0 : ip-> ir2),
					regp-> regname,
					((ip-> ir1 & 0xFE00) ? "\n\t\t\t***Reserved bits non-zero" : ""));
		}
	    }
	    if ((comm) && (regp->longname != ""))
		CopDisVPrintf("\t;%s",regp->longname);
	    CopDisVPrintf("\n");
	}
	else
	{
	    if (!(*WasColour))
		CopDisVPrintf ("\t\t\t\t***Colour registers - surpressed\n");
	    *WasColour = TRUE;
	}
    }
						/* Move to the next insturction. */
    GET_COPINSTRUCTION (++Actual, ip);
    return (Actual);
} /* disassemble_copper_instruction */



/****** copdis16 *************************************************************
*
*   NAME
*	copdis16 - Disassembles a sixteen bit copper list.
*
*   SYNOPSIS
*	copdis16(coplist, comm, data, nocolours, ShowOpcode, ShowConst)
*
*	VOID copdis16 (struct copinstruction *, BOOL, BOOL, BOOL, BOOL, BOOL);
*
*   FUNCTION
*	Top level control of copper list disassembly.  Given a non NULL pointer
*	to a copper list, instructions are disassembled until the instruction
*	for wait until end of frame is encountered.
*
*   INPUTS
*	coplist	   - Pointer to the start of the copper list.
*	comm	   - TRUE the comment field for the register is to be printed.
*	data	   - FALSE the address of the data is to be printed.
*	nocolours  - TRUE don't print moves to the color registers.
*	ShowOpcode - TRUE print the raw copper instruction.
*	ShowConst  - TRUE print the translation of data written.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	WasColour - TRUE if the last instruction disassembled was a surpressed
*		    move to a color register.
*	I	  - Loop counter.
*
******************************************************************************/
extern VOID copdis16(struct copinstruction * coplist, BOOL comm, BOOL data, BOOL nocolours, BOOL ShowOpcode, BOOL ShowConst)
{
						/* check and refuse null list pointers */
    if (coplist == NULL)
	CopDisVPrintf("copdis: copper list pointer is null\n");
    else

						/* while we don't have an instruction of 0xffff 0xfffe, which is
						 * a de facto END PROGRAM instruction, print instructions.
						 *
						 * This should really have a sanity check (like no more than a few
						 * hundred instructions, and/or a length -- heck, it's in the cprlst
						 * and CopList structures anyway), but it doesn't, or at least not
						 * yet
						 */
    {
	struct copinstruction * Buffer;
	if ((Buffer = AllocMem (sizeof(struct copinstruction), MEMF_CLEAR)) != NULL)
	{
	    BOOL WasColour = FALSE;
#ifdef AA_Chips
	    int I;
	    ResetAADisplay();
	    for (I = 0; I < 32; I++)
	    {
		ColorReg[I][0]. Red = 0;
		ColorReg[I][0]. Green = 0;
		ColorReg[I][0]. Blue = 0;
	    }
#endif
#ifdef AAA_CHIPS
	    ResetAAADisplay();
#endif
	    SetField (CPR_NT_SHT);
	    GET_COPINSTRUCTION (coplist, Buffer);
	    do
		coplist = disassemble_copper_instruction(Buffer, coplist, comm, data, nocolours, &WasColour, ShowOpcode, ShowConst);
	    while ((Buffer->ir1 != 0xffff) || (Buffer->ir2 != 0xfffe));

	/* and disassemble the WAIT 255 instruction */
	    disassemble_copper_instruction(Buffer, coplist, comm, data, nocolours, &WasColour, ShowOpcode, ShowConst);
	    FreeMem (Buffer, sizeof(struct copinstruction));
	}
    }
} /* copdis16 */

/****** FormatAddress ********************************************************
*
*   NAME
*	FormatAddress - Formats the disassembly for of a intermediate copper list
*		move to an address register..
*
*   SYNOPSIS
*	Next = FormatAddress (Instruction, CurrentVP NoAddress, ShowOpcode)
*
*	struct CopIns * FormatAddress (struct CopIns *, struct ViewPort *, BOOL, BOOL);
*
*   FUNCTION
*	Moves to address register pairs should be done with two consecutive moves;
*	first the high order bits, then then the low order bits.
*
*	This routine is called when a write to an address register is detected.  If
*	this register is for the high order bits the next instruction is checked
*	skipping any extention blocks (a message is output noting the existance of
*	such blocks).  If this next instruction is a write to a register for the
*	low order address bits the complete address is assembled.
*
*	If the user has not surpressed addresses this assembled address is written
*	as an operand of the a MOVEL opcode.  If the addresses have been surpressed
*	and the register is not for a bit plane, a zero is written as the address.
*	For bit plane pointers, the current ViewPort is checked to find the offset
*	of the loaded address from the corresponding ViewPort bit plane start.
*
*
*   INPUTS
*	Instruction - Pointer to a copy of first move instruction of the pair for
*		      an address.
*	Actual      - Pointer to the actual instruction.
*	CurrentVP   - Pointer to the ViewPort to which this copper list is attached.
*	NoAddress   - TRUE set addresses to zero, unless this is a bitplane pointer.
*	ShowOpcode  - TRUE show the intermediate copper list machine code.
*
*   RESULT
*	Pointer to the last instruction disassembled.
*
*   BUGS
*
*   LOCALS
*	Addr		- Current known address being moved.
*	ArrayIndex	- Offset from start of bit plane to address being moved.
*	HighOrderReg	- Register value from first move.
*	HighOrderOpcode - Opcode from first move (used for matching field information).
*	BitPlane	- Index for bit plane written.
*
******************************************************************************/
STATIC struct CopIns * FormatAddress (struct CopIns * Instruction, struct CopIns * Actual, struct ViewPort *CurrentVP, BOOL NoAddress, BOOL ShowOpcode)
{
    LONG Addr, ArrayIndex;
    WORD HighOrderReg, HighOrderOpcode;
    int BitPlane;


    Addr = Instruction-> DESTDATA;
    HighOrderReg = Instruction-> DESTADDR;
    HighOrderOpcode = Instruction-> OpCode;
    if ((Addr & 0x2) == 0)
    {
						/* Move of high order address bits. */
	GET_COPINS(++Actual, Instruction);
	if ((Instruction-> OpCode & 0x000F) == CPRNXTBUF)
	    if (ShowOpcode)
		CopDisVPrintf ("\n\t\t *** Intervening extention blocks\n\t\t\t");
	    else
		CopDisVPrintf ("\n\t\t *** Intervening extention blocsk\n\t");
	while (((Instruction-> OpCode &0x000F) == CPRNXTBUF) && (Actual != NULL))
						/* Skip any extention blocks. */
	    if (Instruction-> NXTLIST != NULL)
		GET_COPINS ((Actual = (struct CopIns *) CopDisFindPointer((APTR) ((struct CopList *) CopDisFindPointer ((APTR) Instruction-> NXTLIST))-> CopIns)), Instruction);
	    else
		Actual = NULL;
	if (Actual == NULL)
	{
						/* No second move */
	    if (ShowOpcode)
		CopDisVPrintf ("\t\t");
	    CopDisVPrintf ("MOVE\t%04lx,%sh", ((NoAddress) ? 0 : Addr), find_copper_register_info(HighOrderReg)-> regname);
	    if ((HighOrderOpcode & (CPR_NT_LOF | CPR_NT_SHT)) != 0)
		if ((HighOrderOpcode & CPR_NT_LOF) != 0)
		    CopDisVPrintf ("-S");
		else
		    CopDisVPrintf ("-L");
	    CopDisVPrintf ("\n\t\t\t ***Unexpected EOList wanted low order address%s\t",
				((HighOrderReg & 0xFE01) ? "\n\t\t\t***Reserved bits non-zero" : ""));
	}
	else
	    if ((Instruction-> OpCode == HighOrderOpcode) && (Instruction-> DESTADDR == HighOrderReg + 2))
	    {
						/* Second move to the correct register */
		if (ShowOpcode)
		    CopDisVPrintf ("%04lx %04lx %04lx\t", (UWORD) Instruction-> OpCode, (UWORD) Instruction-> DESTADDR, (UWORD) ((NoAddress) ? 0 : Instruction->DESTDATA));
		CopDisVPrintf ("MOVEL\t");
		Addr = (Addr << 16) + ((UWORD)Instruction-> DESTDATA);
		if ((copregisters[HighOrderReg >>1]. flags & ADD_BITPLANE) && (NoAddress))
		{
						/* Moving to a bitplane register. */
#ifdef AAA_CHIPS
		    if (HighOrderReg > 0x400)
			BitPlane = (HighOrderReg & 0x003C);
		    else
#endif
			BitPlane = (HighOrderReg & 0x001C);
		    BitPlane = BitPlane >> 2;
		    ArrayIndex = (ULONG) CopDisFindPointer(&CurrentVP->RasInfo);
		    if (((CopDisFindWord(&CurrentVP-> Modes) & DUALPF) != 0) && ((BitPlane & 0x0001) == 0))
			ArrayIndex = Addr - ((LONG)CopDisFindPointer(((PLANEPTR *)&(((struct BitMap *)CopDisFindPointer(
				&(((struct RasInfo *) CopDisFindPointer(&(((struct RasInfo *) ArrayIndex)->Next)))
				->BitMap)))-> Planes)) + BitPlane));
		    else
			ArrayIndex = Addr - ((LONG)CopDisFindPointer(((PLANEPTR *) &(((struct BitMap *)CopDisFindPointer(
				&(((struct RasInfo *) ArrayIndex)->BitMap)))-> Planes)) + BitPlane));

		    CopDisVPrintf ("%lx(bp%lx)", ArrayIndex, BitPlane);
		    if ((Instruction-> OpCode & (CPR_NT_LOF | CPR_NT_SHT)) != 0)
			if ((Instruction-> OpCode & CPR_NT_LOF) != 0)
			    CopDisVPrintf ("-S\t");
			else
			    CopDisVPrintf ("-L\t");
		}
		else
						/* Moving to a non-bitplane register. */
		    CopDisVPrintf("%08lx", ((NoAddress) ? 0 : Addr));
		CopDisVPrintf(",%s%s\t", find_copper_register_info(HighOrderReg)-> regname, (((HighOrderReg & 0xFE00) || (Instruction-> DESTADDR & 0xFE01)) ? "\n\t\t\t***Reserved bits non-zero" : ""));
	    }
	    else
	    {
						/* Next instruction is not a MOVE to the correct register */
		if (ShowOpcode)
		    CopDisVPrintf ("\t\t");
		CopDisVPrintf ("MOVE\t%04lx,%sh", ((NoAddress) ? 0 : Addr), find_copper_register_info(HighOrderReg)-> regname);
		if ((HighOrderOpcode & (CPR_NT_LOF | CPR_NT_SHT)) != 0)
		    if ((HighOrderOpcode & CPR_NT_LOF) != 0)
			CopDisVPrintf ("-S");
		    else
			CopDisVPrintf ("-L");
		CopDisVPrintf ("\n\t\t\t ***Missing low order address%s\t",
				((HighOrderReg & 0xFE01) ? "\n\t\t\t***Reserved bits non-zero" : ""));
						/* The 'next' instruction has not yet been disassmbled. */
		GET_COPINS(Actual--, Instruction);
	    }
    }
    else
							/* Move of high order address bits. */
    {
	if (ShowOpcode)
	    CopDisVPrintf ("\t\t");
	CopDisVPrintf ("MOVE\t%04lx,%sh",
			((NoAddress) ? 0 : Addr), find_copper_register_info(HighOrderReg)-> regname);
	if ((HighOrderOpcode & (CPR_NT_LOF | CPR_NT_SHT)) != 0)
	    if ((HighOrderOpcode & CPR_NT_LOF) != 0)
		CopDisVPrintf ("-S");
	    else
		CopDisVPrintf ("-L");
	CopDisVPrintf ("\n\t\t\t ***Missing high order address%s\t",
				((HighOrderReg & 0xFE01) ? "\n\t\t\t***Reserved bits non-zero" : ""));
    }
    return (Actual);

}

/****** DisassICopperInstruction *********************************************
*
*   NAME
*	DisassICopperInstruction16 - Translates an intermediate copper instruction into english.
*
*   SYNOPSIS
*	Next = DisassICopperInstruction(Instruction, Comment, NoAddress, SkipColours,
*		WasColour, ShowOpcode, ShowConst)
*
*	struct CopIns * DisassICopperInstruction
*		(struct CopIns *, BOOL, BOOL, BOOL, BOOL *, BOOL, BOOL);
*
*   FUNCTION
*	Take an intermediate copper machine instruction and produce an intermediate
*	copper 'assembly' instruction.
*
*	The basic output format consists of address, machine code, opcode, operands
*	and comments.
*
*	The intermediate copper 'assembly' consists of the following three opcodes:
*	WAIT, MOVE and MOVEL.
*
*	There are at least two and and most five operands for the WAIT opcode.  The
*	first two operands, always present, are the horizontal and vertical positions
*	for the WAIT.  >bfd< is the third operand and is only present if the bitter
*	must be finished to satisfy the wait.  If the masking bits are not all set
*	to one the masks are given as the final two operands.
*
*	The operands for MOVE and MOVEL consist of the data to be written followed
*	by the register name.  The register name may have a -L or a -S appended if
*	the MOVE is specific to either the long Field, or the short Field.  If the
*	data is not an address a comment will break down the data into the constants
*	that make up the data.  A long description of the register complete the
*	comment field.
*
*	There is an option available to suppress any address dependnt information.
*	This suppression is accomplished by setting all addresses to zero, except
*	for bitplane pointers (See FormatAddress for a description of what is done
*	in this case).
*
*	Portions of the disassembly may not be printed due to the setting of flags.
*	The most interesting of these is the SkipColours flag.  When this flag is
*	set to TRUE no moves to the color registers are printed.  Instead a single
*	line is printed to make the palce of these writes.
*
*   INPUTS
*	Instruction - Pointer to instruction to be disassembled.
*	Comment     - TRUE the comment field for the register is to be printed.
*	NoAddress   - FALSE the address of the data is to be printed.
*	SkipColours - TRUE don't print moves to the color registers.
*	WasColour   - TRUE the last instruction was a suppressed move to a color
*		      register.
*	ShowOpcode  - TRUE print the machine code.
*	ShowConst   - TRUE print the constant break down of the register.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	RegisterInfo - Pointer to record containing information about the
*		       destination register for a move.
*	ColorRegIndex	- Index into an array containing current values for color
*			  registers.
*	Field         - Field of a write to a color register.
*
******************************************************************************/
STATIC struct CopIns * DisassICopperInstruction (struct CopIns * Instruction, struct CopIns * Actual, struct ViewPort * CurrentVP,
		BOOL Comment, BOOL NoAddress, BOOL SkipColours, BOOL *WasColour, BOOL ShowOpcode, BOOL ShowConst)
{
    struct copregstruct *RegisterInfo;
    int ColorRegIndex, Field;


    switch ((Instruction-> OpCode) & 0x000F)
    {
	case COPPER_WAIT :
	    *WasColour = FALSE;
	    if (!NoAddress)
		CopDisVPrintf("%08lx  ", Actual);
	    if (ShowOpcode)
		CopDisVPrintf("%04lx %04lx %04lx\t\t%s", (UWORD) Instruction->OpCode, (UWORD) Instruction->VWAITPOS, (UWORD) Instruction->HWAITPOS, NoAddress ? "\t" : "");
	    CopDisVPrintf("WAIT\tvpos=%lx,hpos=%lx\n",  (UWORD) Instruction->VWAITPOS, (UWORD) Instruction-> HWAITPOS);
	    if (Instruction-> HWAITPOS != 255)
		GET_COPINS (++Actual, Instruction);
	    else
		Actual = NULL;
	    break;
	case COPPER_MOVE :
	    RegisterInfo = find_copper_register_info (Instruction-> DESTADDR);
	    if (!SkipColours || ((RegisterInfo-> flags & COLOR) == 0))
	    {
						/* Disassembe this code. */
		*WasColour = FALSE;
		if (!NoAddress)
		    CopDisVPrintf ("%08lx  ", Actual);
		if (ShowOpcode)
		    CopDisVPrintf ("%04lx %04lx %04lx ",
							(UWORD) Instruction->OpCode, (UWORD) Instruction->DESTADDR,
							(UWORD) (((NoAddress) && (RegisterInfo->flags & ADDRESS)) ? 0 : Instruction-> DESTDATA));
		if ((RegisterInfo-> flags & ADDRESS) || (RegisterInfo-> flags & LONG_REG))
		    Actual = FormatAddress (Instruction, Actual, CurrentVP, NoAddress || (RegisterInfo->flags & LONG_REG), ShowOpcode);
		else
		{
		    if (ShowOpcode)
			CopDisVPrintf ("\t\t%s", NoAddress ? "\t" : "");
		    CopDisVPrintf ("MOVE\t%04lx,%s", (UWORD) Instruction-> DESTDATA, RegisterInfo-> regname);
		    if (SetField(Instruction-> OpCode & (CPR_NT_LOF | CPR_NT_SHT)) != 0)
			if ((Instruction-> OpCode & CPR_NT_LOF) != 0)
			    CopDisVPrintf ("-S");
			else
			    CopDisVPrintf ("-L");
		    CopDisVPrintf ("\t%s", ((Instruction->DESTADDR & 0xFE01) ? "\n\t\t\t***Reserved bits non-zero" : ""));
		    if (ShowConst)
			if (RegisterInfo->flags & COLOR)
			{
			    ColorRegIndex = (Instruction->DESTADDR & REGISTER_MASK - 0x180) >> 1;
			    Field = 0;
			    if ((Instruction-> OpCode & (CPR_NT_LOF | CPR_NT_SHT)) != 0)
				if ((Instruction-> OpCode & CPR_NT_LOF) != 0)
				{
				    FixColors (ColorRegIndex, Instruction->DESTDATA, 1);
				    Field++;
				}
				else
				    FixColors (ColorRegIndex, Instruction->DESTDATA, 0);
			    else
			    {
				FixColors (ColorRegIndex, Instruction->DESTDATA, 0);
#ifdef AA_CHIPS
				FixColors (ColorRegIndex, Instruction->DESTDATA, 1);
#endif
			    }
#ifdef AA_CHIPS
			    if ((ColorReg[ColorRegIndex][0].Red == ColorReg[ColorRegIndex][1].Red) &&
					(ColorReg[ColorRegIndex][0].Green == ColorReg[ColorRegIndex][1].Green) &&
					(ColorReg[ColorRegIndex][0].Blue == ColorReg[ColorRegIndex][1].Blue) ||
					(Instruction-> OpCode ^ (CPR_NT_LOF | CPR_NT_SHT)))
				CopDisVPrintf (" Red=%02lx Green=%02lx Blue=%02lx", ColorReg[ColorRegIndex][Field].Red, ColorReg[ColorRegIndex][Field].Green, ColorReg[ColorRegIndex][Field].Blue);
			    else
			    {
				if (Instruction-> OpCode & CPR_NT_SHT)
				    CopDisVPrintf (" Red=%02lx Green=%02lx Blue=%02lx", ColorReg[ColorRegIndex][0].Red, ColorReg[ColorRegIndex][0].Green, ColorReg[ColorRegIndex][0].Blue);
				if (Instruction-> OpCode & CPR_NT_SHT)
				    CopDisVPrintf (" Red=%02lx Green=%02lx Blue=%02lx", ColorReg[ColorRegIndex][1].Red, ColorReg[ColorRegIndex][1].Green, ColorReg[ColorRegIndex][1].Blue);
			    }
#endif
#ifdef AAA_CHIPS
			    CopDisVPrintf (" Red=%02lx Green=%02lx Blue=%02lx", ((Instruction->DESTDATA) & 0x0F00) >> 4, (Instruction->DESTDATA) & 0x00F0, ((Instruction->DESTDATA) & 0x000F) << 4);
#endif

			}
			else
			    (RegisterInfo-> Format) (Instruction-> DESTDATA);
		}
		if (Comment)
		    CopDisVPrintf("\t;%s", RegisterInfo-> longname);
		CopDisVPrintf ("\n");
	    }
	    else
	    {
		if (!(*WasColour))
		    CopDisVPrintf ("\t\t\t\t***Colour registers - surpressed\n");
		*WasColour = TRUE;
	    }
	    GET_COPINS(++Actual, Instruction);
	    break;
	case  CPRNXTBUF :
	    CopDisVPrintf ("\t\t\t*** Linked intermediate copper lists\n");
	    if (Instruction-> NXTLIST != NULL)
		GET_COPINS ((Actual = (struct CopIns *) CopDisFindPointer((APTR) ((struct CopList *) CopDisFindPointer ((APTR) Instruction-> NXTLIST))-> CopIns)), Instruction);
	    break;
	default:
	    CopDisVPrintf ("***** Illegal Intermediate Copper Instruction. *****\n");
	    break;
    }
    return (Actual);
}


/****** ICopDis16 ************************************************************
*
*   NAME
*	ICopDis16 - Disassembles an intermediate copper list.
*
*   SYNOPSIS
*	ICopDis16(Walking, Comments, NoAddress, SkipColours, ShowOpcode, ShowConst)
*
*	VOID ICopDis16 (struct CopIns *, BOOL, BOOL, BOOL, BOOL, BOOL);
*
*   FUNCTION
*	Top level control of the intermediate copper list disassembly.  Given a
*	non NULL pointer to an intermediate copper list, instructions are
*	disassembled until a NULL is encountered.
*
*   INPUTS
*	Walking	    - Pointer to the intermediate instruction to be disassembled.
*	Comments    - TRUE the comment field for the register is to be printed.
*	NoAddress   - FALSE the address of the data is to be printed.
*	SkipColours - TRUE don't print moves to the color registers.
*	ShowOpcode  - TRUE print the intermediate copper machine code.
*	ShowConst   - TRUE print the translation of data written.
*
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	WasColour - TRUE if the last instruction disassembled was a surpressed
*		    move to a color register.
*	I	  - Loop counter.
*
******************************************************************************/
extern VOID ICopDis16 (struct CopIns * Walking, struct ViewPort *CurrentVP, BOOL Comments, BOOL NoAddress, BOOL SkipColours, BOOL ShowOpcode, BOOL ShowConst)

{

	/* check and null pointers */
    if (Walking == NULL)
	CopDisVPrintf("No intermediate copper list\n");
    else
    {
	struct CopIns * Buffer;
	if ((Buffer = AllocMem (sizeof(struct CopIns), MEMF_CLEAR)) != NULL)
	{
	    BOOL WasColour = FALSE;
#ifdef AA_CHIPS
	    int I;
	    ResetAADisplay ();
	    for (I = 0; I < 32; I++)
	    {
		ColorReg[I][0]. Red = 0;
		ColorReg[I][0]. Green = 0;
		ColorReg[I][0]. Blue = 0;
		ColorReg[I][1]. Red = 0;
		ColorReg[I][1]. Green = 0;
		ColorReg[I][1]. Blue = 0;
	    }
#endif
#ifdef AAA_CHIPS
	    ResetAAADisplay();
#endif
	    CopDisVPrintf ("Viewport Addr=%08lx\n\n", CurrentVP);
	    GET_COPINS(Walking, Buffer);
	    do
	    {
	    	Walking = DisassICopperInstruction(Buffer, Walking, CurrentVP, Comments, NoAddress, SkipColours, &WasColour, ShowOpcode, ShowConst);
	    }
	    while (Walking != NULL);

	    CopDisVPrintf ("Completed ViewPort\n\n");
	}
    }
}
