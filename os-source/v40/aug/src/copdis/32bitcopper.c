/*****************************************************************************
*
*	$Id $
*
******************************************************************************
*
*	$Log $
*
******************************************************************************/

#ifndef VERSION
    #include "copdisAAA_rev.h"
#endif

#include <exec/types.h>
#include <exec/memory.h>
#include "v:include/graphics/GFX.H"
#include "v:include/graphics/gfxbase.h"
#include "v:include/graphics/Copper.h"
#include "v:include/graphics/View.h"

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include "CopDis.h"

#include "32BitCopper.h"

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

#define GET_COPINSTRUCTION(This,Here) CopDisReadBlock((APTR) (This), (APTR) (Here), sizeof (struct CopInstruction))
#define GET_COPINS(This,Here) CopDisReadBlock((APTR) (This), (APTR) (Here), sizeof (struct CopIns))


#define DATA_SIZE ULONG
#include "AAAFormatFunctions.h"


STATIC VOID ParseNothing (DATA_SIZE This);
STATIC VOID DataStrobe (DATA_SIZE This);
STATIC VOID AAAColor32 (ULONG This);
STATIC VOID AAAColor32 (ULONG This);



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


/****** RegisterInfo *********************************************************
*
*This structure is used to gather information about the destination of a
*copper move instruction.
*
******************************************************************************/

struct RegisterInfo
{
    char *Name, *Comment;
    DataFormat *Format;
    UWORD Type;
};

