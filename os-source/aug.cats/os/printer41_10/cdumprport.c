/****** printer.device/PRD_DUMPRPORT *************************************
*
*   NAME
*	PRD_DUMPRPORT -- dump the specified RastPort to a graphics printer
*
*   FUNCTION
*	Print a rendition of the supplied RastPort, using the supplied
*	ColorMap, position and scaling information, as specified in
*	the printer preferences.
*
*   IO REQUEST
*	io_Message	mn_ReplyPort set if quick I/O is not possible.
*	io_Command	PRD_DUMPRPORT.
*	io_Flags	IOB_QUICK set if quick I/O is possible.
*	io_RastPort	ptr to a RastPort.
*	io_ColorMap	ptr to a ColorMap.
*	io_Modes	the 'modes' flag from a ViewPort structure,
*			(the upper word is reserved and should be zero).
*
*			If you are running under version 36, or greater
*			of graphics.library, it is recommended that
*			you fill in "io_Modes" with the ULONG (32-bit)
*			value returned from calling:
*
*			ULONG ModeID = GetVPModeID(struct ViewPort *);
*
*			Doing so provides for upwards compatability with
*			the new display modes available under V36
*			(example: aspect ratio calculations for new
*			display modes).
*
*	io_SrcX		x offset into the RastPort to start printing from.
*	io_SrcY		y offset into the RastPort to start printing from.
*	io_SrcWidth	width of the RastPort to print (from io_SrcX).
*	io_SrcHeight	height of the RastPort to print (from io_SrcY).
*	io_DestCols	width of the printout in printer pixels.
*	io_DestRows	height of the printout in printer pixels.
*	io_Special	flag bits
*			(some of which pertain to DestCols and DestRows).
*			-if SPECIAL_MIL is set, then the associated
*			 parameter is specified in thousandths of
*			 an inch on the printer.  ie. if DestCols = 8000,
*			 DestRows = 10500 and SPECIAL_MILROWS and
*			 SPECIAL_MILCOLS is set then the printout would be
*			 8.000 x 10.500 inches.
*			-if SPECIAL_FULL is set, then the specific dimension
*			 is set to the maximum possible as determined
*			 by the printer limits or the configuration
*			 limits; whichever is less.
*			-if SPECIAL_FRAC is set, the parameter is
*			 taken to be a longword binary fraction
*			 of the maximum for that dimension.
*			-if all bits for a dimension are clear,
*			 (ie. SPECIAL MIL/FULL/FRAC and ASPECT are NOT set)
*			 then the parameter is specified in printer pixels.
*			-if SPECIAL_CENTER is set then the image will be
*			 put between the left and right edge of the paper.
*			-if SPECIAL_ASPECT is set, one of the dimensions
*			 may be reduced/expanded to preserve the aspect
*			 ratio of the print.
*			-SPECIAL_DENSITY(1-7) this allows for a maximum of 7
*			 different print densities.  DENSITY1 is the lowest
*			 density and the default.
*			-SPECIAL_NOFORMFEED - this allows for the mixing of
*			 text and graphics or multiple graphic dumps on page
*			 oriented printers (usually laser jet printers).
*			 When this flag is set the page will not be ejected
*			 after a graphic dump.  If you perform another
*			 graphic dump without this flag set OR close the
*			 printer after printing text after a graphic dump,
*			 the page will be ejected.
*			-if SPECIAL_TRUSTME is set then the printer specific
*			 driver is instructed to not issue a reset command
*			 before and after the dump.  If this flag is NOT
*			 checked by the printer specific driver then setting
*			 this flag has no effect.  Since we now recommend
*			 that printer driver writers no longer issue a reset
*			 command it is probably a safe idea to always set
*			 this flag when calling for a dump.
*			-if SPECIAL_NOPRINT is set then the following is done:
*			 Compute print size, set 'io_DestCols' and
*			 'io_DestRows' in the calling program's 'IODRPReq'
*			 structure and exit, DON'T PRINT.  This allows the
*			 calling program to see what the final print size
*			 would be in printer pixels.  Note that it modifies
*			 the 'io_DestCols' and 'io_DestRows' fields of your
*			 'IODRPReq' structure.  It also sets the print
*			 density and updates the 'MaxXDots', 'MaxYDots',
*			 'XDotsInch', and 'YDotsInch' fields of the
*			 'PrinterExtendedData' structure.
*
*	There following rules for the interpretation of io_DestRows and
*	io_DestCols that may produce unexpected results when they are
*	not greater than zero and io_Special is zero.  They have been
*	retained for compatability.  The user will not trigger these
*	other rules with well formed usage of io_Special.
*
*	When io_Special is equal to 0, the following rules
*	(from the V1.1 printer.device, and retained for compatibility
*	reasons) take effect.  Remember, these special rules are
*	for io_DestRows and io_DestCols and only take effect
*	if io_Special is 0).
*
*	a) DestCols>0 & DestRows>0 - use as absolute values.
*	   ie. DestCols=320 & DestRows=200 means that the picture
*	   will appear on the printer as 320x200 dots.
*	b) DestCols=0 & DestRows>0 - use the printers maximum number
*	   of columns and print DestRows lines. ie. if DestCols=0
*	   and DestRows=200 than the picture will appear on the
*	   printer as wide as it can be and 200 dots high.
*	c) DestCols=0 & DestRows=0 - same as above except the driver
*	   determines the proper number of lines to print based on
*	   the aspect ratio of the printer. ie. This results in the
*	   largest picture possible that is not distorted or inverted.
*	   Note: As of this writing, this is the call made by such
*	   program as DeluxePaint, GraphicCraft, and AegisImages.
*	d) DestCols>0 &DestRows=0 - use the specified width and the
*	   driver determines the proper number of lines to print based
*	   on the aspect ratio of the printer. ie. if you desire a
*	   picture that is 500 pixels wide and aspect ratio correct,
*	   use DestCols=500 and DestRows=0.
*	e) DestCols<0 or DestRows>0 - the final picture is either a
*	   reduction or expansion based on the fraction
*	   |DestCols| / DestRows in the proper aspect ratio.
*	   Some examples:
*	   1) if DestCols=-2 & DestRows=1 then the printed picture will
*	      be 2x the AMIGA picture and in the proper aspect ratio.
*	      (2x is derived from |-2| / 1 which gives 2.0)
*	   2) if DestCols=-1 & DestRows=2 then the printed picture will
*	      will be 1/2x the AMIGA picture in the proper aspect ratio.
*	      (1/2x is derived from |-1| / 2 which gives 0.5)
*
*   NOTES
*	The printer selected in preferences must have graphics
*	capability to use this command.  The error 'PDERR_NOTGRAPHICS'
* 	is returned if the printer can not print graphics.
*
*	Color printers may not be able to print black and white or
*	greyscale pictures -- specifically, the Okimate 20 cannot print
*	these with a color ribbon: you must use a black ribbon instead.
*	If the printer has an input buffer option, use it.
*	If the printer can be uni or bi directional, select
*	uni-directional; this produces a much cleaner picture.
*	Most printer drivers will attempt to set unidirectional
*	printing if it is possible under software control.
*
*	Please note that the width and height of the printable area on
*	the printer is in terms of pixels and bounded by the following:
*	 a) WIDTH = (RIGHT_MARGIN - LEFT_MARGIN + 1) / CHARACTERS_PER_INCH
*	 b) HEIGHT = LENGTH / LINES_PER_INCH
*       Margins are set by preferences.
*
*	For BGR printer support, the YMC values in the printer
*	specific render.c functions equate to BGR respectively, ie.
*	yellow is blue, magenta is green, and cyan is red.
*
*	For version 2.1 of the Operating System (the Localization
*	release), some of the printer drivers have been modified to
*	support European A size paper (e.g., "A4").  See preferences.h
*	for a list of defined sizes.  For most printers, this means
*	the maximum X dots will be calculated based on millimeters
*	minus 1/2 inch (approx 13 mm) - this is consistent with existing
*	driver calculations (e.g., 8.0 inches wide for US_LETTER, and
*	US_LEGAL sizes).
*
*	Some printers, like the HP_LaserJet, and HP_DeskJet also
*	calculate maximum Y dots.
*
*	For all modified printer drivers, selecting paper sizes larger
*	than the printer can handle may result in unexpected, or
*	clipped results.  This allows for the possibility of using
*	these drivers with future printers which may physically
*	[and internally] support these larger sizes of paper.
*
*	It is assumed that the user will make reasonable choices when
*	selecting paper size (e.g., not select WIDE TRACTOR for a
*	NARROW TRACTOR printer).
*
* Data Structures
* ---------------
*
*	The printer specific and non-specific data structures can be read
*	ONCE you have opened the printer device.  Here is a code fragment
*	to illustrate how to do just that.
*
*	#include <exec/types.h>
*	#include <devices/printer.h>
*	#include <devices/prtbase.h>
*	#include <devices/prtgfx.h>
*
*	struct IODRPReq PReq;
*	struct PrinterData *PD;
*	struct PrinterExtendedData *PED;
*
*   open the printer device / if it opened...
*   if (OpenDevice("printer.device", 0, &PReq, 0) == NULL) {
*       get pointer to printer data
*       PD = (struct PrinterData *)PReq.io_Device;
*       get pointer to printer extended data
*       PED = &PD->pd_SegmentData->ps_PED;
*       let's see what's there
*       printf("PrinterName = '%s', Version=%u, Revision=%u\n",
*           PED->ped_PrinterName, PD->pd_SegmentData->ps_Version,
*           PD->pd_SegmentData->ps_Revision,);
*       printf("PrinterClass=%u, ColorClass=%u\n",
*           PED->ped_PrinterClass, PED->ped_ColorClass);
*       printf("MaxColumns=%u, NumCharSets=%u, NumRows=%u\n",
*           PED->ped_MaxColumns, PED->ped_NumCharSets, PED->ped_NumRows);
*       printf("MaxXDots=%lu, MaxYDots=%lu, XDotsInch=%u, YDotsInch=%u\n",
*           PED->ped_MaxXDots, PED->ped_MaxYDots,
*           PED->ped_XDotsInch, PED->ped_YDotsInch);
*       CloseDevice(&PReq);
*   }
*
* Preferences
* -----------
*
*    If you want the user to be able to access the printer preferences items
* without having to run preferences (like DPAINT II's printer requester),
* here is what you do.  You can look at the printer's copy of preferences
* by referring to 'PD->pd_Preferences' (the printer device MUST already be
* opened at this point).  After you have this you could put up a requester
* and allow the user to change whatever parameters they wanted.
* BEAR IN MIND THAT YOU ARE RESPONSIBLE FOR RANGE CHECKING THESE SELECTIONS!
* Listed below are the printer preferences items and their valid values.
*
* PrintPitch         - PICA, ELITE, FINE.
* PrintQuality       - DRAFT, LETTER.
* PrintSpacing       - SIX_LPI, EIGHT_LPI.
* PrintLeftMargin    - 1 to PrintRightMargin.
* PrintRightMargin   - PrintLeftMargin to 999.
* PaperLength        - 1 to 999.
* PrintImage         - IMAGE_POSITIVE, IMAGE_NEGATIVE.
* PrintAspect        - ASPECT_HORIZ, ASPECT_VERT.
* PrintShade         - SHADE_BW, SHADE_GREYSCALE, SHADE_COLOR.
* PrintThreshold     - 1 to 15.
* PrintFlags         - CORRECT_RED, CORRECT_GREEN, CORRECT_BLUE, CENTER_IMAGE,
*                      IGNORE_DIMENSIONS, BOUNDED_DIMENSIONS,
*                      ABSOLUTE_DIMENSIONS, PIXEL_DIMENSIONS,
*                      MULTIPLY_DIMENSIONS, INTEGER_SCALING,
*                      ORDERED_DITHERING, HALFTONE_DITHERING.
*                      FLOYD_DITHERING, ANTI_ALIAS, GREY_SCALE2
* PrintMaxWidth      - 0 to 65535.
* PrintMaxHeight     - 0 to 65535.
* PrintDensity       - 1 to 7.
* PrintXOffset       - 0 to 255.
*
* Asynchronous I/O
* ----------------
*
*     The recommended way to do asynchronous i/o is...
*
* a) To send requests for i/o.
*
*     struct IORequest *ioreq;
*     struct MsgPort *port;
*     UBYTE signal;
*
*     port = ioreq->io_Message.mn_ReplyPort;
*     signal = port->mp_SigBit;
*
*     SendIO(ioreq);  send request
*     Wait(signal);  wait for completion (go to sleep)
*     while ((Msg = GetMsg(port)) != NULL) {  get ALL messages
*     }
*
* b) To abort a previous request for i/o.
*
*     struct IORequest *ioreq;
*
*     AbortIO(ioreq);  abort request
*     WaitIO(ioreq);  wait for reply
*
*     at this point you can re-use 'ioreq'.
*
*     Note that in the above examples 'ioreq' could be any one of...
*     a) struct IOStdReq    a standard i/o request
*     b) struct IODRPReq    a dumprport i/o request
*     c) struct IOPrtCmdReq a printer command i/o request
*
*     It is recommend that you do asynchronous i/o in your programs
*     and give the user a way of aborting all requests.
*
*
* 		V1.3 Printer Driver Notes
* 		-------------------------
*
* 	In general densities which use more than one pass should only be
* used for B&W shade dumps.  They can be used for Grey-Scale or Color Shade
* dumps BUT the output may tend to look muddy or dark.  Also multiple pass
* Color dumps tend to dirty or smear the ribbon (ie. yellow will get
* contaminated with the other colors on the ribbon; you have been warned).
*
*
* Alphacom_AlphaPro_101
* ---------------------
* 1. Daisywheel printer (text only).
*
* Brother_HR-15XL
* ---------------
* 1. Daisywheel printer (text only).
*
* CalComp_ColorMaster
* -------------------
* 1. Thermal transfer b&w/color printer (text and graphics).
* 2. Use Black ribbon for non-color dumps; Color ribbon for color dumps.
* 3. Linefeeds # of vertical dots printed.
* 4. Densities supported are 203x200(1) dpi.
* 5. This is a dual printer driver.  Select a PaperSize of 'Narrow Tractor'
*    for use with the ColorMaster; 'Wide Tractor' for use with the
*    ColorView-5912 (which uses 11 x 17 inch paper).
*
* CalComp_ColorMaster2
* -------------------
* 1. Thermal transfer b&w/color printer (text and graphics).
* 2. Use Black ribbon for non-color dumps; Color ribbon for color dumps.
* 3. Linefeeds # of vertical dots printed.
* 4. Densities supported are 203x200(1) dpi.
* 5. This is a dual printer driver.  Select a PaperSize of 'Narrow Tractor'
*    for use with the ColorMaster; 'Wide Tractor' for use with the
*    ColorView-5912 (which uses 11 x 17 inch paper).
* 6. This driver is the same as the Calcomp_ColorMaster driver EXCEPT it is
*    approximately 2 times faster (during color dumps) and requires LOTS of
*    memory (up to 1,272,003 bytes for a full 8 x 10 inch (1600 x 2000 dot)
*    color dump).  Typically full-size (color) dumps are 1600 x 1149 dots and
*    require 730,767 bytes.  Memory requirements for the ColorView-5912
*    are up to 2,572,803 bytes for a full 10 x 16 inch (2048 x 3200 dot)
*    color dump.  Typically full-size (color) dumps are 2048 x 2155 dots and
*    require 1,732,623 bytes.  The memory requirements are 1/3 when doing a
*    non-color printout (on both the ColorMaster and ColorView).
*
* Canon_PJ-1080A
* --------------
* 1. Ink jet b&w/color printer (text and graphics).
* 2. Linefeeds # of vertical dots printed.
* 3. Densities supported are 83x84(1) dpi.
*
* CBM_MPS1000
* -----------
* 1. Dot matrix b&w printer (text and graphics).
* 2. Linefeeds # of vertical dots printed (-1/3 dot if PaperType = Single). *2
* 3. Density	XDPI	YDPI	XYDPI	Comments
* 	1	120	 72	 8640
* 	2	120	144	17280	two pass
* 	3	240	 72	17280			*1
* 	4	120	216	25920	three pass
* 	5	240	144	34560	two pass	*1
* 	6	240	216	51840	three pass	*1
* 	7	same as 6
* 4. Print width for US_LETTER size paper is 8.0 inches.
* 5. As of version 35.48, this driver calculates maximum X dots for
*    European A size paper as defined in preferences.h.  Some of
*    these sizes are too large for this printer.
*
* Diablo_630
* ----------
* 1. Daisywheel printer (text only).
*
* Diablo_Advantage_D25
* --------------------
* 1. Daisywheel printer (text only).
*
* Diablo_C-150
* ------------
* 1. Ink jet b&w/color printer (text and graphics).
* 2. Always linefeeds 4 dots (limitation of printer).
* 3. A PaperSize of 'Wide Tractor' selects a maximum print width of
*    8.5 inches (for wide roll paper).
* 5. Densities supported are 120x120(1) dpi.
*
* EpsonQ (24-pin Epson compatible)
* ------
* 1. Dot matrix b&w/color printer (text and graphics).
* 2. Drives all EpsonQ (LQ1500, LQ2500, etc.) compatible printers.
* 3. Linefeeds # of vertical dots printed.
* 4. Density	XDPI	YDPI	XYDPI	Comments
* 	1	 90	180	16200
* 	2	120	180	21600
* 	3	180	180	32400
* 	4	360	180	64800	*1
* 	5,6,7	same as 4
* 5. A PaperSize of 'Wide Tractor' selects a maximum print width of
*    13.6 inches (for wide carriage printers).
* 6. A PaperType of 'Single' uses only 16 of the 24 pins, whereas a PaperType
*    of 'Fanfold' uses all 24 pins.  The 'Single' option is useful for those
*    printers which have a weak power supply and cannot drive all 24 pins
*    continuously.  If during a single pass of the print head you notice that
*    the top two thirds of the graphics are darker than the bottom one third
*    then you will probably need to drop down to 16 pins.
* 7. As of version 35.71, this driver calculates maximum X dots for
*    European A size paper as defined in preferences.h.  The calculation
*    is based on millimeters minus 1/2 inch (approx 13mm).  Maximum
*    paper width is 13.6 inches, so some European A sizes are too large
*    for this printer.
*
* EpsonX[CBM_MPS-1250] (8/9-pin Epson compatible)
* --------------------
* 1. Dot matrix b&w/color printer (text and graphics).
* 2. Drives all EpsonX (EX/FX/JX/LX/MX/RX, etc.) compatible printers.
* 3. Linefeeds # of vertical dots printed (-1/3 dot if PaperType = Single). *2
* 4. Density	XDPI	YDPI	XYDPI	Comments
* 	1	120	 72	 8640
* 	2	120	144	17280	two pass
* 	3	240	 72	17280			*1
* 	4	120	216	25920	three pass
* 	5	240	144	34560	two pass	*1
* 	6	240	216	51840	three pass	*1
* 	7	same as 6
*
* 5. A PaperSize of 'Wide Tractor' selects a maximum print width of
*    13.6 inches (for wide carriage printers).
* 6. Use this driver if you own a CBM_MPS-1250 (as it is EpsonX compatible).
* 7. As of version 35.42, this driver calculates maximum X dots for
*    European A size paper as defined in preferences.h.  The calculation
*    is based on millimeters minus 1/2 inch (approx 13mm).  Maximum
*    paper width is 13.6 inches, so some European A sizes are too large
*    for this printer.
*
* EpsonXOld (8/9-pin Epson compatible)
* ---------
* 1. Dot matrix b&w printer (text and graphics).
* 2. Drives all very old EpsonX (EX/FX/JX/LX/MX/RX, etc.) compatible printers.
* 3. Linefeeds # of vertical dots printed.
* 4. Density	XDPI	YDPI	XYDPI	Comments
* 	1	 60	72	 4320
* 	2	120	72	 8640	(double speed)			*1
* 	3	120	72	 8640
* 	4	240	72	17280					*1
* 	5	120	72	 8640	(for use on old Star printers)
* 	6	240	72	17280	(for use on old Star printers)	*1
* 	7	240	72	17280	(same as density 4)		*1
* 5. A PaperSize of 'Wide Tractor' selects a maximum print width of
*    13.6 inches (for wide carriage printers).
* 6. Use this driver if the EpsonX driver does not work properly in graphics
*    or text mode on your EpsonX compatible printer.
*
* generic
* -------
* 1. Text only printer.
*
* Howtek_Pixelmaster
* ------------------
* 1. Plastic ink jet b&w/color printer (text and graphics).
* 2. Linefeeds # of vertical dots printed.
* 3. Density	XDPI	YDPI	XYDPI	Comments
* 	1	 80	 80	 6400
* 	2	120	120	14400
* 	3	160	160	25600
* 	4	240	240	57600
* 	5,6,7	same as 4
* 4. Maximum print area is 8.0 x 10.0 inches.
*
* HP_DeskJet
* ----------
* 1. Ink jet non-color printer (text and graphics).
* 2. Linefeeds # of vertical dots printed.
* 3. Density	XDPI	YDPI	XYDPI	Comments
* 	1	 75	 75	 5625
* 	2	100	100	10000
* 	3	150	150	22500
* 	4	300	300	90000
* 	5,6,7	same as 4
* 4. Maximum print area is 8.0 x 10.0 inches US_LEGAL, and 8.0 x
*    13.0 inches US_LETTER.
* 5. As of version 35.29, the driver will calculate maximum area
*    size for European A size paper as defined in preferences.h.
*    The driver calculates the maximum X, and Y dots based on
*    millimeters, minus 1/2 inch from the width, and minus 1"
*    from the height (approx 13mm, and 26mm respectively).
*    Therefore the margin area is consistent with US paper sizes.
* 6. Some European A sizes are too large for this printer.
*
* HP_LaserJet (LaserJet+/LaserJetII compatible)
* -----------
* 1. Laser engine non-color printer (text and graphics).
* 2. Linefeeds # of vertical dots printed.
* 3. Density	XDPI	YDPI	XYDPI	Comments
* 	1	 75	 75	 5625
* 	2	100	100	10000
* 	3	150	150	22500
* 	4	300	300	90000
* 	5,6,7	same as 4
* 4. Maximum print area is 8.0 x 10.0 inches US_LEGAL, and 8.0 x
*    13.0 inches US_LETTER.
* 5. As of version 35.59, the driver will calculate maximum area
*    size for European A size paper as defined in preferences.h.
*    The driver calculates the maximum X, and Y dots based on
*    millimeters, minus 1/2 inch from the width, and minus 1"
*    from the height (approx 13 mm, and 26 mm respectively).
*    Therefore the margin area is consistent with US paper sizes.
* 6. Some European A sizes are too large for this printer.
*
* HP_PaintJet
* -----------
* 1. Ink jet b&w/color printer (text and graphics).
* 2. Linefeeds # of vertical dots printed.
* 3. Densities supported are 180x180(1) dpi.
*
* HP_ThinkJet
* ----------
* 1. Ink jet non-color printer (text and graphics).
* 2. Linefeeds # of vertical dots printed.
* 3. Density	XDPI	YDPI	XYDPI	Comments
* 	1	 96	96	 9216
* 	2	192	96	18432
* 	3,4,5,6,7	same as 4
* 4. This printer prints 640 dots X in 96 DPI mode, and 120 dots
*    X in 192 DPI mode.  Other sizes are not supported by the
*    printer.
*
* Imagewriter II (Imagewriter compatible)
* --------------
* 1. Dot matrix b&w/color printer (text and graphics).
* 2. Linefeeds # of vertical dots printed.
* 3. Density	XDPI	YDPI	XYDPI	Comments
* 	1	 80	 72	 5760
* 	2	120	 72	 8640
* 	3	144	 72	10368
* 	4	160	 72	11520
* 	5	120	144	17280	two pass
* 	6	144	144	20736	two pass
* 	7	160	144	23040	two pass
*
* Nec_Pinwriter (24-wire Pinwriter compatible (P5/P6/P7/P9/P2200))
* -------------
* 1. Dot matrix b&w/color printer (text and graphics).
* 2. Drives all Nec 24-wire Pinwriter compatible printers.
* 3. Linefeeds # of vertical dots printed.
* 4. Density	XDPI	YDPI	XYDPI	Comments
* 	1	 90	180	 16200
* 	2	120	180	 21600
* 	3	180	180	 32400
* 	4	120	360	 43200	two pass
* 	5	180	360	 64800	two pass
* 	6	360	180	 64800
* 	7	360	360	129600	two pass
* 5. A PaperSize of 'Wide Tractor' selects a maximum print width of
*    13.6 inches (for wide carriage printers).
* 6. As of version 35.17, this driver calculates maximum X dots for
*    European A size paper as defined in preferences.h.  The calculation
*    is based on millimeters minus 1/2 inch (approx 13mm).  Maximum
*    paper width is 13.6 inches, so some European A sizes are too large
*    for this printer.
*
* Okidata_92
* ----------
* 1. Dot matrix non-color printer (text and graphics).
* 2. Always linefeeds 7/72 inch (limitation of printer in graphics mode).
* 3. Densities supported are 72x72 dpi.
*
* Okidata_293I
* ------------
* 1. Dot matrix b&w/color printer (text and graphics).
* 2. Drives 292 or 293 using the IBM interface module.
* 3. Linefeeds # of vertical dots printed (-1/2 dot if PaperType = Single) *3
* 4. Density	XDPI	YDPI	XYDPI	Comments
* 	1	120	144	17280
* 	2	240	144	34560
* 	3	120	288	34560	two pass
* 	4	240	288	69120	two pass
* 	5,6,7	same as 4
* 5. A PaperSize of 'Wide Tractor' selects a maximum print width of
*    13.6 inches (for wide carriage printers).
*
* Okimate-20
* ----------
* 1. Thermal transfer b&w/color printer (text and graphics).
* 2. Use Black ribbon for non-color dumps; Color ribbon for color dumps.
* 3. Linefeeds an even # of dots printed. (ie. if 3 printed, 4 advanced).
* 4. Densities supported are 120x144(1) dpi.
*
* Quadram_QuadJet
* ---------------
* 1. Ink jet b&w/color printer (text and graphics).
* 2. Linefeeds # of vertical dots printed.
* 3. Densities supported are 83x84(1) dpi.
*
* Qume_LetterPro_20
* -----------------
* 1. Daisywheel printer (text only).
*
* Seiko_5300
* ----------
* 1. Thermal transfer b&w/color printer (graphics only).
* 2. Use Black ribbon for non-color dumps; Color ribbon for color dumps.
* 3. Density	XDPI	YDPI	XYDPI	Comments
* 	1	152	152	23104	drives CH-5301 printer
* 	2	203	203	41209	drives CH-5312 printer
* 	3	240	240	57600	drives CH-5303 printer
* 	4, 5,6,7	same as 3
* 	You must select the proper density to drive the specific printer
* 	that you have.
* 4. This driver is not on the V1.3 Workbench or Extras disk.  It is
*    available on BIX and directly from Seiko.
*
* Seiko_5300a
* -----------
* 1. Thermal transfer b&w/color printer (graphics only).
* 2. Use Black ribbon for non-color dumps; Color ribbon for color dumps.
* 3. Density	XDPI	YDPI	XYDPI	Comments
* 	1	152	152	23104	drives CH-5301 printer
* 	2	203	203	41209	drives CH-5312 printer
* 	3	240	240	57600	drives CH-5303 printer
* 	4, 5,6,7	same as 3
* 	You must select the proper density to drive the specific printer
* 	that you have.
* 4. This driver is the same as the Seiko_5300 driver EXCEPT it is
*    approximately 2 times faster (during color dumps) and requires LOTS of
*    memory (up to 1,564,569 bytes for a full 8 x 10 inch (1927 x 2173 dot)
*    color dump).  Typically full-size (color) dumps are 1927 x 1248 dots
*    and require 898,569 bytes.  The memory requirements are 1/3 when doing
*    a non-color printout.
* 5. This driver is not on the V1.3 Workbench or Extras disk.  It is
*    available on BIX and directly from Seiko.
*
* Tektronix_4693D
* ---------------
* 1. Thermal transfer b&w/color printer (graphics only).
* 2. Densities supported are 300x300(1) dpi
* 3. Due to the way the printer images a picture none of the printer
*    preferences options affect the printout with the following exceptions:
*    a)Aspect - Horizontal, Vertical
*    b)Shade - B&W, Grey_Scale, Color
*    ...as a result of this only full size pictures can be printed.
* 4. Keypad menu option 3b COLOR ADJUSTMENT may be set from the keypad.
*    For normal prints this option should be set to "do not adjust".
* 5. Keypad menu option 3d VIDEO COLOR CORRECTION may be set from the keypad.
*    For normal prints this option should be set to "do not adjust".
* 6. Keypad menu option 5 BACKGROUND COLOR EXCHANGE may be set from the
*    keypad.  For normal prints this option should be set to "print colors
*    as received".
* 7. Once a picture has been printed additional copies may be printed
*    without resending by using the printers keypad.
* 8. This driver is not on the V1.3 Workbench or Extras disk.  It is
*    available on BIX and directly from Tektronix.
*
* Tektronix_4696
* --------------
* 1. Ink jet b&w/color printer (text and graphics).
* 2. Always linefeeds 4 dots (limitation of printer).
* 3. Densities supported are 121x120(1), 242x120(black)(2) and
*    242x120(color)(3).
*    Selecting a density of 2 or higher really doesn't give you true 242 dpi
*    resolution since the printer only has 121 x dots per inch.
*    Instead this mode tells the printer to go into it's double pass mode.
*    Here, it outputs a line of dots at 121 dpi; and outputs the line again
*    (shifted to the right by 1/242 of an inch).  This produces much more
*    vibrate colors and gives the illusion of more resolution.  One drawback
*    is that large areas of solid colors (red, green, and blue specifically)
*    tend to over-saturate the paper with ink.  Density1 outputs all colors
*    in one pass.  Density 2 does a double pass on black.  Density 3 does a
*    double pass on all colors.  Density 1 to 3 correspond to the printer's
*    graphics printing modes 1 to 3 (respectively).
* 4. This driver is not on the V1.3 Workbench or Extras disk.  It is
*    available on BIX and directly from Tektronix.
* 5. A PaperSize of 'Wide Tractor' selects a maximum print width of
*    9.0 inches (for wide roll paper).
*
* Toshiba_P351C (24-pin Toshiba compatible)
* -------------
* 1. Dot matrix b&w/color printer (text and graphics).
* 2. Drives all Toshiba_P351C compatible printers.
* 3. Linefeeds # of vertical dots printed.
* 4. Density	XDPI	YDPI	XYDPI	Comments
* 	1	180	180	32400
* 	2	360	180	64800
* 	3,4,5,6,7	same as 2
* 5. A PaperSize of 'Wide Tractor' selects a maximum print width of
*    13.5 inches (for wide carriage printers).
*
* Toshiba_P351SX (24-pin Toshiba compatible)
* --------------
* 1. Dot matrix b&w/color printer (text and graphics).
* 2. Drives all Toshiba_P351SX (321SL, 321SLC, 341SL) compatible printers.
* 3. Linefeeds # of vertical dots printed.
* 4. Density	XDPI	YDPI	XYDPI	Comments
* 	1	180	180	 32400
* 	2	360	180	 64800
* 	3	180	360	 64800	two pass
* 	4	360	360	129600	two pass
* 	5,6,7	same as 4
* 5. A PaperSize of 'Wide Tractor' selects a maximum print width of
*    13.5 inches (for wide carriage printers).
*
* Xerox_4020
* ----------
* 1. Ink jet b&w/color printer (text and graphics).
* 2. Always linefeeds 4 dots (limitation of printer).
* 3. This driver is IDENTICAL to the Diablo_C-150 driver EXCEPT it outputs
*    all black dots TWICE.  This is a special feature of this printer and
*    produces much more solid, darker black shades.  Please note that some
*    printing time overhead results from this feature; if you don't want it
*    use the Diablo_C-150 driver.
* 4. Densities supported are 121x120(1) and 242x240(2) dpi.
*    Selecting a density of 2 or higher really doesn't give you true 240 dpi
*    resolution since the Xerox_4020 only has 121 x dots per inch.
*    Instead this mode tells the printer to go into it's pseudo 240 dpi mode.
*    Here, it outputs a line of dots at 121 dpi; moves the paper up 1/240 of
*    an inch and outputs the line again (shifted to the right by 1/240 of an
*    inch).  This produces much more vibrate colors and gives the illusion
*    of more resolution.  One drawback is that large areas of solid colors
*    (red, green, and blue specifically) tend to over-saturate the paper with
*    ink.
* 5. A PaperSize of 'Wide Tractor' selects a maximum print width of
*    9.0 inches (for wide roll paper).
*
*
* Notes
* -----
*
* *0 - on most printers friction fed paper tends to produce better looking
*      (ie. less horizontal banding) graphic dumps than tractor fed paper.
*
* *1 - in this mode the printer cannot print two consecutive dots in a row.
*      It is recommended that you only use this density for B&W Shade dumps.
*
* *2 - only when 72 YDPI is selected.  This option is useful if you notice
*      tiny white horizontal strips in your printout.
*
* *3 - only when 144 YDPI is selected.  This option is useful if you notice
*      tiny white horizontal strips in your printout.
*
*********************************************************************/

