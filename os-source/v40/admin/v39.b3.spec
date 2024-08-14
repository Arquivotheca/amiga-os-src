V39 Specification
ROM and Disk
June 8, 1992
rev 1.6

 
(©) Copyright 1992 Commodore-Amiga, Inc.  All Rights Reserved.
Proprietary and Confidential.
   

Doc:	rev 1.0   Created February 14, 1992

This document contains a description of the modules and programs that have changed 
for Amiga OS V39. For a low-level function specification please refer to the Autodocs.

Overview

Graphics
	o	support new AA modes/features
	o	double buffering
	o	Retarget sprite and screen/viewport functions
	o	Palette sharing 
	o	Bitmap functions
	o 	Interleaved BitMaps
	o	Modepromotion, coercion and selection
	o	RTG compatible RastPort functions.

Intuition
	o	Support new AA modes
	o	Double buffering
	o	Support AA graphics functions
	o	Default Busy pointer
	o	support interleaved bitmaps
	o	Attached screens (incl. menu lending)
	o	promotion
	o	support new sprites
	o	Gadget help
	o	Tablet support
	o	NewLook menus
	o	Special gadget relativity and GM_LAYOUT method

GadTools
	o	Palette enhancement
	o	ListView enhancement
	o	GT_GetGadgetAttrs() function
	o	Support for Intuition's NewLook menus and Gadget Help.
	o	Support for gadgets in requesters
	o	All-new PALETTE_KIND gadget
	o	Many small enhancements

BootMenu & Battmem
	o	PAL/NTSC switch
	o	Chip select switch

Layers
	o	Use new optimized layers
	o	Layerinfo backfill hook
	o	ClipRect traversal

Workbench
	o	Use ILBM as backdrop pattern
	o	WB config notification
	o	Path of file in 'Information'
	
Console
	o	fix console not to lock layers for long
	o	Use Mask bit to speed up rendering.
	o 	Misc. enhancements and fixes.

Clipboard
	o	fix r/w bug with units above unit 0

Filesystem
	o	New filesystem with directory blocks.

Exec:
	o	System specific changes
	o	Memory pools
	o	Memory handler
	o	Procure/Vacate
	o	Timed out alerts
	o	SAD

CIA resource
	o	bump priority to 120

DOS
	o	Rearrange strings.

Preferences
	o	New Iprefs
	o	New WBPattern editor 
	o	New Pointer editor to support AA.
	o	New Palette editor with pen assignments (using the Colorwheel class).
	o	Icontrol changes (promotion)
	
WB Disks
	o	MultiView
	o	Localization of selected programs
		- DiskSalv
		- PrepCard
		- MultiView
		- misc. new strings in preferences programs
		- non-concatenated DOS strings.
	o	DiskSalv. Disk-repair program
	o	ColorWheel Class: Set of boopsi classes implementing colorwheel 
colorselection.
	o	New c: commands to use ASL-file requester and easy-requester from 
scripts .

Appendix A

	New Workbench-disk programs and libraries and their location on the diskset.


Detailed Specifications

The following sections describe in more detail the changes and new features in 
AmigaOS V39


Graphics Specification:

Philosophy:

The original amiga graphics library exposed many device-dependent details to the 
application programmer. Because of this, introduction of new graphics devices has 
been slowed, and application support fo new features has been delayed. To reverse 
this trend, no features have been added to the new graphics system which cannot be 
kept for the future. Thus, when newer graphics systems are introduced, system 
software will need fewer changes, and applications will be ready to automatically use 
new capabilities.  Thus, parameters such as number of bits per pixel, resolution, color 
palette size, etc are either variable or have been made very large.

Low level functionality
 
Graphics copper list generation must be extended to handle loading of  >6 bit plane 
pointers, loading of extended color registers, handling of fetch and scrolling for 32 and 
64 bit fetch modes, promotion of interlaced modes to 31khz ("flicker-fixing"), promotion 
of non-interlaced modes via scan-doubling, and color palette banking.
    
