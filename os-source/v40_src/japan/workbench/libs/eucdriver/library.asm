**
**	$Id: library.asm,v 1.3 93/04/23 13:59:07 darren Exp $
**
**      EUC bitmap font driver
**
**      (C) Copyright 1992-1993 Commodore-Amiga, Inc.
**          All Rights Reserved
**


	SECTION		eucdriver

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/libraries.i"
	INCLUDE		"exec/resident.i"
	INCLUDE		"exec/devices.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/tasks.i"
	INCLUDE		"exec/io.i"
	INCLUDE		"exec/initializers.i"
	INCLUDE		"exec/execbase.i"
	INCLUDE		"exec/alerts.i"

	INCLUDE		"graphics/text.i"

	INCLUDE		"dos/dos.i"

	INCLUDE		"eucdriver_rev.i"
	INCLUDE		"driver.i"
	INCLUDE		"debug.i"

*------ Imported Globals ---------------------------------------------

	XREF	_EUCLoaderSeg
	XREF	_EUCLoaderName
	XREF	_EUCInstall
	XREF	_EUC_Endmodule
	XREF	_EUC_STARTSEGMENT
	XREF	_EUC_ENDSEGMENT

	XREF	EUC_PROCESS_SIZE
	XREF	_EUCName

*------ Exported Globals ---------------------------------------------

	XDEF	 DFName

*------ Imported Functions -------------------------------------------

	XREF	_EUCLowMemory

*------ Exported Functions -------------------------------------------

*------ Equates ------------------------------------------------------

	IFND	DSKF_MEMHANDLER_PRI
DSKF_MEMHANDLER_PRI	EQU	64
	ENDC