#include <exec/memory.h>
#include <exec/errors.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <graphics/displayinfo.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "printer.h"
#include "prtbase.h"
#include "prtgfx.h"

#include "internal.h"

#include "printer_iprotos.h"

#define DEBUG0		0	/* debugging flag: 0-off, 1-on */
#define DEBUG2		0	/* more debugging (list of colors, etc.) */
#define ALLOCDEBUG	0	/* debugging of allocated memory */
#define TDEBUG		0	/* time debugging */
#define SDEBUG		0	/* display symbols */
#define ADEBUG		0	/* aspect ration debugging */

PCDumpRPort(ior)
struct IODRPReq *ior; /* ior is a pointer to the input request */
{
	extern void *AllocMem();
	extern int Scale();
	extern ULONG TypeOfMem();
	extern DisplayInfoHandle FindDisplayInfo();

	extern struct GfxBase *GfxBase;
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	/* misc vars */
	struct Layer *layer;
	struct ClipRect *cliprect;
	UWORD temp1, temp2, v39, colorfunc, loop, depth;
	LONG ltemp;
	ULONG ulong1, ulong2, ulong3, ulong4, ulongpc, ulongpr, colors32[3];
	UBYTE *ptr;
	ULONG PrefsMaxWidth, PrefsMaxHeight, PrefsXOffset;
	UWORD *buf1, xmult, xmod;
	WORD etx;