Compatibility

The AA chips are register level compatible with the old and ECS chips at boot up time. 
However, when new AA modes are enabled and displayed, and a game then takes over 
the screen display without informing the OS of it, some registers may be in incompatible 
settings. Two approaches will reduce this:

	(a) old Programs which boot with their own custom boot block (games) will have 
AA features disabled unless they are specifically asked for. This should ensure 
transparent compatibility for all bootable games.

	(b) It will be possible to disable AA features for non-compatible programs. This 
will be done via the "BootMenu" which is available at system boot time. new 
options in this menu will allow disabling of AA, disabling of ECS, and switching of 
PAL and NTSC.

Get/Set functions

This release of graphics.library will attempt to ease the (future) transition to 
Retargetable Graphics. New functions are provided to do some operations in a more 
device-independent manner. This will help when we have to support foreign graphics 
devices, more than 8 bits per pixel, chunky pixels, and true-color displays.

This new calls of "Get/Set" function will allow device independent access to fields in the 
RastPort structure which were only previosuly manipulable by direct structure access.

Color Map functions

The color palette in AA is different in a lot of ways from the ECS one:

o	It has 24 bits per entry, plus one bit to select transparency.
	
o	There are 256 entries which is enough for many programs running on the same 
screen to share the palette.

The new color palette functions allow for multiple applications to coordinate their access 
to the palette. This allows applications to, for instance, dynamically remap pictures to 
match the color palette of the workbench screen, display animations in windows, etc.

All colors are now specified as 32 bit left-justified fractions, and are trunacted based 
upon the number of bits that the hardware is capable of displaying.
 
 
Bitmap functions

These function exist because the new AA chips have alignment restrictions in high 
bandwidth modes. Changing InitBitMap and AllocRaster to obey these restrictions 
appears to be very incompatible. Bitmaps created by AllocRaster with a multiple of 32 or 
64 pixels per line will be compatible with high fetch modes (2x or 4x respectively). 
Incompatible ones will fall back to 1x mode, if 1x mode is capable of displaying the 
screen.

AllocBitMap allocated an entire bitmap structure, and the display memory for it. 
AllocBitMap allows you to use more than 8 planes, and also allows you to specify 
another bitmap pointer, thus telling the system to allocate the bitmap to be "like" another 
bitmap. A bitmap allocated in such a matter may be able to blit to this bitmap faster. 
Such a bitmap may be stored in a foreign device's local memory. Do not assume 
anything about the structure of a bitmap allocated in this manner. The size of a bitmap 
structure is subject to change in future graphics releases. Thus, you should use 
AllocBitMap/FreeBitMap for your raster allocation. 

Sprite functions
 
Graphics sprite functions (MoveSprite) will be extended to understand large sprites, 
selectable sprite pixel resolution, and movement of scan-doubled sprites. Sprite 
positioning will no longer be rounded down to lo-res pixel resolution. Applications will no 
longer have to "know" about the hardware-dependent format of sprite data.
	

Miscellaneous

Some new features have been added, in order to:

	o	Improve the programming model.	
	o	Provide new capabilities to applications
	o	More fully take advantage of AA.

Interleaved screens have been added. These use a different layout of the graphics data 
in order to speed rendering operations and eliminate the annoying "color-flash" problem 
which is the hallmark of planar (as opposed to "chunky") display architetctures.

Double buffering functions have been added. These allow applications to display very 
smooth, synchronized animation in fully an efficient "intuition-friendly" manner.

Support has been added for AA border blanking, sprites in borders, and color palette 
banking. See also the ScrollRasterBF() function.

Some operations have been sped up: RectFill() has been rewritten, WritePixel() uses 
the CPU (3x speedup) and other optimizations.

Some bugs have been fixed.


Intuition Specification

New Graphics Modes