******* eucdriver.library/--background -- *****************************
*
* VECTORED FONTS
*
* Introduction -
*
*	The Japanese 3.xx release of the Amiga OS makes it possible
* for applications to print text in the Japanese (Nihongo) language.
* The first problem is that the Amiga font sub-system was designed
* to print ECMA Latin1 character sets ... infact this assumption that
* byte streams are ECMA Latin1 characters is deeply rooted in various
* O.S. functions, and in the C linker library functions (e.g., strcmp())
* which applications may use.
*
* 	So the very first problem to be solved is how to print some
* 6000+ Japanese glyphs using our existing Fonts, and Text() rendering
* model.  By good fortune, left to right printing direction is acceptable.
* This leaves the problem of how to associate more than 256 characters with
* a font.  Because of the large number of glyphs used in Japan, we often do
* not want to store all glyphs in memory all the time, but we would like to
* support multiple Japanese font families, sizes, and relevant styles.
* 
*	The solution in the Japanese 3.xx release of the O.S. is support
* for fonts which render themselves.  Another way to describe these fonts
* is to refer to them as "VECTORED FONTS."  Not to be confused with
* OUTLINE fonts, vectored fonts know how to render themselves by providing
* vectors which redirect text related graphics.library functions.
* In actual use this means that a vectored font could render itself from a
* set of bitmap fonts (as we are doing in the eucdriver.library), or even
* do on the fly rendering from outline data.
*
*	From an application point of view, vectored fonts are basically
* used like any Amiga bitmap font.  The application uses OpenFont(),
* of OpenDiskFont() to open a vectored font.  It uses SetFont()
* to use the font for rendering, and CloseFont() to free the font.
* Vectored fonts provide redirection vectors for all relevant
* graphics.library functions (e.g., Text(), TextFit(), CloseFont(), etc.)
*
*
* Vectored Fonts vs O.S. domain responsibilities -
*
*
*	When you break down the Amiga text sub-system, we have --
*
*	o Opening fonts
*
*	o Closing fonts
*
*	o Expunging fonts
*
*	o Provide access to a list of available fonts
*
*	o Text rendering
*
*	o Text fitting functions
*
*	o Bitmap scaled font support
*
*	o Outline font support
*
*	o Setting a font for use
*
* 	Opening fonts includes the process of selecting a font
* which best matches the application's request.  Requesting a font
* requires a TextAttr structure, or TTextAttr structure which may
* include additional tags.  The process of selection, and match
* criteria MUST remain within the realm of O.S. responsibilities,
* since match criteria may be improved in the future.  An example
* would be the addition of new font tags.  If it was possible for
* a vectored font to provide its own match checking, the font might
* require software modification everytime new match criteria is
* added to the OpenFont() or OpenDiskFont() procedure.
*
*	In V37 diskfont.library we added support for bitmap
* scaled fonts, and outline fonts.  In both cases it is the
* responsibility of diskfont.library to interpret the TextAttr,
* or TTextAttr structure, and return a font which best matches
* the requested criteria.  For the sake of future expansion, we will
* leave interpretation, and creation of scaled fonts in the
* O.S. domain.  This does not preclude a vectored font from providing
* its own scaled glyphs, it just means that the font driver is not
* required to provide its own code to interpret TextAttr structures,
* or provide its own scaling code which provides all the capabilities
* already in diskfont.library.
*
*	Responsibility for providing a list of available fonts
* will remain in the O.S. domain for the Japanese release of
* the O.S.  In theory vectored fonts could provide their own list
* of available fonts which could be merged with fonts diskfont.library
* already knows about.  The main problem with this is that the vector
* font must have been started, and fully installed before it can provide
* a list, however diskfont.library does not require a font be open to
* provide the availablility list.  This means that disk based data must
* still be provided to identify available sizes for a font, and we already
* have mechanisms which provide this information.  So for now, a vectored
* font must be stored on disk like any other bitmap font, or outline font.
*
*	A vectored font is responsible for providing its own rendering
* code, essentially replacing the built-in Text() function in
* graphics.library.  A vectored font may also need to provide its own
* text fitting functions such as the Japanese fonts.  The interface
* to these functions are well defined, though pixel based.  Outline
* fonts which measure placement more accurately than screen pixel
* resolution will need to translate internal measurements into pixel
* coordinates.
*
*	New functionality in diskfont.library provides a mechanism
* for immediately expunging a diskfont.  Vectored fonts may also
* provide a function to be called when a font is closed.  Finally
* under KS V39, applications may add themselves to the low-memory
* expunge handler chain.  Between these, vectored fonts can provide
* whatever is needed to expunge a vectored font.
*
*	Setting a font for use with the SetFont() function will
* remain an O.S. domain function.  Since this function essentially
* does nothing more than attach a font to a rastport, a vectored font
* should not need to alter this procedure.  
*
*
* Vectored fonts and the TextFont structure -
*
*
*	The current TextFont structure is unchanged, and is used
* as a handle for a vectored font.  What has changed is the
* TextFontExentsion structure created by the OS.  This structure
* is now longer, and may contain redirection vectors, and other
* data needed for a vectored font.  Since many applications look
* in the TextFont structure for font information, this practice
* is still mostly acceptable.  The TextFont structure looks like:
*
* struct TextFont {             
*
*----The following fields are unchanged for vectored fonts---
*
*     struct Message tf_Message;
*     UWORD   tf_YSize;    
*     UBYTE   tf_Style;    
*     UBYTE   tf_Flags;    
*     UWORD   tf_XSize;    
*     UWORD   tf_Baseline; 
*     UWORD   tf_BoldSmear;
*     UWORD   tf_Accessors;
*
*---The following fields may no longer be entirely correct---
*
*     UBYTE   tf_LoChar;   
*     UBYTE   tf_HiChar;   
*     APTR    tf_CharData; 
*
*     UWORD   tf_Modulo;
*     APTR    tf_CharLoc;
*     APTR    tf_CharSpace;
*     APTR    tf_CharKern;
* };
*
*	All TextFont structure fields for a vectored font must
* be provided, but it is not necessarily true that glyph data
* and related tables are entirely contained within this structure. 
* In general applications do not look at glyph data, or number of
* characters in TextFont structure.  To avoid crashing though, at
* least one dummy glyph must be provided in the TextFont structure.
* One additional point, even though tf_Style is essentially unchanged
* for now, we are looking at extending style's through use of
* tag lists, so the actual style of the font may not be entirely
* described by the tf_Style field.
*
*
* CODESETS
*
*	The second part of the problem is that the 8 bit Latin1
* codeset is inadequate for storing Japanese text.  Codesets such
* as the UNICODE 16 bit codeset make it possible to assign a unique
* code to most of the Japanese characters, but UNICODE is not what is
* being used in Japan.
*
*	UNICODE makes it possible to store codes for MANY languages
* in a single text file, or stream.  However UNICODE does not
* attempt to address the more complex problem of rendering glyphs
* for many languages, nor does it address language handling rules
* (e.g., word demarcation, mixed rendering direction, etc.).  This
* kind of problem is mostly outside of the scope of UNICODE beyond
* noting that text should be stored in the LOGICAL order (the
* order in which it would be TYPED), and that rendering should
* be handled in the expected way for each language.  A complete
* multi-language solution is outside of the scope of what can
* be handled using the Text() function in graphics.library.
*
*	The Japanese have at least 3 significant codesets
* (and derivations) of which we have selected the EUC codeset for
* use on the Amiga.  Other common codesets in Japan are JIS, and
* SHIFT-JIS.  Of these, EUC has a number of advantages.  It is
* mappable into JIS, or SHIFT-JIS.  It is more easily identified
* because Japanese characters are stored as two bytes, both of
* which have the high bit set.  It allows us to use all characters
* in the ASCII set ($00-$7F), and all C1 control codes (e.g., $9B 
* used by console.device).  It does not allow us to use any of the
* Latin1 characters ($A0-$FF), but these are not required for use in
* Japan, and infact, are not provided for by any of the Japanese
* codesets noted above.  This may appear to be a signficant problem,
* but remember, just as European users would never question the lack
* of Japanese ideographs, the Japanese do not question the lack of
* Latin1 characters.  When the Japanese computer industry decides that
* Latin1 characters are needed (if ever), they will probably be
* provided for using unassigned 16 bit codes.
*
*	EUC is a mixed single, and double byte codeset.  At first
* this appears to be more complicated than use of a pure double
* byte codeset such as UNICODE.  Hoewver in actual use this turns
* out not to be the case.  For some purposes a pure double byte
* codeset is simpler to use, but it is not true that something
* as fundamental as string comparison is simplifed by use of
* UNICODE.  For example it is not true that simply recompiling
* a string comparison function to compare UWORDs instead of
* UBYTEs is all that is needed.  The order of UNICODE codes
* is random; the order of EUC is essentially pre-sorted, except
* for proper comparison of single byte, and double byte pairing
* in the strings.
*
*
*
* CODESET IDENTIFICATION
*
*	A more general problem which is not addressed by
* UNICODE is codeset identification.  Most existing Amiga
* software, and even many OS functions, assume text streams
* are single byte text.  Text oriented applications often make
* significant assumptions about text.  Functions like strcmp()
* lack codeset identification arguments.  Worse, the UNICODE
* set tries to avoid duplication of glyphs, so sorting order
* and rendering rules may vary from language to language, even
* though they share the same codes.  An example would be sharing
* of codes for the ideographs used by the Chinese, and Japanese
* languages.
*
*	So the fundamental problems do not go away by using
* UNICODE, though it offers an opportunity to avoid some future
* problems when trying to support many languages at the same time.
* As noted above, multi-language support cannot be retrofitted
* into existing software without a considerable amount of rework,
* so the compromise for Japanese localization is use of the EUC
* codeset which is a standard in Japan.  For some applications,
* Japanese support will consist of little more than providing
* localized EUC text strings, and use of a Japanese font.  Text
* oriented applications like word-processors will need to be
* rewritten, which makes sense given that the needs of the
* Japanese user entering text are significantly different from
* those provided by existing Amiga word-processors.
*
*
* CODESET AND FONTS
*
*	The Japanese 3.xx release of the OS provides a new
* graphics.library function called GetFontCodeSet( struct TextFont *);
* This function returns the codeset identifier for a font, which
* for most existing fonts will be 0 (meaning that the font is
* meant to be used with an 8 bit codeset).  Japanese fonts will
* return a different value (see locale.h/i for definitions).
* In the case of the Operating System, Intuition string gadgets,
* and the console-handler (CON:) switch to Japanese entry mode
* when a Japanese font is selected.  They both use the
* GetFontCodeSet() function to make this determination.  The same
* function can be used by applications to switch to Japanese specific
* code when a Japanese font is selected by the user.
*
*
* IMPLEMENTATION OF THE EUC DRIVER
*
*	The EUC font driver is constructed as a standard Amiga
* shared library.  This driver is opened by diskfont.library
* automatically when a Japanese font is opened using OpenDiskFont().
* A small extension file is stored in Japanese bitmap font directories
* which tell diskfont.library which driver library is required for the font.
* This extension file, and details of initialization are described in
* the documentation for diskfont.library.
*
*	The EUC font driver stores glyph data as a set of standard
* Amiga bitmap fonts.  This is not a requirement, though it works
* out well for printing of Japanese text.  Each font file is
* reasonable size, so each file can be loaded, and expunged as needed
* when memory is low.  It is also possible to edit the files using
* a standard Amiga font editor if needed (requires assigning FONTS:
* to the directories in which these files are stored).  Use of
* many standard Amiga font files lets us use diskfont.library
* to create bitmap scaled fonts if needed.
*
*	The EUC font driver provides replacement functions for
* the Text() routine, and most of the text fitting functions in
* graphics.library.  These functions are automatically redirected
* to the EUC font routines, so you just call graphics.library
* functions in the normal way.
*
*
* MEMORY MANAGEMENT
*
*	Because of the large amount of RAM required for the
* Japanese fonts, the eucdriver.library loads font data from
* disk only when needed.  This is done by a separete PROCESS, so
* TASKS may continue to call graphics.library functions like Text().
* The EUC driver and new diskfont.library try to expunge least recently
* used font files first during low memory conditions.
*
*
*	***NOTE*** The use of many Amiga bitmap fonts is nearly ideal
* for loading/expunging ASCII, Hiragana, Katakana, double-wide ALPHA,
* double-wide NUMERIC, and the glyphs for the double-wide SYMBOLS.  This
* is because these glyphs all fit a few font files, and it actually
* makes sense that most of these glyphs are quickly loaded (because they
* are commonly used by the language).  However the loading/expunging of
* the Kanji glyphs is less optimal, so a future implementation may
* cache these on a per glyph basis to minimize memory usage.  Still,
* the pre-loading of ranges of glyphs (e.g., all glyphs whose EUC
* code starts with the same byte value) minimizes disk access at the
* expense of requiring more memory.
*
* JAPANESE FONTS
*
*	The Japanese 3.xx release of the OS ships with four
* Japanese font families called:
*
*	coral/16	- A gothic Japanese font
*	
*	pearl/24	- A mincho Japanese font
*
*	coral_e/18	- An enhanced version of Coral
*
*	pearl_e/26	- An enhanced version of Pearl
*
*
*	The 16x16 gothic font, and 24x24 mincho font are common
* sizes for Japanese bitmap fonts, however, many of the glyphs in
* these fonts do not have any white space, so the fonts may 'touch'
* when printed.
*
*	Many existing Amiga applications assume that fonts
* have whitespace in the font data, so it makes sense to provide
* a version of these fonts which have whitespace.  What does not
* make sense is storing a separate (and larger) copy of all the font
* data on disk, or in memory, so the EUC driver provides a special
* feature.  The trick here is the EUC driver can create Japanese fonts
* with whitespace from the original font data which lacks it.  So you will
* notice that the enhanced versions of Coral, and Pearl are two pixels
* taller than the un-ehanced versions (and two pixels wider).
*
*	Selection of font size will depend on your needs.  For example,
* the 16x16 size makes it easier for applications which need to re-layout
* requesters designed for use with Topaz 8.  All Japanese bitmap
* fonts are currently non-proportional, so layout for most applications
* means modifying height, and top edge of visual objects.  The minimum
* usable Workbench screen size for Japanese is 640x400, so the extra
* height of windows is expected.
*
*	The EUC driver is flexable enough that other sizes for
* Coral, and Pearl can be added later (even other Japanese font
* families with the addition of a small extension file described
* in the diskfont.library documentation).
*
*	Each Japanese font has a regular 8 bit character set font
* stored like any other Amiga bitmap font.  This is the font file
* which is used as a 'handle' for the vectoring graphics routines, and
* in the case of the EUC driver, contains the glyph data to use to render
* the ASCII characters $00-$7F.
*
*	The EUC codeset is a mixed single, and double byte codeset,
* so for the sake of simplicity, the single byte ASCII glyphs
* are always rendered half as wide as the double byte Japanese
* glyphs.  Therefore, for a 16x16 Japanese font, it doesn't
* matter that two bytes are two ASCII characters, or one Japanese
* character; the width of two bytes is 16 pixels.  For Coral_E/18,
* the same is true, except two bytes are always 18 pixels wide,
* and 18 pixels high.
*
*	Additional Japanese bitmap fonts can be added to the
* system later.  An example might be a 32x32 mincho font.  This
* would be added as a set of 32x32 fonts in the Pearl font
* directory.  A base font 32 high by 16 wide would needed to be
* created (usually done with a font editor) for the ASCII characters.
* To add an extended version of this font, a 34 high by 17 wide font
* would be added to the Pearl_E font directory.
*
*	To add a whole new family of fonts, a small loadable
* file would need to be created, but the process is essentially
* the same.  Specific documentation regarding font storage format
* for the eucdriver.library can be obtained from CATS.
*
*	***NOTE*** The use of multiple Amiga fonts is not a
*	requirement for creating a vectored font.  The driver
*	for a vectored font may use any data storage format
*	it wishes.  Once again, the EUC driver uses multiple
*	Amiga font files for convience.
*
**********************************************************************

	IFEQ	ROM_MODULE
                moveq   #-1,d0
                rts
	ENDC