	/* used for V36 graphics modes - new way to calculate
	   aspect ratio
	*/

	struct	DisplayInfo dpinfo;	/* to be filled in by graphics.lib V36. */
	ULONG	modeID;			/* set equal to io_Modes */
	DisplayInfoHandle mode_handle;
	UWORD	DefaultDPMY,DefaultDPMX;

	/* flags */
	int Aspect, HiRes, Lace, YReduce, XReduce, Center, Dimension;

	int MaxXDots, MaxYDots;
	int	maxX, maxY;
	ULONG aspectX, aspectY, XDotsInch, YDotsInch;
	int err, (*render)();
	ULONG special, mask;

	struct RastPort *rp;
	struct PrtInfo PInfo;
	union colorEntry *cm;
	UWORD xstart, ystart, width, height;
	UWORD maxcols, maxrows;
	LONG pc, pr;

	static UBYTE dmatrix[] = { /* ordered, halftone */
		/* ordered dither matrix */
		0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 14, 7, 13, 5, /* */
#if 1
		/* halftone dither matrix (rotated 45 degrees, looks better) */
		5, 12, 14, 3, 8, 0, 6, 10, 13, 2, 4, 14, 7, 11, 9, 1 /* */
#else
		/* halftone dither matrix (traditional orientation) */
		12, 4, 11, 14, 8, 0, 3, 7, 6, 2, 1, 9, 14, 10, 5, 13 /* */
#endif
	};

/* scale up to 8 bit */
#define SU(x) ((x<<4)|x)