Intuition directly supports the new graphics modes.

New Sprite Features

Intuition supports the new sprite-resolutions and sizes.  This gives the user a fine-grain 
pointer, which looks a lot better.  As well, Intuition goes to great lengths to remain 
compatible with users of other sprites.

Intuition now has a default busy pointer.

NewLook Menus and TitleBar

By adding three additional pens to the DrawInfo pen-array, Intuition now allows control 
over the colors used in rendering menus.  This will allow the black on white menus with 
black trim that the Style Guide recommends.  As well, the screen title bar is rendered to 
match.

In addition, the Amiga-key and checkmark symbols that appear in the menus are 
colored accordingly, and are scaled by default to the screen's font.  It is possible to use 
a custom font in a window's menus, and have the checkmark and Amiga-key symbols 
for that window scaled to that font.

It is trivial to write compatible applications that come up optimally under all conditions 
(including V37).

GadTools recognizes and supports NewLook Menus.

Double-Buffering in Screens

Intuition now supports double-buffering in screens, via the new double-buffering calls in 
graphics.library.  It is possible to have both gadgets and menus (in a borderless 
backdrop window) on a double-buffered screen.

Attached Screens

Intuition now supports the concept of attached screens.  The {SA_Attach,parent} tag is 
used for this purpose.  First open the parent screen.  Then open one or more child 
screens with this tag. Child screens can be dragged independently of each other and 
their parent (though never above their parent).  Pulling down the parent below its 
natural top causes the child screens to move as well.

Attached screens also depth-arrange as a group, whenever the user does the 
depth-arranging.  Programmatic depth arrangement of the parent does the same.  
Programmatic depth arrangement of a child moves it alone, but only with respect to the 
group. 

Gadget Help and Special Gadget Relativity

The gadget structure now has an additional rectangle, which is the bounding box.  The 
HelpControl() function can be used to turn on "Gadget Help" for one or more windows.  
When Gadget Help is on, passing the mouse over the bounding box of a gadget having 
the GMORE_GADGETHELP property causes an IDCMP_GADGETHELP message to 
be sent.  IDCMP_GADGETHELP messages are also sent when the mouse is not over 
the window, or is over the window but not over a help-reporting gadget.  There is a 
GM_HELPTEST method which boopsi gadgets can use to refine their sensitivity area or 
to delegate the test to member gadgets.

Also, GRELxxx gadgets now use the bounding box to erase themselves, so you can 
now have GREL gadgets whose imagery extends outside of the gadget select box.  As 
well, there is a GM_LAYOUT method which boopsi gadgets can use to do any arbitrary 
layout within their window.

New Tablet Support

Intuition defines a new subclass of input-event which should solve tablet handling quite 
nicely.  Tablet drivers now have an easy time knowing what they should be scaling to.  
In addition, it is now possible for applications to receive extended IntuiMessages that 
can contain such information as pressure, sub-pixel position, etc.  The extra information 
is extensible to cover things we cannot yet forsee.

Custom Image Disable-Rendering

For a disabled gadget under V37, Intuition always rendered the dot pattern itself.  Under 
V39, an image can set the IA_SupportsDisable attribute to TRUE, and then Intuition will 
let it do its own disabled rendering, through the IDS_DISABLED or the (new) 
IDS_SELECTEDDISABLED image drawing states.  This is transparently compatible 
with V37.

Promotion

Crudely speaking, promotion is de-interlacing in software.  IControl has a "promotion 
option".  This flags instructs Intuition to tell graphics to change the mapping of the 
default monitor.  Database-aware programs will now receive information about the 
double-NTSC or double-PAL mode when enquiring about the default monitor.  Of 
course, if the user changes the promotion setting after the application is open, not 
everything it learned from the database will still be true. The new method also means 
that an application can request to never be promoted by asking for explicit NTSC or 
explicit PAL, instead of the default monitor.

Pen-Sharing