residentEUC:
                dc.w    RTC_MATCHWORD
                dc.l    residentEUC
                dc.l    _EUC_Endmodule
                dc.b    RTF_AUTOINIT
                dc.b    VERSION
                dc.b    NT_LIBRARY
                dc.b    -120
                dc.l    _EUCName
                dc.l    identEUC
                dc.l    EUCInit

**********************************************************************
EUCInit:
		dc.l	EUCDriver_SIZEOF
		dc.l	EUCFuncInit
		dc.l	EUCStructInit
		dc.l	EUCinit

EUCinit:
		movem.l	a5/a6,-(a7)
		move.l	d0,a5

		move.l	a0,edvr_SegList(a5)
		move.l	a6,edvr_SysBase(a5)

	;-- Init open/close semaphore

		lea	edvr_OpenSemaphore(a5),a0
		JSRLIB	InitSemaphore

	;-- Init MustMem semaphore

		lea	edvr_MustMemSemaphore(a5),a0
		JSRLIB	InitSemaphore

	;-- Init vectored font list

		lea	edvr_EUCFontList(a5),a1
		NEWLIST	a1

	;-- Return library base

		move.l	a5,d0
		movem.l	(a7)+,a5/a6
		rts


*------ Open ---------------------------------------------------------
*
*   NAME
*	Open - a request to open this library
*
*   SYNOPSIS
*	Open(), libbase
*		a6
*
*   FUNCTION
*	The open routine grants access to the library.
*
*---------------------------------------------------------------------
Open:
	;-- Driver is FORBID state (task switching disallowed)

		addq.w	#1,LIB_OPENCNT(a6)	;lock out expunge

		movem.l	d2-d4/a4-a6,-(sp)
		move.l	a6,a5
		move.l	edvr_SysBase(a5),a6

		lea	edvr_OpenSemaphore(a5),a0
		JSRLIB	ObtainSemaphore

	PRINTF	DBG_ENTRY,<'EUC--OpenLibrary()'>

	;-- check for V39 exec.library now

		cmp.w	#39,LIB_VERSION(a6)
		bcs	open_fail_gfx

	;-- check for open completion inside of semaphore; if not
	;-- 1, we know that the library has already been opened, and
	;-- all initialization has been completed because any
	;-- previous opener has called ReleaseSemaphore().  It
	;-- may also be that the count was 2, but then was decremented
	;-- back to 1 again if the previous caller failed to get
	;-- all needed resources.  No problem, we just try again
	;-- on this caller's context.

		cmp.w	#1,LIB_OPENCNT(a5)
		bne	open_succeed

	PRINTF	DBG_FLOW,<'EUC--1st OpenLibrary()'>

	;-- allocate MustMemory for use by our Text() function

		move.l	#MAXMUSTMEM,d0
		move.l	#MEMF_CHIP,d1
		JSRLIB	AllocMem
		move.l	d0,edvr_MustMem(a5)
		beq	open_fail_mustmem

	;-- First time opened; open needed libraries, and
	;-- complete initialization (opening libraries may
	;-- break Forbid()


		lea	GLName(pc),a1
		moveq	#39,d0				;requires V39 or greater
		JSRLIB	OpenLibrary
		move.l	d0,edvr_GfxBase(a5)
		beq	open_fail_gfx

	PRINTF	DBG_FLOW,<'EUC--graphics.library @$%lx'>,D0

		lea	ULName(pc),a1
		moveq	#39,d0				;requires V39 or greater
		JSRLIB	OpenLibrary
		move.l	d0,edvr_UtilityBase(a5)
		beq	open_fail_util

	PRINTF	DBG_FLOW,<'EUC--utility.library @$%lx'>,D0

		lea	DLName(pc),a1
		moveq	#39,d0				;requires V39 or greater
		JSRLIB	OpenLibrary
		move.l	d0,edvr_DOSBase(a5)
		beq	open_fail_dos

	PRINTF	DBG_FLOW,<'EUC--dos.library @$%lx'>,D0

		lea	DFName(pc),a1
		moveq	#40,d0				;requires V40 or greater
		JSRLIB	OpenLibrary
		move.l	d0,edvr_DiskFontBase(a5)

	;; Success is no longer required, may retry at load font time
	;;	beq	open_fail_dfont

	PRINTF	DBG_FLOW,<'EUC--diskfont.library @$%lx'>,D0

	;-- Create DOS process for loading font files via diskfont.library

		moveq	#EUCLibPort_SIZEOF,d0
		move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
		JSRLIB	AllocVec
		move.l	d0,edvr_LibMsgPort(a5)
		beq	open_fail_mport

	PRINTF	DBG_FLOW,<'EUC--Library Msgport @$%lx'>,D0

		move.l	d0,a0

		move.l	#_EUCName,LN_NAME(a0)
		move.b	#NT_MSGPORT,LN_TYPE(a0)
		move.b	#PA_SIGNAL,MP_FLAGS(a0)
		move.b	#SIGB_SINGLE,MP_SIGBIT(a0)
		move.l	ThisTask(a6),MP_SIGTASK(a0)

		move.l	a5,elp_LibBase(a0)

		lea	MP_MSGLIST(a0),a1
		NEWLIST	a1

		move.l	a0,a1
		JSRLIB	AddPort

		moveq	#00,d0
		moveq	#SIGF_SINGLE,d1
		JSRLIB	SetSignal

	IFNE	ROM_MODULE

		lea	_EUC_STARTSEGMENT(pc),a4
		move.l	a4,d3
	ENDC

	IFEQ	ROM_MODULE

	; copy process code to high memory where it can stay out
	; of the way, and be unloaded without having to break Forbid()

		lea	proc_memlist(pc),a0
		JSRLIB	AllocEntry
		bclr.l	#31,d0
		bne	open_fail_mport

	PRINTF	DBG_FLOW,<'EUC--MemList for loader process at $%lx'>,D0

		move.l	d0,a4

		move.l	a4,edvr_MemList(a5)	;cache

		lea	_EUC_STARTSEGMENT(pc),a0
		lea	_EUC_ENDSEGMENT(pc),a1
		move.l	ML_ME+ME_ADDR(a4),d3
		move.l	d3,a4

	PRINTF	DBG_FLOW,<'EUC--Memory for loader process at $%lx'>,a4

open_copyproc:
		move.l	(a0)+,(a4)+
		cmpa.l	a1,a0
		blt.s	open_copyproc

		move.l	d0,a4			;restore
	ENDC


	; start process, and wait for reply

		move.l	#_EUCLoaderName,d1
		moveq	#00,d2
		move.l	#4096,d4
		addq.l	#4,d3			;past segment length
		lsr.l	#2,d3		;CPTR -> BPTR
		move.l	edvr_DOSBase(a5),a6

	PRINTF	DBG_OSCALL,<'EUC--CreateProc($%lx,$%lx,$%lx,$%lx)'>,D1,D2,D3,D4

		JSRLIB	CreateProc

		tst.l	d0
		BEQ_S	open_fail_process

		move.l	edvr_SysBase(a5),a6
		moveq	#SIGF_SINGLE,d0

	PRINTF	DBG_OSCALL,<'EUC--Wait($%lx)'>,D0

		JSRLIB	Wait

	PRINTF	DBG_FLOW,<'EUC--Parent signaled'>,D0

	;-- add low memory handler

		lea	edvr_LowMemHandler(a5),a1
		move.l	#_EUCName,LN_NAME(a1)
		move.b	#DSKF_MEMHANDLER_PRI,LN_PRI(a1)
		move.l	a5,IS_DATA(a1)
		move.l	#_EUCLowMemory,IS_CODE(a1)

	PRINTF	DBG_OSCALL,<'EUC--AddMemHandler($%lx)'>,A1

		JSRLIB	AddMemHandler

open_succeed:

		move.l	a5,d0				;return LIB base

open_failed:
		lea	edvr_OpenSemaphore(a5),a0
		JSRLIB	ReleaseSemaphore		;preserves D0
		movem.l	(sp)+,d2-d4/a4-a6

		rts

open_fail_process:

	IFEQ	ROM_MODULE

		move.l	a4,a0
		JSRLIB	FreeEntry

	ENDC

		move.l	edvr_LibMsgPort(a5),a1
		JSRLIB	RemPort

		move.l	edvr_LibMsgPort(a5),a1
		JSRLIB	FreeVec

open_fail_mport:

	; open of diskfont is not required so that a ROM
	; only system can be supported

		move.l	edvr_DiskFontBase(a5),d0
		beq.s	open_fail_dfont
		move.l	d0,a1
		JSRLIB	CloseLibrary
		clr.l	edvr_DiskFontBase(a5)

open_fail_dfont:
		move.l	edvr_DOSBase(a5),a1
		JSRLIB	CloseLibrary

open_fail_dos:
		move.l	edvr_UtilityBase(a5),a1
		JSRLIB	CloseLibrary

open_fail_util:

		move.l	edvr_GfxBase(a5),a1
		JSRLIB	CloseLibrary

open_fail_gfx:
		move.l	edvr_MustMem(a5),a1
		move.l	#MAXMUSTMEM,d0
		JSRLIB	FreeMem

open_fail_mustmem:

		moveq	#00,d0				; no library
		subq.w	#1,LIB_OPENCNT(a5)
		bra.s	open_failed


*------ Close --------------------------------------------------------
*
*   NAME
*	Close - terminate access to a library
*
*   SYNOPSIS
*	Close(), libbase
*		 a6
*
*   FUNCTION
*	The close routine notifies a library that it will no longer
*	be used.
*
*   NOTES
*	This vector is called under two conditions:
*
*	1.) Diskfont.library managed to open your library, but
*	    the call to InstallVectoredFont() failed.
*
*	2.) A font belong to your library has been closed, and
*	    has an open count of 0.
*
*---------------------------------------------------------------------
Close:
		movem.l	a5/a6,-(sp)
		move.l	a6,a5
		move.l	edvr_SysBase(a5),a6

		subq.w	#1,LIB_OPENCNT(a5)
		BNE_S	CloseRTS

	PRINTF	DBG_ENTRY,<'EUC--CloseLibrary()'>

	; signal loader process to quit ASAP - process will unload
	; itself - if code is in ROM, process code is safe.  Signal
	; does not break FORBID!  If process code is in RAM,
	; loader is still safe

		move.l	edvr_LoaderPort+MP_SIGTASK(a5),a1
		move.l	#SIGBREAKF_CTRL_C,d0

	PRINTF	DBG_OSCALL,<'EUC--Signal($%lx,$%lx)'>,A1,D0

		JSRLIB	Signal

		move.l	edvr_LibMsgPort(a5),a1
		JSRLIB	RemPort

		move.l	edvr_LibMsgPort(a5),a1
		JSRLIB	FreeVec

		lea	edvr_LowMemHandler(a5),a1
		JSRLIB	RemMemHandler

	; assume closing DiskFontBase does not break Forbid()
	; since all it does is freemem's, and close DOS/Utility/Gfx
	; as we are doing here - better not break Forbid(), or
	; diskfont.library also needs work!!

		move.l	edvr_DiskFontBase(a5),a1
		JSRLIB	CloseLibrary

		move.l	edvr_DOSBase(a5),a1
		JSRLIB	CloseLibrary

		move.l	edvr_UtilityBase(a5),a1
		JSRLIB	CloseLibrary

		move.l	edvr_GfxBase(a5),a1
		JSRLIB	CloseLibrary
		
		move.l	edvr_MustMem(a5),a1
		move.l	#MAXMUSTMEM,d0
		JSRLIB	FreeMem