	static UBYTE dmatrix24[] = { /* ordered, halftone */
		/* ordered dither matrix */
		0, SU(8), SU(2), SU(10), SU(12), SU(4), SU(14), SU(6), SU(3), SU(11), SU(1), SU(9), SU(14), SU(7), SU(13), SU(5), /* */
#if 1
		/* halftone dither matrix (rotated 45 degrees, looks better) */
		SU(5), SU(12), SU(14), SU(3), SU(8), SU(0), SU(6), SU(10), SU(13), SU(2), SU(4), SU(14), SU(7), SU(11), SU(9), SU(1) /* */
#else
		/* halftone dither matrix (traditional orientation) */
		SU(12), SU(4), SU(11), SU(14), SU(8), SU(0), SU(3), SU(7), SU(6), SU(2), SU(1), SU(9), SU(14), SU(10), SU(5), SU(13) /* */
#endif
	};

/* CAS - 8x8 matrices - NOTE I just duplicated the halftone one... */
	static UBYTE dmatrix24_8[] = { /* ordered, halftone */
	     /* ordered dither matrix */
	       0, 192,  48, 240,  12, 204,  60, 252,
	     128,  64, 176, 112, 140,  76, 188, 124,
	      32, 224,  16, 208,  44, 236,  28, 220,
	     160,  96, 144,  80, 172, 108, 156,  92,
	       8, 200,  56, 248,   4, 196,  52, 244,
	     136,  72, 184, 120, 132,  68, 180, 116,
	      40, 232,  24, 216,  36, 228,  20, 212,
	     168, 104, 152,  88, 164, 100, 148,  84,
	     /* halftone dither matrix (rotated 45 degrees, looks better) */
	     SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),
	     SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),
	     SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),
	     SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1),
	     SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),
	     SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),
	     SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),
	     SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1)
}; 

/* CAS - 16x16 matrices - NOTE I just duplicated the halftone one... */
	static UBYTE dmatrix24_16[] = { /* ordered, halftone */
	/* ordered dither matrix */
       0, 192,  48, 240,  12, 204,  60, 252,   3, 195,  51, 243,  15, 207,  63, 255,
     128,  64, 176, 112, 140,  76, 188, 124, 131,  67, 179, 115, 143,  79, 191, 127,
      32, 224,  16, 208,  44, 236,  28, 220,  35, 227,  19, 211,  47, 239,  31, 223,
     160,  96, 144,  80, 172, 108, 156,  92, 163,  99, 147,  83, 175, 111, 159,  95,
       8, 200,  56, 248,   4, 196,  52, 244,  11, 203,  59, 251,   7, 199,  55, 247,
     136,  72, 184, 120, 132,  68, 180, 116, 139,  75, 187, 123, 135,  71, 183, 119,
      40, 232,  24, 216,  36, 228,  20, 212,  43, 235,  27, 219,  39, 231,  23, 215,
     168, 104, 152,  88, 164, 100, 148,  84, 171, 107, 155,  91, 167, 103, 151,  87,
       2, 194,  50, 242,  14, 206,  62, 254,   1, 193,  49, 241,  13, 205,  61, 253,
     130,  66, 178, 114, 142,  78, 190, 126, 129,  65, 177, 113, 141,  77, 189, 125,
      34, 226,  18, 210,  46, 238,  30, 222,  33, 225,  17, 209,  45, 237,  29, 221,
     162,  98, 146,  82, 174, 110, 158,  94, 161,  97, 145,  81, 173, 109, 157,  93,
      10, 202,  58, 250,   6, 198,  54, 246,   9, 201,  57, 249,   5, 197,  53, 245,
     138,  74, 186, 122, 134,  70, 182, 118, 137,  73, 185, 121, 133,  69, 181, 117,
      42, 234,  26, 218,  38, 230,  22, 214,  41, 233,  25, 217,  37, 229,  21, 213,
     170, 106, 154,  90, 166, 102, 150,  86, 169, 105, 153,  89, 165, 101, 149,  85,
	/* halftone dither matrix (rotated 45 degrees, looks better) */

     SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),
     SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),
     SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),
     SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1),
     SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),
     SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),
     SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),
     SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1),
     SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),
     SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),
     SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),
     SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1),
     SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),SU(5), SU(12), SU(14), SU(3),
     SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),SU(8), SU(0), SU(6), SU(10),
     SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),SU(13), SU(2), SU(4), SU(14),
     SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1),SU(7), SU(11), SU(9), SU(1)

}; 


#if TDEBUG
    ULONG t1, t2, tl1, tl2;
    t1 = ReadVBlankTime();
#endif

#if SDEBUG
	kprintf("PCDumpRPort=%lx, roundit=%lx, GetSpecialDensity=%lx\n",
		&PCDumpRPort(), &roundit(), &GetSpecialDensity());
	kprintf("yexe=%lx, yexr=%lx, yrxe=%lx, yrxr=%lx\n",
		&YEnlargeXEnlarge(), &YEnlargeXReduce(),
		&YReduceXEnlarge(), &YReduceXReduce());
	kprintf("InitHamArray=%lx, ScanConvertPixelArray=%lx\n",
		&InitHamArray(), &ScanConvertPixelArray());
	kprintf("ConvertPixelArray=%lx, CompactPixelArray=%lx\n",
		&ConvertPixelArray(), &CompactPixelArray());
	kprintf("TransferPixelArray=%lx, CheckBuf=%lx, GetBlack=%lx\n",
		&TransferPixelArray(), &CheckBuf(), &GetBlack());
	kprintf("FixScalingVars=%lx, Force1to1Scaling=%lx, SwapInts=%lx\n",
		&FixScalingVars(), &Force1to1Scaling(), &SwapInts());
	kprintf("FixColorMap=%lx, FixColorsPixelArray=%lx\n",
		&FixColorMap(), &FixColorsPixelArray());
	kprintf("PCQuery=%lx, FloydPixelArray=%lx, FloydSwapDest=%lx\n",
		&PCQuery(), &FloydPixelArray(), &FloydSwapDest());
	kprintf("AliasSwapBufs=%lx, AliasPixelArray=%lx\n",
		&AliasSwapBufs(), &AliasPixelArray());
	kprintf("PD=%lx, PED=%lx, render=%lx, PInfo=%lx\n",
		PD, PED, PED->ped_Render, &PInfo);
#endif /* SDEBUG */
	loop = sizeof(struct PrtInfo);
	ptr = (UBYTE *)&PInfo;
	do {
		*ptr++ = 0; /* zero all members of structure */
	} while (--loop);

	PInfo.pi_rp = rp = ior->io_RastPort;
	xstart = ior->io_SrcX;
	ystart = ior->io_SrcY;
	width = ior->io_SrcWidth;
	height = ior->io_SrcHeight;
	pc = ior->io_DestCols;
	pr = ior->io_DestRows;
	PInfo.pi_render = render = PED->ped_Render;
	PInfo.pi_PrefsFlags = PD->pd_Preferences.PrintFlags;
	Dimension = PInfo.pi_PrefsFlags & DIMENSIONS_MASK;
	special = ior->io_Special;
	GetSpecialDensity(&special);

#if	DEBUG0
	kprintf("PCDumpRPort: enter, ior=%08lx\n", ior);
	kprintf("RastPort=%08lx, ColorMap=%08lx, Modes=%04lx, Special=%04lx, PrefsFlags=%04lx\n",
		rp, ior->io_ColorMap, ior->io_Modes, special, PInfo.pi_PrefsFlags);
	kprintf(
"xstart,ystart=%ld,%ld, width,height=%ld,%ld, pc,pr=%ld,%ld, render=%lx\n",
		xstart, ystart, width, height, pc, pr, render);
	kprintf("SPECIAL = ");
	if (special & SPECIAL_MILCOLS) {
		kprintf("MILCOLS ");
	}
	if (special & SPECIAL_MILROWS) {
		kprintf("MILROWS ");
	}
	if (special & SPECIAL_FULLCOLS) {
		kprintf("FULLCOLS ");
	}
	if (special & SPECIAL_FULLROWS) {
		kprintf("FULLROWS ");
	}
	if (special & SPECIAL_FRACCOLS) {
		kprintf("FRACCOLS ");
	}
	if (special & SPECIAL_FRACROWS) {
		kprintf("FRACROWS ");
	}
	if (special & SPECIAL_CENTER) {
		kprintf(" ");
	}
	if (special & SPECIAL_ASPECT) {
		kprintf("ASPECT ");
	}
	if (ltemp = (special & SPECIAL_DENSITYMASK)) {
		if (ltemp == SPECIAL_DENSITY1) {
			kprintf("DENSITY1 ");
		}
		else if (ltemp == SPECIAL_DENSITY2) {
			kprintf("DENSITY2 ");
		}
		else if (ltemp == SPECIAL_DENSITY3) {
			kprintf("DENSITY3 ");
		}
		else if (ltemp == SPECIAL_DENSITY4) {
			kprintf("DENSITY4 ");
		}
		else if (ltemp == SPECIAL_DENSITY5) {
			kprintf("DENSITY5 ");
		}
		else if (ltemp == SPECIAL_DENSITY6) {
			kprintf("DENSITY6 ");
		}
		else {
			kprintf("DENSITY7 ");
		}
	}
	if (special & SPECIAL_NOFORMFEED) {
		kprintf("NOFORMFEED ");
	}
	if (special & SPECIAL_TRUSTME) {
		kprintf("TRUSTME ");
	}
	if (special & SPECIAL_NOPRINT) {
		kprintf("NOPRINT ");
	}
	if (special & SPECIAL_COLORFUNC) {
		kprintf("COLORFUNC ");
	}
	kprintf("\n");
	kprintf("PRINTFLAGS = ");
	if (PInfo.pi_PrefsFlags & CORRECT_RED) {
		kprintf("CORRECT_RED ");
	}
	if (PInfo.pi_PrefsFlags & CORRECT_GREEN) {
		kprintf("CORRECT_GREEN ");
	}
	if (PInfo.pi_PrefsFlags & CORRECT_BLUE) {
		kprintf("CORRECT_BLUE ");
	}
	if (PInfo.pi_PrefsFlags & CENTER_IMAGE) {
		kprintf("CENTER_IMAGE ");
	}
	if (Dimension & MULTIPLY_DIMENSIONS) {
		kprintf("MULTIPLY_DIMENSIONS ");
	}
	else if (Dimension & PIXEL_DIMENSIONS) {
		kprintf("PIXEL_DIMENSIONS ");
	}
	else if (Dimension & BOUNDED_DIMENSIONS) {
		kprintf("BOUNDED_DIMENSIONS ");
	}
	else if (Dimension & ABSOLUTE_DIMENSIONS) {
		kprintf("ABSOLUTE_DIMENSIONS ");
	}
	else {
		kprintf("IGNORE_DIMENSIONS ");
	}
	if (PInfo.pi_PrefsFlags & INTEGER_SCALING) {
		kprintf("INTEGER_SCALING ");
	}
	if ((PInfo.pi_PrefsFlags & DITHERING_MASK) == HALFTONE_DITHERING) {
		kprintf("HALFTONE_DITHERING ");
	}
	else if ((PInfo.pi_PrefsFlags & DITHERING_MASK) == FLOYD_DITHERING) {
		kprintf("FLOYD_DITHERING ");
	}
	else {
		kprintf("ORDERED_DITHERING ");
	}
	if (PInfo.pi_PrefsFlags & ANTI_ALIAS) {
		kprintf("ANTI_ALIAS ");
	}
	kprintf("\n");
#endif /* DEBUG0 */

	if (!(PED->ped_PrinterClass & PPCF_GFX)) {
		return(Backout(PDERR_NOTGRAPHICS, ior, &PInfo));
	}

	/* MUST DO THIS FIRST AS IT MAY ALTER SOME VARS IN PED */
	Center =
		(PInfo.pi_PrefsFlags & CENTER_IMAGE) | (special & SPECIAL_CENTER);
	if (Center) { /* if want to center picture */
		special &= ~SPECIAL_CENTER; /* clear flag since we did the work */
	}
#if DEBUG0
	kprintf("calling case 5 render (init density stuff)\n");