static struct RegisterInfo RegisterBank[] =
{
    #include "AAAChipRegisters.H"
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

/****** AAAColor32 ***********************************************************
*
*   NAME
*	AAAColor32 - Prints the colors for a write to a 32 bit color register.
*
*   SYNOPSIS
*	AAAColor32 (This)
*
*	VOID AAAColor32 (ULONG);
*
*   FUNCTION
*	Indentifies the individual RGB components from the write to the color
*	register.
*
*   INPUTS
*	This - Register value to be parsed.
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
STATIC VOID AAAColor32 (ULONG This)
{
    CopDisVPrintf (" Red=%02lx Green=%02lx Blue=%02lx, (This & 0x00FF0000) >> 16, (This & 0x0000FF00) >> 8, This & 0x000000FF);
} /* AAAColor32 */

/****** AAAColor16 ***********************************************************
*
*   NAME
*	AAAColor16 - Prints the colors for a write to a 16 bit color register.
*
*   SYNOPSIS
*	AAAColor16 (This)
*
*	VOID AAAColor16 (ULONG);
*
*   FUNCTION
*	Indentifies the individual RGB components from the write to the color
*	register.
*
*   INPUTS
*	This - Register value to be parsed.
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
STATIC VOID AAAColor16 (ULONG This)
{
    CopDisVPrintf (" Red=%02lx Green=%02lx Blue=%02lx", (This & 0x0F00) >> 4, This & 0x00F0, (This & 0x000F) << 4);
} /* AAAColor16 */




/****** FindRegisterInfo ******************************************************
*
*   NAME
*	FindRegisterInfo - Finds the record for the copper register.
*
*   SYNOPSIS
*	RegisterInfo = FindRegisterInfo (RegisterAddress)
*
*	struct RegisterInfo * FindRegisterInfo (UWORD)
*
*   FUNCTION
*	Uses the register address to index an array containing register
*	specific information to be used in the instruction disassembly.
*
*   INPUTS
*	RegisterAddrees - Address of register for which the information is to be
*			  found.
*
*   RESULT
*	Pointer to a structure that describes the register.
*
*   BUGS
*
******************************************************************************/
STATIC struct RegisterInfo *FindRegisterInfo(UWORD RegisterAddress)
{
    RegisterAddress &= REGISTER_MASK;
    if (RegisterAddress > 0x01FF)
	return (&(RegisterBank[(((RegisterAddress > MAX_CHIP_REGISTERS) ? MAX_CHIP_REGISTER + 2 : RegisterAddress) >> 2) + 0x0080]));
    return (&(RegisterBank[RegisterAddress >> 1]));
} /* FindRegisterInfo */


/****** DisassCopperInstruction **********************************************
*
*   NAME
*	DisassCopperInstruction - Translates copper instruction into english.
*
*   SYNOPSIS
*	Next = DisassCopperInstruction(Instruction, Actual, Comm, NoAddress, SkipColour,
*		WasColour, ShowOpcode, ShowConst)
*
*	struct CopInstruction * DisassCopperInstruction
*		(struct CopInstruction *, struct CopInstruction *, BOOL, BOOL, BOOL, BOOL *, BOOL, BOOL);
*
*   FUNCTION
*	Take a copper machine instruction and produce a copper 'assembly'
*	instruction.
*
*	The basic format of the disassembly consists of address, machine code,
*	opcode, operands and comments.
*
*	The copper 'assembly' consists of the following six opcodes:  WAIT32, SKIP32,
*	MOVE32, MOVEM, | and \.
*
*	There are at least two and at most five operands for WAIT and SKIP opcodes.
*	The first two operands (always present) are the horizontal and vertical
*	positions for the WAIT32, or SKIP32.  If the SKIP32 also requires the
*	blitter be finished the operand >bf< is added.  If the WAIT32 requires
*	strict equality the operand >GTDIS< is added.  Finally, if the masking
*	bits are not all set to one, the masks are given.
*
*	The operands for MOVE, MOVEM, | and \ consist of the data to be written
*	followed by the register name.  When the data is not an address a comment
*	may break down the data to the constants.  A long description of the
*	register completes the comment.
*
*	There is an option available to suppress any address dependent information.
*	This suppression is accomplished by setting all adresses to zero and not
*	printing the instruction address.
*
*	Portions of the disassembly may be supressed by the various flags.  The
*	most interesting of these is the SkipColours flag.  When this is set TRUE
*	no moves to the color registers are printed.  Instead a single line is
*	printed to mark the place of these writes.
*
*   INPUTS
*	Instruction - Pointer to instruction to be disassembled.
*	Actual      - Address where the copper instruction is stored.  This can be
*		      an address in a remote machine.
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
*	ThisRegister - Pointer to record containing information on the current register.
*
******************************************************************************/
STATIC struct CopInstruction * DisassCopperInstruction(struct CopInstruction *Instruction, struct CopInstruction *Actual, BOOL Comment, BOOL NoAddress, BOOL SkipColours, BOOL *WasColour, BOOL ShowOpcode, BOOL ShowConst)
{
    if (Instruction->ir1 & 0x00000001)
    {
						/* true if it's a branch or skip instruction */
	if (!NoAddress)
	    CopDisVPrintf("%08lx  ", Actual);
	if (ShowOpcode)
	    CopDisVPrintf("%08lx %08lx\t", Instruction->ir1, Instruction->ir2);
	if (Instruction0>ir2 & 0x000000001)
	    CopDisVPrintf ("SKIP32\t%s", (Instruction->ir2 & 0x00008000) ? "" : "bf,");
	else
	    CopDisVPrintf ("MOVE32\t%s", (Instruction->ir2 & 0x80000000) ? "" : "GTDIS,");

	CopDisVPrintf("vpos=%03lx,hpos=%03lx",(Instruction->ir1 >> 16) & 0x7ff, (Instruction->ir1 >> 1) & 0x3FE);

						/* if enable masks are not basically "all", print them */
	if ((Instruction->ir2 & 0x07FF07F8) != 0x07FF07F8)
	    CopDisVPrintf(",vcomp=%03lx,hcomp=%03lx",((Instruction->ir2 &0x07FF0000)>>16), ((Instruction->ir2 &0x000007F8) >> 1));
	CopDisVPrintf ("%s\n", ((Instruction->ir1 & 0xF800F806) ||
		(Instruction->ir2 & ((Instruction->ir2 & 0x00000001) : 0xF8007806 ? 0x7800F806)) ?
		 "\n\t\t\t***Reserved bits non-zero" : ""));

	*WasColour = FALSE;
	GET_COPINSTRUCTION(++Actual, Instruction);
    }
    else
    {
	struct RegisterInfo *ThisRegister;
						/* it's a move instruction */

						/* calculate register ID and look it up */
	ThisRegister = FindRegisterInfo(Instruction->ir1);
	if (!((SkipColours) && (ThisRegister->Type & COLOR)))
	{
	    *WasColour = FALSE;
	    if (!NoAddress)
		CopDisVPrintf("%08lx  ", Actual);
	    if (ShowOpcode)
		CopDisVPrintf ("%08lx %08lx\t", Instruction->ir1,(((NoAddress) && (ThisRegister->Type & ADDRESS)) ? 0 : Instruction->ir2));
	    CopDisVPrintf("MOVE%s\t%08lx,%s%s\t", (Instruction->ir1 & 0x00FF0000) ? "M" : "32",
			(((NoAddress) && (ThisRegister->Type & ADDRESS)) ? 0 : Instruction->ir2),
			ThisRegister->Name,
			((Instruction->ir1 & 0xFF00F000) ? "\n\t\t\t***Reserved bits non-zero" : ""));
	    if (ShowConst)
		(ThisRegister-> Format) (Instruction->ir2);
	    if ((Comment) && (Instruction->Comment != ""))
		CopDisVPrintf("\t;%s", Instruction->Comment);
	    if ((Instruction->Type & LONG_REG) != LONG_REG) && ((Instruction->ir2) >> 16) != (Instruction->ir2 & 0x0000FFFF)))
		CopDisVPrintf("\n\t\t\t***Invalid 16bit Register High word must equal Low word");
	    CopDisVPrintf("\n");
	}
	else
	{
	    if (!(*WasColour))
		CopDisVPrintf ("\t\t\t\t***Colour registers - surpressed\n");
	    *WasColour = TRUE;
	}
	Instruction->ir2 = (ULONG) CopDisFindPointer(++Actual);
	while ((Instruction->ir2 & 0x00FF0000) != 0)
	{
	    ThisRegister++;
	    if (!((SkipColours) && (ThisRegister->Type & COLOR)))
	    {
		*WasColour = FALSE;
	    	if (!NoAddress)
		    CopDisVPrintf("%08lx  ", Actual);
	    	if (ShowOpcode)
		    CopDisVPrintf ("%08lx\t\t", (((NoAddress) && (ThisRegister->Type & ADDRESS)) ? 0 : Instruction->ir2));
	    	CopDisVPrintf("%s\t%08lx,%s\t", ((Instruction->ir1 & 0x00FF0000) == 0x00010000) ? "\\" : "|",
			(((NoAddress) && (ThisRegister->Type & ADDRESS)) ? 0 : Instruction->ir2),
			ThisRegister->Name);
	    	if (ShowConst)
		    (ThisRegister-> Format) (Instruction->ir2);
	    	if ((Comment) && (Instruction->Comment != ""))
		    CopDisVPrintf("\t;%s", Instruction->Comment);
	    	if ((Instruction->Type & LONG_REG) != LONG_REG) && ((Instruction->ir2) >> 16) != (Instruction->ir2 & 0x0000FFFF)))
		    CopDisVPrintf("\n\t\t\t***Invalid 16bit Register High word must equal Low word");
	    	CopDisVPrintf("\n");
	    }
	    else
	    {
	    	if (!(*WasColour))
		    CopDisVPrintf ("\t\t\t\t***Colour registers - surpressed\n");
	    	*WasColour = TRUE;
	    }
	    Instruction->ir2 = (ULONG)CopDisFindPointer((APTR) ((ULONG) Actual + 4));
	    Instruction->ir1 -= 0x00010000;
	}
    }
						/* Move to the next insturction. */
    GET_COPINSTRUCTION (Actual, Instruction);
    return (Actual);
} /* DisassCopperInstruction */