CloseRTS:

		movem.l	(sp)+,a5-a6

Future1:
Future2:
Future3:
Future4:
ExtFunc:
		moveq	#00,d0
		rts

*------ Expunge ------------------------------------------------------
*
*   NAME
*	Expunge - indicate a desire to remove this library
*
*   SYNOPSIS
*	segment = Expunge(), libbase
*	d0		     a6
*
*   FUNCTION
*	The Expunge routine is called when a user issues a RemLibrary
*	call.  The existance of any other users of the library, as
*	determined by the library open count being non-zero, will
*	cause the Expunge to be deferred.  When the library is not
*	in use, or no longer in use, the Expunge is actually
*	performed, and the library removed from the system list.
*
*	This library keeps the library access count at zero so that it
*	can use the Expunge call to free unused fonts.
*
*---------------------------------------------------------------------

Expunge:
	PRINTF	DBG_FLOW,<'EUC--Expunge Vector'>

	;-- Library is not completely expungable since some of the
	;-- vectors currently need to hang around (still, we do free
	;-- up must resources during library close especially that
	;-- process which requires a 4K stack)

		tst.w	LIB_OPENCNT(a6)
		bne.s	deferExpunge

		movem.l	a2/a5-a6,-(sp)
		move.l	a6,a5
		move.l	edvr_SysBase(a5),a6

	;-- Remove library from list

		move.l	a5,a1
		REMOVE

	;-- deallocate library data

		move.l	edvr_SegList(a5),a2

		move.l	a5,a1
		move.w	LIB_NEGSIZE(a5),d0
		suba.w	d0,a1
		add.w	LIB_POSSIZE(a5),d0
		ext.l	d0
		JSRLIB FreeMem

	;-- Return SegList for ramlib/UnLoadSeg()

		move.l	a2,d0

	PRINTF	DBG_EXIT,<'EUC--$%lx=Expunge'>,D0

		movem.l	(sp)+,a2/a5-a6
		rts