#endif /* DEBUG0 */
	/* init special stuff (density, etc.) */
    /* the 5th parameter here is for V1.0 compatability */
	if (err = (*render)(ior, special, 0, 5, special)) {
#if DEBUG0
		kprintf("case 5 render failed, calling Backout with err=%ld\n", err);
#endif /* DEBUG0 */
		return(Backout(err, ior, &PInfo));
	}


	PInfo.pi_special = special;


	/* CAS - set up standard 4x4 in 12 or 24-bit,
	 * and new 8x8 and 16x16 (both 24-bit)
	 */
	PInfo.pi_dmatrix8 = (PInfo.pi_PrefsFlags & HALFTONE_DITHERING) ?
			&dmatrix24_8[64] : dmatrix24_8;
	PInfo.pi_dmatrix16 = (PInfo.pi_PrefsFlags & HALFTONE_DITHERING) ?
			&dmatrix24_16[256] : dmatrix24_16;

	if((PED->ped_ColorClass & PCC_24BIT))
	    {
	    PInfo.pi_dmatrix = (PInfo.pi_PrefsFlags & HALFTONE_DITHERING) ?
			&dmatrix24[16] : dmatrix24;
	    }
	else
	    PInfo.pi_dmatrix = (PInfo.pi_PrefsFlags & HALFTONE_DITHERING) ?
		&dmatrix[16] : dmatrix;

	/* set up convenience variables */

	/* set flag for whether the printer has black capabilities */
	temp1 = PED->ped_ColorClass & (PCC_BW | PCC_YMC | PCC_YMC_BW | PCC_YMCB);
	if (temp1 != PCC_YMC) { /* if not YMC */
		/* if not YMC_BW or (YMC_BW and not a color picture) */
		if (temp1 != PCC_YMC_BW || (temp1 == PCC_YMC_BW &&
			PD->pd_Preferences.PrintShade != SHADE_COLOR)) {
			PInfo.pi_flags |= PRT_BLACKABLE; /* prt has black capabilities */
		}
	}
	/* set flag for horizontal or vertical pictures */
	if (PD->pd_Preferences.PrintAspect == ASPECT_VERT) {
		PInfo.pi_flags |= PRT_INVERT;
	}
	/* set flag for a b/w or half-tone pictures */
	if (PD->pd_Preferences.PrintShade != SHADE_COLOR) {
		PInfo.pi_flags |= PRT_BW;
	}
	/* set flag for non-dithering of b/w pictures */
	if (PD->pd_Preferences.PrintShade == SHADE_BW) {
		PInfo.pi_threshold = PD->pd_Preferences.PrintThreshold;
	}
	else {
		PInfo.pi_threshold = 0;
	}

	if (ior->io_Modes & HAM) {
		PInfo.pi_flags |= PRT_HAM;
	}
	HiRes = (ior->io_Modes & HIRES) != 0; /* hires flag */
	Lace = (ior->io_Modes & LACE) != 0; /* interlace flag */

	if (layer = rp->Layer) { /* if doing a layer */
		if (layer->SuperBitMap) { /* SuperBitMap ? */
			maxcols = layer->SuperBitMap->BytesPerRow * 8;
			maxrows = layer->SuperBitMap->Rows;

			if (GfxBase->LibNode.lib_Version >= 39) {
				maxcols = GetBitMapAttr(layer->SuperBitMap,BMA_WIDTH);
				maxrows = GetBitMapAttr(layer->SuperBitMap,BMA_HEIGHT);

			}

		}
		else { /* not a SBM */
			maxcols = layer->bounds.MaxX - layer->bounds.MinX + 1;
			maxrows = layer->bounds.MaxY - layer->bounds.MinY + 1;
		}
	}
	else { /* else doing a screen (bitmap) */
		maxcols = rp->BitMap->BytesPerRow * 8;
		maxrows = rp->BitMap->Rows;

		if (GfxBase->LibNode.lib_Version >= 39) {
			maxcols = GetBitMapAttr(rp->BitMap,BMA_WIDTH);
			maxrows = GetBitMapAttr(rp->BitMap,BMA_HEIGHT);

		}
	}
#if	DEBUG0
	kprintf("source size: maxcols=%ld, maxrows=%ld\n", maxcols, maxrows);
#endif /* DEBUG0 */

	if ( (xstart<0) || (ystart<0) || (width<=0) || (height<=0)
		|| (xstart + width > maxcols) || (ystart + height > maxrows) ) {
		return(Backout(PDERR_BADDIMENSION, ior, &PInfo));
	}

	XDotsInch = PED->ped_XDotsInch;
	YDotsInch = PED->ped_YDotsInch;

	/* calculate maximum printer width */
	MaxXDots = PED->ped_MaxXDots;
	/* BOUNDED | ABSOLUTE | PIXELS | MULTIPLY */
	if (Dimension != IGNORE_DIMENSIONS) {
		/* get raw number (assume pixels for PIXEL_DIMENSIONS) */
		PrefsMaxWidth = PD->pd_Preferences.PrintMaxWidth;
		if (Dimension & MULTIPLY_DIMENSIONS) {
			PrefsMaxWidth *= width; /* use max width as a multiplier */
		}
		else if (!(Dimension & PIXEL_DIMENSIONS)) {
			/* convert max width from 10ths/" to printer pixels */
			PrefsMaxWidth = (PrefsMaxWidth * XDotsInch + 5) / 10;
		}
	}
	else { /* IGNORE_DIMENSIONS */
		PrefsMaxWidth = (PD->pd_Preferences.PrintRightMargin + 1 -
			PD->pd_Preferences.PrintLeftMargin) * XDotsInch;
		switch (PD->pd_Preferences.PrintPitch) {
			case ELITE:
				PrefsMaxWidth = (PrefsMaxWidth + 6) / 12;
				break;
			case FINE:
				PrefsMaxWidth = (PrefsMaxWidth + 7) / 15;
				break;
			default /* PICA */:
				PrefsMaxWidth = (PrefsMaxWidth + 5) / 10;
				break;
		}
	}
	/*
		We want the lessor of the two BUT not if 'PrefsMaxWidth == 0'
		since this is a special case which means use the maximum width.
	*/
	maxX = (PrefsMaxWidth && (PrefsMaxWidth < MaxXDots)) ?
		PrefsMaxWidth : MaxXDots;
#if DEBUG0
	kprintf("MaxXDots=%ld, PrefsMaxWidth=%ld, maxX=%ld\n",
		MaxXDots, PrefsMaxWidth, maxX);
#endif

	/* calculate maximum printer height */
	MaxYDots = PED->ped_MaxYDots;
	/* BOUNDED | ABSOLUTE | PIXELS | MULTIPLY */
	if (Dimension != IGNORE_DIMENSIONS) {
		/* get raw number (assume pixels for PIXEL_DIMENSIONS) */
		PrefsMaxHeight = PD->pd_Preferences.PrintMaxHeight;
		if (Dimension & MULTIPLY_DIMENSIONS) {
			PrefsMaxHeight *= height; /* use max height as a multiplier */
		}
		else if (!(Dimension & PIXEL_DIMENSIONS)) {
			/* convert max height from 10ths/" to printer pixels */
			PrefsMaxHeight = (PrefsMaxHeight * YDotsInch + 5) / 10;
		}
	}
	else { /* IGNORE_DIMENSIONS */
		PrefsMaxHeight = PD->pd_Preferences.PaperLength * YDotsInch;
		switch (PD->pd_Preferences.PrintSpacing) {
			case EIGHT_LPI:
				PrefsMaxHeight = (PrefsMaxHeight + 4) / 8;
				break;
			default /* SIX_LPI */:
				PrefsMaxHeight = (PrefsMaxHeight + 3) / 6;
				break;
		}
	}
	/*
		We want the lessor of the two BUT not if 'PrefsMaxHeight == 0'
		or 'MaxYDots == 0' since both are special cases which mean
		use the maximum height.
	*/
	maxY = (PrefsMaxHeight && (PrefsMaxHeight < MaxYDots) || !MaxYDots) ?
		PrefsMaxHeight : MaxYDots;
	/*
		If maxY is 0 then the printer has no maximum height.  So we
		chose an arbitrary large number (100 feet) for the maximum
		height.
	*/
	if (maxY == 0) {
		maxY = 600 * 1200; /* 600 dpi * 1200 inches (100 ft) */
	}
#if DEBUG0
	kprintf("MaxYDots=%ld, PrefsMaxHeight=%ld, maxY=%ld\n",
		MaxYDots, PrefsMaxHeight, maxY);
#endif

	if (Dimension &
		(ABSOLUTE_DIMENSIONS | PIXEL_DIMENSIONS | MULTIPLY_DIMENSIONS)) {
		special &= ~SPECIAL_DIMENSIONSMASK; /* clear flags */
		pc = PrefsMaxWidth; /* force # of print columns */
		pr = PrefsMaxHeight; /* force # of print rows */
	}

#if	DEBUG0
	kprintf("Ver=%ld, Rev=%ld, DPMY=%ld, DPMX=%ld\n",
		GfxBase->LibNode.lib_Version, GfxBase->LibNode.lib_Revision,
		GfxBase->NormalDPMY, GfxBase->NormalDPMX);
#endif /* DEBUG0 */

	/*
		The following is a known bug/feature!  Technically, special
		must be zero along with pc or pr to enable aspecting.
		However, some software packages assume that this bug is a
		feature so I cannot fix it.  Arrgh!
	*/
/*	if ((special == 0 && (pc == 0 || pr == 0)) ||
*/	if ((pc == 0 || pr == 0) ||
		(special & SPECIAL_ASPECT)) {
		Aspect = TRUE;
		/* Hedley Hi-Res has an aspect ratio of 15:16 (1 : 1.06666) */
		if (PD->pd_Preferences.LaceWB & 128) {
			aspectX = 15;
			aspectY = 16;
		}
		else {
			if (GfxBase->LibNode.lib_Version >= 33) {
				aspectX = roundit(10 * GfxBase->NormalDPMY / 182);
				aspectY = roundit(10 * GfxBase->NormalDPMX / 182);

			}

			/* graphics.library < V33 */
			else {
				aspectX = 6;
				aspectY = 7;
			}
			if (!HiRes) {
				aspectX += aspectX;
			}
			if (!Lace) {
				aspectY += aspectY;
			}
		}

		/* New for V36 - we let graphics.library */
		/* tell us what the aspect ratio is, but */
		/* we have set-up some defaults above    */
		/* incase our checks don't pass.         */
		/* We do check for valid modeID, and let */
		/* graphics.library check for a garbage  */
		/* modeID.                               */

		/* For compatability with our own docs   */
		/* there is now code here to check to	 */
		/* to see if someone swizzled GfxBase    */
		/* NormalDPMX/DPMY - (sigh)              */

		if(GfxBase->LibNode.lib_Version >= 36)
		{
		    /* default is NTSC                       */
		    /* graphics uses these values hard-coded */
		    /* and infact never uses them, but sets  */
		    /* them up at startup for historical     */
		    /* reasosn.  Sadly, they are also used   */
		    /* by the printer.device, and documented */
		    /* as values you can swizzle :-(         */

		    DefaultDPMX=1280;
		    DefaultDPMY=1098;		/* roughly 6/7 */

		    if(GfxBase->DisplayFlags & PAL)
		    {
			DefaultDPMX=1226;
			DefaultDPMY=1299;	/* roughly 1/1 */
		    }

		    if(GfxBase->NormalDPMX == DefaultDPMX)
		    {
			if(GfxBase->NormalDPMY == DefaultDPMY)
			{
			    modeID=ior->io_Modes;
			    if(modeID != INVALID_ID)
			    {
				/* validate ID */
				/* graphics lib knows what a good id is*/

				if(mode_handle=FindDisplayInfo(modeID))
				{
				    if(GetDisplayInfoData(mode_handle,
					(UBYTE *)&dpinfo,
					sizeof(struct DisplayInfo),
					DTAG_DISP,modeID))
				    {
#if ADEBUG
					kprintf("V36 Aspect ratio %ld : %ld\n",
						dpinfo.Resolution.x,
						dpinfo.Resolution.y);
#endif
					aspectX=(ULONG)dpinfo.Resolution.x;
					aspectY=(ULONG)dpinfo.Resolution.y;
				    }
				}
			    }
			}
		    }
		}
	}
	else {
		Aspect = FALSE;
	}

	/* compute inverting stuff */
	if (PInfo.pi_flags & PRT_INVERT) { /* if want to invert picture */
		/* swap x&y vars */
		temp1 = ystart;
		ystart = xstart;
		xstart = temp1 + height - 1;
		ltemp = pc;
		pc = pr;
		pr = ltemp;
		ltemp = aspectX;
		aspectX = aspectY;
		aspectY = ltemp;
		temp1 = width;
		width = height;
		height = temp1;
		/* swap spc bits 1, 3, and 5 (0x15) with bits 2, 4, and 6 (0x2a) */
		ulong1 = special & 0x3f; /* get first 6 bits */
		ulong2 = special & ~0x3f; /* get all but first 6 bits */
		special = ulong2 | ((ulong1 >> 1) & 0x15) | ((ulong1 << 1) & 0x2a);
#if	DEBUG0
		kprintf("inversion corrected pc=%ld, pr=%ld, special=%04lx\n",
			pc, pr, special);
#endif
	}

	/* old auto enlarge/reduce */
	if ((special == 0) && ((pc < 0) || (pr < 0))) {
		pc = width * pc / pr;
		if (pc < 0) {
			pc = -pc;
		}
		pr = 0;
#if	DEBUG0
		kprintf("obsolete auto enlarge/reduce: pc=%ld, pr=%ld\n", pc, pr);
#endif
	}

	/*
		At this point we have taken care of the special case where
		pc and/or pr may be < 0.  The are now definately >= 0.  Thus
		we are going to switch to ulongpc and ulongpr which gives us
		an extra bit of precision.
	*/
	ulongpc = (ULONG)pc;
	ulongpr = (ULONG)pr;

	if (special & SPECIAL_MILCOLS) {
		ulongpc = (ulongpc * XDotsInch + 500) / 1000;
#if	DEBUG0
		kprintf("milcorrected ulongpc=%ld\n", ulongpc);
#endif
	}
	if (special & SPECIAL_MILROWS) {
		ulongpr = (ulongpr * YDotsInch + 500) / 1000;
#if	DEBUG0
		kprintf("milcorrected ulongpr=%ld\n", ulongpr);
#endif
	}

	/* same bug/feature as explained above */