/****** CopDis32 *************************************************************
*
*   NAME
*	CopDis32 - Disassembles a thirty-two bit copper list.
*
*   SYNOPSIS
*	CopDis32(coplist, comm, data, nocolours, ShowOpcode, ShowConst)
*
*	VOID CopDis32 (struct CopInstruction *, BOOL, BOOL, BOOL, BOOL, BOOL);
*
*   FUNCTION
*	Top level control of copper list disassembly.  Given a non NULL pointer
*	to a copper list, instructions are disassembled until the instruction
*	for wait until end of frame is encountered.
*
*   INPUTS
*	Actual      - Address where the copper instruction is stored.  This can be
*		      an address in a remote machine.
*	Comment     - TRUE the comment field for the register is to be printed.
*	NoAddress   - FALSE the address of the data is to be printed.
*	SkipColours - TRUE don't print moves to the color registers.
*	ShowOpcode  - TRUE print the machine code.
*	ShowConst   - TRUE print the constant break down of the register.
*   RESULT
*	NONE
*
*   BUGS
*
*   LOCALS
*	Buffer    - Address into which the copper instruction to be disassembled
*		    is copied.
*	WasColour - TRUE if the last instruction disassembled was a surpressed
*		    move to a color register.
*
******************************************************************************/
extern VOID CopDis32(struct CopInstruction * Actual, BOOL Comment, BOOL NoAddress, BOOL SkipColours, BOOL ShowOpcode, BOOL ShowConst)
{
						/* check and refuse null list pointers */
    if (Actual == NULL)
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
	struct CopInstruction * Buffer;
	if ((Buffer = AllocMem (sizeof(struct CopInstruction), MEMF_CLEAR)) != NULL)
	{
	    BOOL WasColour = FALSE;
	    ResetAAADisplay();
	    SetField (CPR_NT_SHT);
	    GET_COPINSTRUCTION (Actual, Buffer);
	    do
		Actual = DisassCopperInstruction(Buffer, Actual, Comment, NoAddress, SkipColours, &WasColour, ShowOpcode, ShowConst);
	    while ((Buffer->ir1 != 0xffff) || (Buffer->ir2 != 0xfffe));

	/* and disassemble the WAIT 255 instruction */
	    DisassCopperInstruction(Buffer, Actual, Comment, NoAddress, SkipColours, &WasColour, ShowOpcode, ShowConst);
	    FreeMem (Buffer, sizeof(struct copinstruction));
	}
    }
} /* CopDis32 */

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
	GET_COPINS(Actual++, Instruction);
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
*	DisassICopperInstruction - Translates an intermediate copper instruction into english.
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
		GET_COPINS (Actual++, Instruction);
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
		if (RegisterInfo-> flags & ADDRESS)
		    Actual = FormatAddress (Instruction, Actual, CurrentVP, NoAddress, ShowOpcode);
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
	    GET_COPINS(Actual++, Instruction);
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


/****** ICopDis32 ************************************************************
*
*   NAME
*	ICopDis32 - Disassembles an intermediate copper list.
*
*   SYNOPSIS
*	ICopDis32(Walking, Comments, NoAddress, SkipColours, ShowOpcode, ShowConst)
*
*	VOID ICopDis32 (struct CopIns *, BOOL, BOOL, BOOL, BOOL, BOOL);
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
extern VOID ICopDis32 (struct CopIns * Walking, struct ViewPort *CurrentVP, BOOL Comments, BOOL NoAddress, BOOL SkipColours, BOOL ShowOpcode, BOOL ShowConst)

{
CopDisVPrintf ("This is not yet implemented\n");
return ();
	/* check and null pointers */
    if (Walking == NULL)
	CopDisVPrintf("No intermediate copper list\n");
    else
    {
	struct CopIns * Buffer;
	if ((Buffer = AllocMem (sizeof(struct CopIns), MEMF_CLEAR)) != NULL)
	{
	    BOOL WasColour = FALSE;
	    ResetAAADisplay();
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