deferExpunge:

		moveq	#0,d0		;seglist NULL

	PRINTF	DBG_EXIT,<'EUC--$%lx=Expunge'>,D0

		rts

*----------------------------------------------------------------------
*
* Definitions for Library Initialization
*
*----------------------------------------------------------------------
EUCFuncInit:
		dc.w	-1
		dc.w	Open-EUCFuncInit
		dc.w	Close-EUCFuncInit
		dc.w	Expunge-EUCFuncInit
		dc.w	ExtFunc-EUCFuncInit
		dc.w	_EUCInstall+(*-EUCFuncInit)
		dc.w	Future1-EUCFuncInit
		dc.w	Future2-EUCFuncInit
		dc.w	Future3-EUCFuncInit
		dc.w	Future4-EUCFuncInit
		dc.w	-1

EUCStructInit:
*	;------ Initialize the library
		INITBYTE    LN_TYPE,NT_LIBRARY
		INITLONG    LN_NAME,DFName
		INITBYTE    LIB_FLAGS,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD    LIB_VERSION,VERSION
		INITWORD    LIB_REVISION,REVISION

		dc.w	0
		ds.w	0

		
identEUC:
                VSTRING

version:
		VERSTAG

DFName:
		dc.b	'diskfont.library',0
GLName:
		dc.b	'graphics.library',0
DLName:
		dc.b	'dos.library',0
ULName:
		dc.b	'utility.library',0

		CNOP	0,2

* MemList structure for process memory
proc_memlist:
		ds.b	LN_SIZE			; reserve space for list node
		dc.w	1			; number of entries
		dc.l	(MEMF_REVERSE!MEMF_PUBLIC)
		dc.l	EUC_PROCESS_SIZE
	END