/*	if ((special == 0 && ulongpc == 0) || (special & SPECIAL_FULLCOLS)) {
*/	if (ulongpc == 0 || (special & SPECIAL_FULLCOLS)) {
		ulongpc = maxX;
#if	DEBUG0
		kprintf("full corrected ulongpc=%ld\n", ulongpc);
#endif
	}
	else if (special & SPECIAL_FRACCOLS) {
		ulongpc >>= 15;
		ulongpc++;
		ulongpc = ((ulongpc >> 1) * maxX) >> 16;
#if DEBUG0
		kprintf("frac corrected ulongpc=%ld\n", ulongpc);
#endif
	}
/*	if ((special == 0 && ulongpr == 0) || (special & SPECIAL_FULLROWS)) {
*/	if (ulongpr == 0 || (special & SPECIAL_FULLROWS)) {
		ulongpr = maxY;
#if	DEBUG0
		kprintf("full corrected ulongpr=%ld\n", ulongpr);
#endif
	}
	else if (special & SPECIAL_FRACROWS) {
		ulongpr >>= 15;
		ulongpr++;
		/*
			Old code (can cause 32 bit overflow).
			ulongpr = ((ulongpr >> 1) * maxY) >> 16;
		*/
		ulongpr >>= 1;
		loop = 0;
		temp1 = 1;
		temp2 = 0;
		do {
			ulong1 = (maxY + temp2) / temp1;
			ulong2 = ulongpr * ulong1;
			loop++;
			temp1 *=2;
			temp2 = temp1 / 2;
		} while (ulong2 / ulongpr != ulong1);
		ulongpr = ulong2 >> 16;
#if DEBUG0
		kprintf("frac corrected ulongpr=%ld, loop=%ld, temp1=%ld\n",
			ulongpr, loop, temp1);
#endif
	}

	/* else use ulongpr & ulongpc as absolute values */

	if (Aspect) { /* aspect ratio correct image */
#if	DEBUG0
		kprintf("aspecting...");
#endif
		/*
			This is a KLUDGE so that I don't overrun ulong1 and
			ulong2.  I start by dividing the temp. values ulong3
			and ulong 4 by 1.  If the result of ulong3 x ulongpr
			or ulong4 x ulongpc overruns 32 bits then I go back
			and do it again BUT I increase the ulong3 and ulong4
			divisor by a power of 2.  This will prevent overrun
			at the expence of precision.
		*/
		loop = 0;
		temp1 = 1;
		temp2 = 0;
		do {
			ulong3 = (width * aspectX * XDotsInch + temp2) / temp1;
			ulong4 = (height * aspectY * YDotsInch + temp2) / temp1;
			ulong1 = ulongpr * ulong3;
			ulong2 = ulongpc * ulong4;
			loop++;
			temp1 *=2;
			temp2 = temp1 / 2;
		} while ((ulong1 / ulongpr != ulong3) ||
			(ulong2 / ulongpc != ulong4));
#if DEBUG0
		kprintf("width=%ld, aspectX=%ld, XDotsInch=%ld\n",
			width, aspectX, XDotsInch);
		kprintf("height=%ld, aspectY=%ld, YDotsInch=%ld\n",
			height, aspectY, YDotsInch);
		kprintf("ulong3=%ld, ulong4=%ld, ulong1=%ld, ulong2=%ld\n",
			ulong3, ulong4, ulong1, ulong2);
	kprintf("loop=%ld, temp1=%ld, temp2=%ld, ulongpc=%ld, ulongpr=%ld\n",
			loop, temp1, temp2, ulongpc, ulongpr);
#endif
		if (ulong1 > ulong2) {
			/* diminish ulongpr to correct aspect ratio */
			ulongpr = ulong2 / ulong3;
#if DEBUG0
			kprintf("aspect corrected ulongpr=%ld\n", ulongpr);
#endif
		}
		else if (ulong1 < ulong2) {
			/* diminish ulongpc to correct aspect ratio */
			ulongpc = ulong1 / ulong4;
#if	DEBUG0
			kprintf("aspect corrected ulongpc=%ld\n", ulongpc);
#endif
		}
	}

	/*
		If INTEGER scaling is on then we must scale the # of print columns
		and print rows up or down to the nearest multiple of width and
		height respectively.
	*/
	if (PInfo.pi_PrefsFlags & INTEGER_SCALING) {
#if DEBUG0
		kprintf("INTEGER: ulongpc=%ld, width=%ld, ", ulongpc, width);
#endif
		if (ulongpc < width) {
			ulongpc = width;
		}
		else {
			temp1 = ulongpc % width;
			if (temp1 < width / 2) {
				ulongpc -= temp1;
			}
			else {
				ulongpc += width - temp1;
			}
		}
#if DEBUG0
		kprintf("CORRECTED ulongpc=%ld\nINTEGER: ulongpr=%ld, height=%ld, ",
			ulongpc, ulongpr, height);
#endif
		if (ulongpr < height) {
			ulongpr = height;
		}
		else {
			temp1 = ulongpr % height;
			if (temp1 < height / 2) {
				ulongpr -= temp1;
			}
			else {
				ulongpr += height - temp1;
			}
		}
#if DEBUG0
		kprintf("CORRECTED ulongpr=%ld\n", ulongpr);
#endif
	}
	/*
		If the printer's maximum width is not 0 and the requested
		number of print columns (ulongpc) is greater then the
		printer's maximum width, then set ulongpc to the printer's
		maximum width.
	*/
	if (MaxXDots && (ulongpc > MaxXDots)) {
		ulongpc = MaxXDots;
	}
	/*
		If the printer's maximum height is not 0 and the requested
		number of print rows (ulongpr) is greater then the printer's
		maximum height, then set pr to the printer's maximum height.
	*/
	if (MaxYDots && (ulongpr > MaxYDots)) {
		ulongpr = MaxYDots;
	}

	/*
		At this point 'ulongpc' is at its final value so it is safe to
		calculate the x offset.
	*/
	if (Center) {
		/* calc start posn (note: xpos range is 0 to MaxXDots - 1) */
		PInfo.pi_xpos = (PED->ped_MaxXDots - ulongpc + 1) / 2;
	}
	else { /* check for a user specified x offset */
		if (PrefsXOffset = PD->pd_Preferences.PrintXOffset) {
			/* convert 10ths/" offset to printer pixels */
			PrefsXOffset = (PrefsXOffset * PED->ped_XDotsInch + 5) / 10;
			/* if offset too big, make as big as possible */
			if (PrefsXOffset + ulongpc > PED->ped_MaxXDots) {
				PrefsXOffset = PED->ped_MaxXDots - ulongpc;
			}
			PInfo.pi_xpos = PrefsXOffset;
		}
	}

	/*
		At this point 'ulongpc' and 'ulongpr' are at their final
		value.  If 'NOPRINT' is selected, then we return these values
		to the calling program.
	*/
	if (special & SPECIAL_NOPRINT) {
		ior->io_DestCols = ulongpc + PInfo.pi_xpos;;
		ior->io_DestRows = ulongpr;
		return(Backout(PDERR_NOERR, ior, &PInfo)); /* exit cleanly */
	}

	/* compute y scaling stuff */
	if (YReduce = (height > ulongpr)) { /* if Y REDUCTION */
		PInfo.pi_ymult = height / ulongpr;
		PInfo.pi_ymod = height % ulongpr;
		PInfo.pi_ety = ulongpr;
	}
	else { /* Y ENLARGEMENT or 1:1 */
		PInfo.pi_ymult = ulongpr / height;
		PInfo.pi_ymod = ulongpr % height;
		PInfo.pi_ety = height;
	}

	/* compute x scaling stuff */
	if (XReduce = (width > ulongpc)) { /* if X REDUCTION */
		xmult = width / ulongpc;
		xmod = width % ulongpc;
		etx = ulongpc;
	}
	else { /* X ENLARGEMENT or 1:1 */
		xmult = ulongpc / width;
		xmod = ulongpc % width;
		etx = width;
	}

	/* can alloc mem from this point on cause the NOPRINT option is above */

	depth = rp->BitMap->Depth;

	if (GfxBase->LibNode.lib_Version >= 39) {
		depth = GetBitMapAttr(rp->BitMap,BMA_DEPTH);

	}


	/* see if there are too many planes for ClipBlit use */
	if (depth > MAXDEPTH) {
#if DEBUG0
		kprintf("TOO MANY PLANES FOR CLIPBLIT!\n");
#endif
		PInfo.pi_flags |= PRT_NOBLIT; /* cant use blitter */
	}
	/* see if any of the original rp's planes are in fast mem */
	for (loop=0; loop<depth; loop++) {
		if ((TypeOfMem(rp->BitMap->Planes[loop]) & MEMF_CHIP) != MEMF_CHIP) {
#if DEBUG0
			kprintf("SOURCE RP PLANES ARE NOT IN CHIP MEM!\n");
#endif
			PInfo.pi_flags |= PRT_NOBLIT; /* cant use blitter */
			break;
		}
	}
	/* if not using blitter and this rp has a layer */
	if ((PInfo.pi_flags & PRT_NOBLIT) && layer) {
		for (cliprect = layer->ClipRect; cliprect != NULL;
			cliprect = cliprect->Next) {
			if (cliprect->lobs != NULL) {
				PInfo.pi_flags |= PRT_NORPL;
#if DEBUG0
				kprintf("SOURCE RP PLANES HAVE OBSCURED CLIPRECTS!\n");
#endif
				break;
			}
		}
	}

	if (!(PInfo.pi_flags & PRT_NORPL)) {
		/* get mem for temp rastport */
		if ((PInfo.pi_temprp = AllocMem(sizeof(struct RastPort), MEMF_PUBLIC))
			== NULL) {
#if ALLOCDEBUG
			kprintf("couldn't get %ld bytes for rp\n",
				sizeof(struct RastPort));
#endif
			return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
		}
#if ALLOCDEBUG
		kprintf("depth=%ld, got %ld bytes for rp @ %lx-%lx\n",
			depth, sizeof(struct RastPort), PInfo.pi_temprp,
			sizeof(struct RastPort) + (UBYTE *)PInfo.pi_temprp - 1);
#endif /* DEBUG0 */
		*(PInfo.pi_temprp) = *rp; /* copy rp */
		/* make sure we can write to all planes */
		PInfo.pi_temprp->Mask = 0xff;
		PInfo.pi_temprp->BitMap = NULL; /* havnt got this yet */
		PInfo.pi_temprp->Layer = NULL; /* no layers for this rp */
		/* make sure we have enough mem for ALL the planeptrs */
		temp1 = sizeof(struct BitMap) + depth * sizeof(PLANEPTR);
		/* get mem for temp bitmap */
		if ((PInfo.pi_temprp->BitMap = AllocMem(temp1, MEMF_PUBLIC)) ==
			NULL) {
#if ALLOCDEBUG
			kprintf("couldn't get %ld bytes for bm\n",
				temp1);
#endif
			return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
		}
#if ALLOCDEBUG
		kprintf("got %ld bytes for bm @ %lx-%lx\n",
			temp1, PInfo.pi_temprp->BitMap,
			temp1 + (UBYTE *)PInfo.pi_temprp->BitMap - 1);
#endif /* DEBUG0 */
		*(PInfo.pi_temprp->BitMap) = *(rp->BitMap); /* copy rp bitmap */
		/* have not got these yet */
		PInfo.pi_temprp->BitMap->Planes[0] = NULL;
		/* force copy of bitmap to 1 line high */
		PInfo.pi_temprp->BitMap->Rows = 1;
		/*
			we may need to read in an entire row or an entire col of pixels,
			so make sure that the temprp is big enough for either.
		*/
		temp1 = maxcols > maxrows ? maxcols : maxrows;
		temp1 = ((temp1 + 15) / 16) * 2; /* calculate # of BytesPerRow */
		/* set # of BytesPerRow */
		PInfo.pi_temprp->BitMap->BytesPerRow = temp1;
		temp2 = temp1 * depth; /* calculate # of bytes for all planes */
		if ((PInfo.pi_temprp->BitMap->Planes[0] = AllocMem(temp2, MEMF_CHIP))
			== NULL) {
#if ALLOCDEBUG
			kprintf("couldn't get %ld bytes for Planes\n", temp2);
#endif
			return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
		}
#if ALLOCDEBUG
		kprintf("got %ld bytes for Planes[0] @ %lx-%lx\n",
			temp2, PInfo.pi_temprp->BitMap->Planes[0],
			temp2 + (UBYTE *)PInfo.pi_temprp->BitMap->Planes[0] - 1);
#endif /* DEBUG0 */
		/* initialize plane ptrs for temp bitmap */
		for (loop=1; loop<depth; loop++) {
			PInfo.pi_temprp->BitMap->Planes[loop] =
				PInfo.pi_temprp->BitMap->Planes[loop - 1] + temp1;
#if DEBUG0
			kprintf("Planes[%ld] = %lx-%lx\n",
				loop, PInfo.pi_temprp->BitMap->Planes[loop],
				temp1 + PInfo.pi_temprp->BitMap->Planes[loop] - 1);
#endif
		}
	}

	/****** initialize printerColorMap ******/
	/* CAS - COLORFUNC */
	colorfunc = (special & SPECIAL_COLORFUNC) ? 1 : 0;