Under V39, Graphics supports sharing pens (ObtainPen(), etc.). Intuition now uses and 
supports the graphics.library pen-sharing scheme.  For all screens, pens found in the 
DrawInfo->dri_Pens are obtained as sharable.  For existing public screens, all other 
pens are allocated as exclusive.  For custom screens and aware public screens (those 
that set the new SA_SharePens tag), all other pens are left unallocated.  On the 
Workbench screen, pens 0-3 are guaranteed to be sharable.

Interleaved Screens

As graphics.library now supports interleaved bitmaps, Intuition makes this feature 
available to screen openers via the SA_Interleaved tag. Currently, the Workbench is 
interleaved by default. 

Coercion and Inter-Screen-Gap

Intuition now relies on graphics.library to figure out coerced screen modes, and the 
required inter-screen-gap.  This allows greater inter-mode compatibility, better coercion 
results, and compatibility with future monitors.




Pens and DrawInfo

The new SA_LikeWorkbench tag gives you a screen just like the Workbench.  This was 
basically a consequence of getting pens settable through preferences while keeping 
NewLook menus on the Workbench screen.

When the SA_Pens pen-array is evaluated, pens are masked to the number of available 
colors.  As well, special definitions of pen-number (PEN_C3, PEN_C2, PEN_C1, and 
PEN_C0) mean the complementary pen numbers of pens 0-3.

Preferences now listens to only 8 colors for the bitmap, which Intuition will set as the 
first four and the last four colors of the Workbench or any "Workbench-like" screen 
(those having the SA_FullPalette or SA_LikeWorkbench attributes).

The way the DrawInfo pens are determined is Intuition picks a default pen-array.  Then, 
any pens you supply with SA_Pens override the defaults, up until the ~0 in your array.  
If the screen is monochrome or old-look, the default will be the standard two-color pens.  
If the screen is two or more planes deep, the default will be the standard four-color 
pens, which now include the new-look menu colors.

If the screen has the SA_LikeWorkbench property, the default will be the user's 
preferred pen-array, changeable through preferences.  There is a preferred pen-array 
for four colors, and one for eight or more colors.

Miscellaneous Window Features

If a window is not frontmost, but nevertheless is unobscured, then clicking on its depth 
gadget used to bring it to front, which had no visual effect.  It now depth-arranges 
correctly (i.e.  to back).

It is now possible to receive an IDCMP_CHANGEWINDOW message when a window is 
depth-arranged.

Size-only zooming (where the window top-left corner is preserved) is now supported.

Miscellaneous Screen Features

Screens may now be made non-draggable.  Screens can be designated as exclusive, 
meaning they will never share the display with another screen.  The SA_BackFill tag 
can be used to specify a backfill hook for the screen's LayerInfo.  Is it now possible to 
control the SA_Pens or other previously unavailable attributes of the Workbench 
screen. This will allow things such as light-on-dark text.  Screens can have interleaved 
bitmaps.  There are new universal screen movement and depth-arrangement functions.  
Intuition now supports scrollable 2024/Moniterm screens.

Screen depth is always validated.  Making a request for a screen too deep causes 
failure with OSERR_TOODEEP.


Miscellaneous

The DoGadgetMethodA() function allows you to invoke a method on a gadget, which 
will then receive a valid GadgetInfo pointer.

The boopsi image class "frameiclass" supports the standard border types (eg.  bevelled 
box, string-gadget ridge, "moat", AppWindow icon drop-box).

Intuition now notices and repairs damage when boopsi gadgets use ScrollRaster().  
(Such damage occurs when the gadget is in a simple-refresh window and part of the 
scrolled area is obscured). Such gadgets MUST set GMORE_SCROLLRASTER in 
order to benefit from this magic repair feature.  (Note that ScrollWindowRaster() is for
applications.  Boopsi gadgets ought to use ScrollRaster() or ScrollRasterBF()).


Speed Improvements