#if	DEBUG0
		kprintf("colorfunc on=%ld func=$%08lx special=$%04lx\n",
				colorfunc, ior->io_ColorMap, special);
#endif
	if(colorfunc)  {
		PInfo.pi_colorfunc = (void *)ior->io_ColorMap;
		PInfo.pi_flags |= PRT_COLORFUNC;
		temp1 = 64; /* just so code GetBlack(), etc. will work */
	}
	else temp1 = 1 << rp->BitMap->Depth;	/* get # of colors in palette */

	if (GfxBase->LibNode.lib_Version >= 39) {
		temp1 = 1 << GetBitMapAttr(rp->BitMap,BMA_DEPTH);
	}

	if (ior->io_Modes & EXTRA_HALFBRITE) {
		/* only half as many real colors, other half will be computed */
		temp1 >>= 1;
	}
	temp2 = temp1 * sizeof(union colorEntry);
#if DEBUG0
	kprintf("# planes = %ld, # colors = %ld, mem req = %ld bytes\n",
		depth, temp1, temp2);
#endif /* DEBUG0 */
	if (ior->io_Modes & EXTRA_HALFBRITE) {
		temp2 += (32 + 32 - temp1) * sizeof(union colorEntry);
	}
	PInfo.pi_ColorMapSize = temp2;
	if ((cm = AllocMem(temp2, MEMF_PUBLIC)) == NULL) {
#if ALLOCDEBUG
		kprintf("couldn't get %ld bytes for ColorMap\n", temp2);
#endif
		return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
	}
	PInfo.pi_ColorMap = cm;
#if ALLOCDEBUG
	kprintf("got %ld bytes (%ld entries) for ColorMap @ %lx-%lx\n",
		temp2, temp1, cm, temp2 + (UBYTE *)cm - 1);
#endif /* DEBUG0 */

	/* CAS */
	v39 =  (GfxBase->LibNode.lib_Version >= 39) ? 1 : 0;


	/* CAS */
	if (!colorfunc)	for (loop=0; loop<temp1; loop++) {

		if(v39) {
			GetRGB32(ior->io_ColorMap, loop, 1, colors32);
			cm->colorByte[PCMYELLOW]  = *((UBYTE *)&colors32[2]);
			cm->colorByte[PCMMAGENTA] = *((UBYTE *)&colors32[1]);
			cm->colorByte[PCMCYAN]    = *((UBYTE *)&colors32[0]);
		}
		else {
 			temp2 = GetRGB4(ior->io_ColorMap, loop);
			cm->colorByte[PCMYELLOW] = temp2 & 0xf;
			cm->colorByte[PCMYELLOW] += (cm->colorByte[PCMYELLOW]<<4);
			cm->colorByte[PCMMAGENTA] = (temp2 >> 4) & 0xf;
			cm->colorByte[PCMMAGENTA] += (cm->colorByte[PCMMAGENTA]<<4);
			cm->colorByte[PCMCYAN] = (temp2 >> 8) & 0xf;
			cm->colorByte[PCMCYAN] += (cm->colorByte[PCMCYAN]<<4);
		}
#define DEBUGCAS 0
#if	DEBUGCAS
		kprintf("Init cm=%08lx, colorMap[%02ld]=%08lx\n", cm, loop, cm->colorLong);
#endif
		if (ior->io_Modes & EXTRA_HALFBRITE) {
			(cm + 32)->colorLong = (cm->colorLong >> 1) & 0x7f7f7f7f;
#if	DEBUG2
			kprintf("halfbright[%02ld]=%08lx\n", loop + 32,
				(cm + 32)->colorLong);
#endif
		}
		cm++;
	}

	/*
		At this point the colors are in rgb (ADDITIVE) form.  If we
		want them in ymc (!ADDITIVE) form then we need to convert them.
		If we want negative colors then we also need to convert them.
	*/

	/* CAS  (were 0x0f0f0f0f) */
	mask = (PED->ped_ColorClass & PCC_ADDITIVE) ? 0x0 : 0xffffffff;
	mask ^= (PD->pd_Preferences.PrintImage == IMAGE_NEGATIVE) ?
		0xffffffff : 0x0;
	cm = PInfo.pi_ColorMap;
	for (loop=0; loop<temp1; loop++) {
		cm->colorLong ^= mask;
		if (ior->io_Modes & EXTRA_HALFBRITE) {
			(cm + 32)->colorLong ^= mask;
		}
#if DEBUG2
		kprintf("add/sub corrected[%02ld]=%08lx\n",
			loop, cm->colorLong);
		if (ior->io_Modes & EXTRA_HALFBRITE) {
			kprintf("halfbrite add/sub corrected[%02ld]=%08lx\n",
				loop + 32, (cm + 32)->colorLong);
		}
#endif
		cm++;
	}

	/* calculate black values (MUST BE DONE BEFORE CORRECTING COLORS) */
	cm = PInfo.pi_ColorMap;
	for (loop=0; loop<temp1; loop++) {
		/* calculate black values */
		GetBlack(cm, PInfo.pi_flags);
		if (ior->io_Modes & EXTRA_HALFBRITE) {
			GetBlack(cm + 32, PInfo.pi_flags);
		}
#if	DEBUG2
		kprintf("black corrected[%02ld]=%08lx\n", loop, cm->colorLong);
		if (ior->io_Modes & EXTRA_HALFBRITE) {
			kprintf("halfbrite black corrected[%02ld]=%08lx", loop + 32,
				(cm + 32)->colorLong);
		}
#endif
		cm++;
	}

	/* if want to fix color(s) and NOT in HAM mode and doing a color dump */
	if ((PInfo.pi_PrefsFlags & CORRECT_RGB_MASK) &&
		!(PInfo.pi_flags & PRT_HAM) &&
		(PD->pd_Preferences.PrintShade == SHADE_COLOR)) {
#if DEBUG0
		kprintf("fixing color map\n");
#endif
		FixColorMap(&PInfo);
	}


	PInfo.pi_xstart = xstart;
	PInfo.pi_ystart = ystart;
	PInfo.pi_width = PInfo.pi_tempwidth = width;
	PInfo.pi_height = height;

	/*
		Must round up to nearest multiple of 16 since ReadPixelLine
		works in 16 byte chunks.
	*/
	temp1 = (width + 15) & (~15);
	PInfo.pi_RowBufSize = temp1 * 2;
	if ((PInfo.pi_RowBuf = AllocMem(PInfo.pi_RowBufSize, MEMF_PUBLIC)) ==
		NULL) {
#if ALLOCDEBUG
		kprintf("couldn't get %ld bytes for RowBuf\n", PInfo.pi_RowBufSize);
#endif
		return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
	}
#if ALLOCDEBUG
	kprintf("got %ld bytes (%ld entries) for RowBuf @ %lx-%lx\n",
		PInfo.pi_RowBufSize, temp1, PInfo.pi_RowBuf,
		PInfo.pi_RowBufSize + (UBYTE *)PInfo.pi_RowBuf - 1);
#endif



	PInfo.pi_pc = ulongpc + PInfo.pi_xpos;
	PInfo.pi_pr = ulongpr;

	/* if anti-aliasing , alloc TopBuf and BotBuf */
	if (PInfo.pi_PrefsFlags & ANTI_ALIAS) {
		if ((PInfo.pi_TopBuf = AllocMem(PInfo.pi_RowBufSize, MEMF_PUBLIC)) ==
			NULL) {
#if ALLOCDEBUG
			kprintf("couldn't get %ld bytes for TopBuf\n",
				PInfo.pi_RowBufSize);
#endif
			return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
		}
#if ALLOCDEBUG
		kprintf("got %ld bytes (%ld entries) for TopBuf @ %lx-%lx\n",
			PInfo.pi_RowBufSize, temp1, PInfo.pi_TopBuf,
			PInfo.pi_RowBufSize + (UBYTE *)PInfo.pi_TopBuf - 1);
#endif
		if ((PInfo.pi_BotBuf = AllocMem(PInfo.pi_RowBufSize, MEMF_PUBLIC)) ==
			NULL) {
#if ALLOCDEBUG
			kprintf("couldn't get %ld bytes for BotBuf\n",
				PInfo.pi_RowBufSize);
#endif
			return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
		}
#if ALLOCDEBUG
		kprintf("got %ld bytes (%ld entries) for BotBuf @ %lx-%lx\n",
			PInfo.pi_RowBufSize, temp1, PInfo.pi_BotBuf,
			PInfo.pi_RowBufSize + (UBYTE *)PInfo.pi_BotBuf - 1);
#endif
	}

	/* if floyd dithering or anti-aliasing, alloc Dest1Int */
	if (PInfo.pi_PrefsFlags & (FLOYD_DITHERING | ANTI_ALIAS)) {
		PInfo.pi_Dest1IntSize = (PInfo.pi_pc + 1) * sizeof(union colorEntry);
		if ((PInfo.pi_Dest1Int =
			AllocMem(PInfo.pi_Dest1IntSize, MEMF_PUBLIC|MEMF_CLEAR)) == NULL){
#if ALLOCDEBUG
			kprintf("couldn't get %ld bytes for Dest1Int\n",
				PInfo.pi_Dest1IntSize);
#endif
			return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
		}
#if ALLOCDEBUG
		kprintf("got %ld bytes (%ld entries) for Dest1Int @ %lx-%lx\n",
			PInfo.pi_Dest1IntSize, PInfo.pi_pc + 1, PInfo.pi_Dest1Int,
			PInfo.pi_Dest1IntSize + (UBYTE *)PInfo.pi_Dest1Int - 1);
#endif
	}

	/* if floyd dithering, alloc Dest2Int */
	if (PInfo.pi_PrefsFlags & FLOYD_DITHERING) {
		PInfo.pi_Dest2IntSize = (PInfo.pi_pc + 1) * sizeof(union colorEntry);
		if ((PInfo.pi_Dest2Int =
			AllocMem(PInfo.pi_Dest2IntSize, MEMF_PUBLIC|MEMF_CLEAR)) == NULL){
#if ALLOCDEBUG
			kprintf("couldn't get %ld bytes for Dest2Int\n",
				PInfo.pi_Dest2IntSize);
#endif
			return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
		}
#if ALLOCDEBUG
		kprintf("got %ld bytes (%ld entries) for Dest2Int @ %lx-%lx\n",
			PInfo.pi_Dest2IntSize, PInfo.pi_pc + 1, PInfo.pi_Dest2Int,
			PInfo.pi_Dest2IntSize + (UBYTE *)PInfo.pi_Dest2Int - 1);
#endif
	}

	PInfo.pi_ColorIntSize = width * sizeof(union colorEntry);
	if ((PInfo.pi_ColorInt = AllocMem(PInfo.pi_ColorIntSize, MEMF_PUBLIC)) ==
		NULL) {
#if ALLOCDEBUG
		kprintf("couldn't get %ld bytes for ColorInt\n",
			PInfo.pi_ColorIntSize);
#endif
		return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
	}
#if ALLOCDEBUG
	kprintf("got %ld bytes (%ld entries) for ColorInt @ %lx-%lx\n",
		PInfo.pi_ColorIntSize, width, PInfo.pi_ColorInt,
		PInfo.pi_ColorIntSize + (UBYTE *)PInfo.pi_ColorInt - 1);
#endif



/* CAS - tmp color buffer for row converted to 12-bit */
	if ((PInfo.pi_ColorTmp = AllocMem(PInfo.pi_ColorIntSize, MEMF_PUBLIC)) == NULL) {
#if ALLOCDEBUG
		kprintf("couldn't get %ld bytes for ColorTmp\n", PInfo.pi_ColorIntSize);
#endif
		return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
	}


	/* if doing a HAM picture */
	if (PInfo.pi_flags & PRT_HAM) {
		if (!(PInfo.pi_flags & PRT_INVERT)) { /* if not inverted */
			PInfo.pi_HamIntSize = height * sizeof(union colorEntry);
			temp1 = xstart;
		}
		else { /* inverted */
			PInfo.pi_HamIntSize = width * sizeof(union colorEntry);
			temp1 = ystart;
		}

		if ((PInfo.pi_HamInt = AllocMem(PInfo.pi_HamIntSize, MEMF_PUBLIC))
			== NULL) {
#if ALLOCDEBUG
			kprintf("couldn't get %ld bytes for HamInt\n",
				PInfo.pi_HamIntSize);
#endif
			return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
		}
#if ALLOCDEBUG
		kprintf("got %ld bytes (%ld entries) for HamInt @ %lx-%lx\n",
			PInfo.pi_HamIntSize, height, PInfo.pi_HamInt,
			PInfo.pi_HamIntSize + (UBYTE *)PInfo.pi_HamInt - 1);
#endif
		if (temp1) { /* if not starting at left edge */
			/* allocate mem for left edge buffer */
			PInfo.pi_HamBufSize = temp1 * 2;
			if ((PInfo.pi_HamBuf = AllocMem(PInfo.pi_HamBufSize, MEMF_PUBLIC))
				== NULL) {
#if ALLOCDEBUG
				kprintf("couldn't get %ld bytes for HamBuf\n",
					PInfo.pi_HamBufSize);
#endif
				return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
			}
#if ALLOCDEBUG
			kprintf("got %ld bytes (%ld entries) for HamBuf @ %lx-%lx\n",
				PInfo.pi_HamBufSize, temp1, PInfo.pi_HamBuf,
				PInfo.pi_HamBufSize + (UBYTE *)PInfo.pi_HamBuf - 1);
#endif
		}
		InitHamArray(&PInfo);
	}

	PInfo.pi_ScaleXSize = width * 2;
	if ((PInfo.pi_ScaleX = AllocMem(PInfo.pi_ScaleXSize, MEMF_PUBLIC)) ==
		NULL) {
#if ALLOCDEBUG
		kprintf("couldn't get %ld bytes for ScaleX\n",
			PInfo.pi_ScaleXSize);
#endif
		return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
	}
#if ALLOCDEBUG
	kprintf("got %ld bytes (%ld entries) for ScaleX @ %lx-%lx\n",
		PInfo.pi_ScaleXSize, width, PInfo.pi_ScaleX,
		PInfo.pi_ScaleXSize + (UBYTE *)PInfo.pi_ScaleX - 1);
#endif
	buf1 = PInfo.pi_ScaleX;
	temp1 = width;
	temp2 = etx;
	do {
		*buf1++ = Scale(xmult, xmod, etx, &temp2);
	} while (--temp1);

	/* if floyd dithering or anti-aliasing or xreducing, alloc ScaleXAlt */
	if ((PInfo.pi_PrefsFlags & (FLOYD_DITHERING | ANTI_ALIAS)) || XReduce) {
		PInfo.pi_ScaleXAltSize = PInfo.pi_pc * 2;
		if ((PInfo.pi_ScaleXAlt = AllocMem(PInfo.pi_ScaleXAltSize,
			MEMF_PUBLIC)) == NULL) {
#if ALLOCDEBUG
			kprintf("couldn't get %ld bytes for ScaleXAlt\n",
				PInfo.pi_ScaleXAltSize);
#endif
			return(Backout(PDERR_INTERNALMEMORY, ior, &PInfo));
		}
#if ALLOCDEBUG
		kprintf("got %ld bytes (%ld entries) for ScaleXAlt @ %lx-%lx\n",
			PInfo.pi_ScaleXAltSize, width, PInfo.pi_ScaleXAlt,
			PInfo.pi_ScaleXAltSize + (UBYTE *)PInfo.pi_ScaleXAlt - 1);
#endif
		buf1 = PInfo.pi_ScaleXAlt;
		temp1 = PInfo.pi_pc;
		do {
			*buf1++ = 1;
		} while (--temp1);
	}
#if DEBUG0
	kprintf(
	"xstart,ystart=%ld,%ld, width,height=%ld,%ld, pc,pr=%ld,%ld, xpos=%ld\n",
	xstart, ystart, width, height, PInfo.pi_pc, PInfo.pi_pr, PInfo.pi_xpos);
	kprintf("flags=%lx, threshold=%ld, special=%lx, render=%lx\n",
		PInfo.pi_flags, PInfo.pi_threshold, PInfo.pi_special, PInfo.pi_render);
	kprintf("xmult=%ld, xmod=%ld, etx=%ld, ymult=%ld, ymod=%ld, ety=%ld\n",
		xmult, xmod, etx, PInfo.pi_ymult, PInfo.pi_ymod, PInfo.pi_ety);
#endif
#if TDEBUG
    tl1 = ReadVBlankTime();
#endif
	/* get render buf mem (do this last after all other mem allocations) */
#if DEBUG0
	kprintf("calling case 0 render (getting render buf mem)\n");
#endif /* DEBUG0 */
    /* the 5th parameter here is for V1.0 compatability */
	err = (*render)(ior, ulongpc + PInfo.pi_xpos, ulongpr, 0, ulongpc + PInfo.pi_xpos);
	/* case 0 (memory allocation) has been called */
	PInfo.pi_flags |= PRT_RENDER0;
	if (err != PDERR_NOERR) {
#if DEBUG0
		kprintf("case 0 render failed, calling Backout with err=%ld\n", err);
#endif /* DEBUG0 */
		return(Backout(err, ior, &PInfo));
	}
#if DEBUG0
	kprintf("calling case 3 render (clearing & initing render buf mem)..");
#endif /* DEBUG0 */
	(*render)(0, 0, 0, 3); /* clear & init render buffer */
#if DEBUG0
	kprintf("ok\n");
#endif /* DEBUG0 */

	temp1 = PCMYELLOW; /* assume starting at PCMYELLOW */
	temp2 = temp1 + 1; /* assume one pass */
	if (PED->ped_ColorClass & PCC_MULTI_PASS) { /* if multi-pass */
		if (PD->pd_Preferences.PrintShade == SHADE_COLOR) { /* if color */
			if (PED->ped_ColorClass == PCC_YMCB ||
				PED->ped_ColorClass == PCC_BGRW) { /* if 4 color */
				temp2 = temp1 + 4; /* must do 4 passes */
			}
			else {
				temp2 = temp1 + 3; /* must do 3 passes */
			}
		}
		else { /* if B&W or GreyScale */
			temp1 = PCMBLACK; /* must start at PCMBLACK */
			temp2 = temp1 + 1; /* one pass */
		}
	}

	if (PInfo.pi_ystart + PInfo.pi_height < maxrows) {
#if DEBUG0
		kprintf("PRT_BELOW: ystart=%ld, height=%ld, maxrows=%ld\n",
			PInfo.pi_ystart, PInfo.pi_height, maxrows);
#endif /* DEBUG0 */
		PInfo.pi_flags |= PRT_BELOW;
	}

	for (; temp1<temp2; temp1++) {
		FixScalingVars(&PInfo); /* necessary */
		width = PInfo.pi_width; /* re-init local var */
#if DEBUG0
		kprintf("temp1=%ld, TopBuf=%lx, RowBuf=%lx, BotBuf=%lx\n",
		temp1, PInfo.pi_TopBuf, PInfo.pi_RowBuf, PInfo.pi_BotBuf);
		kprintf("\tColorInt=%lx, Dest1Int=%lx, Dest2Int=%lx\n",
		PInfo.pi_ColorInt, PInfo.pi_Dest1Int, PInfo.pi_Dest2Int);
		kprintf("\tpi_width=%ld, pi_tempwidth=%ld, width=%ld\n",
			PInfo.pi_width, PInfo.pi_tempwidth, width);
#endif
		if (PInfo.pi_PrefsFlags & ANTI_ALIAS) {
			if (PInfo.pi_ystart != 0) {
#if DEBUG0
				kprintf("initing RowBuf to line ystart - 1\n");
#endif
				MyReadPixelArray(PInfo.pi_ystart - 1, &PInfo,
					PInfo.pi_RowBuf);
			}
			else {
#if DEBUG0
				kprintf("initing RowBuf to %lx\n", ~0);
#endif
				buf1 = PInfo.pi_RowBuf;
				/* invalidate RowBuf */
				do {
					*buf1++ = ~0;
				} while (--width);
			}
#if DEBUG0
			kprintf("initing BotBuf to line ystart\n");
#endif
			MyReadPixelArray(PInfo.pi_ystart, &PInfo,
				PInfo.pi_BotBuf);
		}
		/*
			after the above (in one of the four routines below):
				TopBuf <= RowBuf (which we just invalidated)
				RowBuf <= BotBuf (which we just read)
				BotBuf (will be read)
		*/

		if (YReduce && XReduce) {
#if DEBUG0
			kprintf("calling YRXR...");
#endif /* DEBUG0 */
			err = YReduceXReduce(&PInfo, temp1);
		}
		else if (YReduce) {
#if DEBUG0
			kprintf("calling YRXE...");
#endif /* DEBUG0 */
			err = YReduceXEnlarge(&PInfo, temp1);
		}
		else if (XReduce) {
#if DEBUG0
			kprintf("calling YEXR...");
#endif /* DEBUG0 */
			err = YEnlargeXReduce(&PInfo, temp1);
		}
		else {
#if DEBUG0
			kprintf("calling YEXE...");
#endif /* DEBUG0 */
			err = YEnlargeXEnlarge(&PInfo, temp1);
		}
#if DEBUG0
		kprintf("err=%ld\n", err);
#endif /* DEBUG0 */
		if (err) {
			break;
		}
		else {
			if (temp1 != temp2 - 1) { /* if NOT on last pass */
#if DEBUG0
				kprintf("calling case 6 render (switch to next color)\n");
#endif /* DEBUG0 */
				err = (*render)(0, 0, 0, 6); /* switch to next color */
			}
		}
	}
#if TDEBUG
    tl2 = t2 = ReadVBlankTime();
    if (t2 < t1) {
        t2 += 65536;
    }
    if (tl2 < tl1) {
        tl2 += 65536;
    }
    kprintf("PCDumpRPort: t2=%ld, t1=%ld, elapsed jiffies=%ld, secs=%ld\n",
        t2, t1, t2 - t1, (t2 - t1 + 30) / 60);
    kprintf("Loop: tl2=%ld, tl1=%ld, elapsed jiffies=%ld, secs=%ld\n",
        tl2, tl1, tl2 - tl1, (tl2 - tl1 + 30) / 60);
#endif
	return(Backout(err, ior, &PInfo)); /* return status */
}

roundit(number)
int number;
{
	return( (((number % 10) > 4) ? (number + 10) : number) / 10);
}

GetSpecialDensity(special)
ULONG *special;
{
	extern struct PrinterData *PD;

	static UWORD densities[8] = {
		0, SPECIAL_DENSITY1, SPECIAL_DENSITY2, SPECIAL_DENSITY3,
		SPECIAL_DENSITY4, SPECIAL_DENSITY5, SPECIAL_DENSITY6, SPECIAL_DENSITY7
	};
	UWORD temp;

	/* if user density settings override the application density settings */
	if (temp = densities[PD->pd_Preferences.PrintDensity]) {
		*special &= ~SPECIAL_DENSITYMASK; /* clear application settings */
		*special |= temp; /* get user settings */
	}
}