When a window opens active, its border is rendered only once, not twice.  This looks 
better and is faster.

EasyRequests and AutoRequests now put their imagery and gadgets directly in their 
window, instead of also using a requester.  This saves memory, and is faster.

The rubber-band frame when moving or dragging windows is now drawn faster.

Several other speed optimizations were taken.

Gadtools specification

For V39, work has been done to GadTools to address new Intuition features, to add 
some important features missing from V37 GadTools,   and to fix a few bugs.  The key 
changes are:

  	- A "get gadget attributes" function

  	- Support for Intuition's NewLook menus

  	- Support for Intuition's new Gadget Help feature

   	- Many enhancements to the different kinds of gadget, including a complete 
re-write of PALETTE_KIND, support for proportional fonts in the slider level 
display, as well as scalable checkmark and radio button imagery.


BootMenu & Battmem:

Bootmenu has been enhanced with support for selection of default video mode (PAL or 
NTSC) and selection of default chipset.

Bootmenu will switch between PAL and NTSC when a key on the keyboard is pressed 
(except certain special purpose keys like SHIFT, ALT etc).

Chipselection offers the following choices:

	- Original	[ the original pre ECS chipset]
	- ECS
	- Best

'Best' means that the system will boot into the 'AA' on a AA system, ECS on an ECS 
system etc. 

Setting can be saved to battmem or used without saving.  Support for saving to 
battmem may be missing from the first releases.

Layers specification:

The new optimized layers.library is used in V39. Layers has been further enhanced by 
adding ClipRect traversal and support for Layerinfo backfill hook

Workbench specification:

Workbench has been updated to support ILBM backdrop patterns. As a further 
enhancement 'Information' is now asynchronous and shows the complete filename
path.


Console specification

	- AA scrolling optimizations
	- AA change in EOR cursor logic
	- Fix CMD_CLEAR
	- Research a command to specify new defaults for ESC[0m.  This would allow 
programs which send ESC[0m not to change the users preferred defaults - a 
nice thing for many of our programs which run from the Shell, and insist on 
reseting my preferred SGR settings.  
	- Cursor & highlighting only use colors 0-3 in screens which support color sharing 
(e.g., WB).


Clipboard.device specification:

Fix units 1-255 multi R/W handling.

Filesystem specification

A new filesystem type (option) has been added: Directory Caching. This caches all the 
information normally returned by ExNext/ExAll in a series of blocks attached to each 
directory.  These blocks normally hold 15 entries; the actual number can range from 3 to 
20 for 512-byte blocks.

These blocks allow exnext/exall to do drastically less disk access. Roughly, this makes 
floppies almost as fast as harddisks under 2.04, and harddisks almost as fast as the 
ramdisk.  This is dramatically noticable to users, especially floppy-based users.

There is a slowdown in file-creation (about 30% on harddisks), some slowdown on 
delete and on close of a modified file due to the need to update these caches.  Also, 
validation, if needed, may take slightly longer since it must validate all the directory 
caches.  These caches do add to the ability of disk-repair utilities to rebuild the directory 
structure of a trashed disk, though, or rebuild trashed fileheader blocks.

The code cost is about 2-2.5K.  Also, in the process of doing these modifications, 
several nasty holes from 2.04 (and before) were fixed.  These include the 
multiple-files-of-the-same-name bug, and a bug not seen experimentally where a 
rename of a file and a delete of the previous file in that hash chain could collide, losing 
entire strings of files (or in rename versus rename, entire strings of files could be moved 
to the wrong directory). All modification of directory structures is now locked behind 
coroutine semaphores, so no holes like this should show up again.

Various other bugfixes are being done as required.

CIA reource specification:

Bump priority to 120

DOS specification:

Rearrange strings to avoid using concatenation.

Preferences specification:

Prefs/Pointer

Changes are required for AA sprite support. This includes larger sprites with varying 
resolutions and number of colors. Another needed change is to support editing of the 
default busy pointer.

Prefs/Palette

With 3.0's pen sharing abilities, a different more abstract color model must be available 
to users. Palette prefs will gain a pen editing ability which will let the user associate 
arbitrary RGB values to any of Intuition's logical pens. So for example, the user will be 
able to specify which RGB value should be used for his text, background, window 
border, etc...

Prefs/WBPattern

Must learn to deal with more complex patterns. This will be done in a manner similar to 
the current Sound preferences in that a user can just enter the name of a picture to 
display as Workbench pattern.

Prefs/IControl

A few checkboxes need to be added to enable control of Promotion.

C/IPrefs

Needs to adapt to the new extended Workbench pattern ability. It must load an ILBM file 
and pass the result on to Workbench. Operations can be done on the graphics before 
displaying it such as color remapping.

Needs to adapt to the new way Workbench expects to hear about changes to Font and 
Locale preferences.


WB disk (tools, utilities etc. ) specification

Tools/DiskSalv

New program to repair and recover hard drives.

Utilities/MultiView

This is a object-oriented data viewer that uses the datatypes.library for its object 
handling.  Any type of data file can be viewed using this viewer, as long as class for the 
data type is available.

Objects can be viewed on the Workbench screen, any public screen, or a custom 
screen (for HAM pictures for example).

Classes that will be shipped will be:

  ANSI - 	Used to view any text document that Utilities/More could have viewed, 
including decoding of ANSI escape sequences.

  ILBM - 	Used to view Amiga ILBM pictures.  Includes the ability to remap a picture 
to the current palette (uses new graphic functions to obtain pens, plus does 
simple dithering).

  AmigaGuide - Used to browse through AmigaGuide documents.  This is backwards 
compatible with the existing AmigaGuide.  There have been a number of small 
enhancements, such as; proportional font support, word wrap, and text attributes 
(bold, italic, and underline).

Storage/Monitors

New monitors for AA support

Localization

Localize the following applications:
	- DiskSalv
	- PrepCard
	- MultiView
	- misc new strings in updated preferences programs.
	- new non-concatenated DOS strings.
	
Tools/Commodities/MouseBlanker

A new mouse blanking commodity that shuts off the mouse pointer whenever the user 
types on the keyboard.

Tools/Commodities/NumericPad

A new commodity emulating numeric keypad. Mainly intended for use on A600 class 
systems without numeric keypad.


Appendix A

Workbench 3.0 Disk-changes (layout)

program				Change

C/#?					- Misc. bug fixes
C/AddDataTypes			- New
C/IPrefs				- pointer, palette, and wbpattern changes
C/RequestChoice			- New
C/RequestFile			- New
Classes/amigaguide.class 		- New
Classes/ascii.class			- New
Classes/colorwhell.class		- New
Classes/floatstring.class		- New
Classes/ilbm.class			- New
Classes/picture.class		- New
Classes/text.class			- New
Devs/DataTypes/AmigaGuide	- New
Devs/DataTypes/FTXT		- New
Devs/DataTypes/ILBM		- New
Devs/printer.device			- HAM-8 support
L/FastFileSystem			- DCFS version
Libs/amigaguide.library		- New
Libs/datatypes.library		- New
Libs/iffparse.library			- Optimization
Locale/Catalogs/#?			- Updated translations
Prefs/Palette				- Color whell
Prefs/Pointer				- AA support
Prefs/WBPattern			- WB backdrop support
Storage/Monitors/A2024		- 3.0 ROM support
Storage/Monitors/DoubleNTSC	- New
Storage/Monitors/DoublePAL	- New
System/Format			- DCFS support
Tools/Commodities/MouseBlanker- New
Tools/Commodities/NumericPad	- New
Tools/DiskSalv			- New
Tools/HDToolBox			- UI overhaul
Utilities/More				- Make it look like MultiView from the WB
Utilities/MultiView			- New






