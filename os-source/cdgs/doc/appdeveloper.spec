
                     Tips For Developing An AmigaCD Title
                     ------------------------------------

Last Revised: $Date: 93/05/07 11:10:13 $ $Revision: 40.7 $


This document presents many issues relevant to developers creating titles for
the AmigaCD console. Additional documentation can be found in the autodocs and
include files for the standard Amiga system software, and AmigaCD-specific
extensions.


CDTV Compatibility
------------------

Although the AmigaCD console can run most CDTV titles, we recommend you do not
use any of the old CDTV-specific system modules. These modules were only
slightly modified in order to make existing CDTV titles work on AmigaCD. There
are some known bugs and limitations in these modules when dealing with the new
hardware of the AmigaCD.

Another good reason to avoid using CDTV-specific system modules is that in the
future, our intent is to support running AmigaCD titles from a properly
configured Amiga computer. On these systems, CDTV-specific modules will not
exist, and therefore any titles relying on them will not function.

These are the CDTV-specific modules you should avoid using in any AmigaCD title:

    - cdtv.device

    - playerprefs.library

    - bookmark.device

    - cardmark.device

    - debox.library

    - rmtm

    - CDTVPrefs

Five special system modules were created for the AmigaCD console:

    - lowlevel.library

    - nonvolatile.library

    - freeanim.library

    - cd.device

    - CD-ROM file system

You can use these modules with confidence. lowlevel.library,
nonvolatile.library and the CD-ROM file system will be part of Workbench 3.1
and will be shipped within the 3.1 Enhancer kits and with new machines that
include Workbench 3.1. freeanim.library will not be included with the normal
Workbench distribution, but your code does not need to rely on its presence
(see below for more on this). Finally, cd.device will be included with any
AmigaCD-compatible solution that Commodore would potentially offer for its
computer systems.


Trademark File
--------------

In order to get a title to boot on an AmigaCD, an AmigaCD trademark file must
be included directly after the PVD sectors. CDTV supported having the trademark
file either right after the PVD sectors, or as a normal file in the root of the
CD directory structure. This is not the case with AmigaCD, the trademark file
must be included after the PVD sectors.

The AmigaCD trademark file differs from the CDTV one. This is how AmigaCD
determines if a title is a CDTV title of an AmigaCD title. Contact CATS for
information on obtaining the trademark file if you plan on generating your own
ISO images. In order to ensure the trademark file gets placed directly after
the PVD sectors, do not simply copy it to your CD's directory. Instead make
sure that the trademark file is in the directory you are currently CD'd to
(probably where ISOCD is located) when creating your ISO image.

Note that use of the trademark file requires a CD License Agreement with
Commodore. Contact CATS for licensing information.

The type of trademark file on a CD determines the environment in which the
title is started. If a CDTV trademark file is found, the title is booted in ECS
mode. If an AmigaCD trademark is found, the title is booted directly in AA
mode. In addition, many system kludges are applied on a per-title basis when
booting CDTV titles in order to keep them working on the AmigaCD.


Booting A Title
---------------

AmigaCD boots just like a normal Amiga. The only available boot device is
normally the CD (CD0:), so the system boots off the CD in the same manner it
would boot from a hard drive, executing a startup-sequence et al.

Competing game consoles generally have instantaneous startup times for titles,
due to the cartridge interface used. AmigaCD benefits in many ways from its
CD-ROM technology, booting time is not one of these benefits. Due to the very
slow seek times of CD-ROM devices, the dynamics of disk IO is quite a bit
different from a hard drive. It is crucial that developers be aware of this
situation and take appropriate actions to reduce boot times.

The startup animation sequence provided by the AmigaCD ROM has some continuous
motion to indicate that the system is doing something while a title is booting.
This animation sequence requires little memory and gets out of the way as soon
as the title is ready to run. This animation helps eliminate the long periods
of black screen visible when booting a CDTV.

An important factor in bootup time is the length of the startup-sequence
executed by the system prior to running a title. The startup-sequence shipped
with standard Amiga systems is intended for use in a multitasking computer
environment. Understanding the startup-sequence, and optimizing it can help
reduce bootup time considerably.

The following is a startup-sequence which is adequate for most titles. It is
much smaller and many times faster than the standard startup-sequence provided
with Workbench 3.1. It also leaves noticeably more memory for your title than
the standard startup-sequence.

*******

C:SetPatch QUIET

; Only include this line if you are using locale.library
C:Assign >NIL: LOCALE: SYS:Locale

; here you can add any assignments that are specific to your title.
C:Assign >NIL: WestChesterWarrior: SYS:WestChesterWarrior

; Only include these lines if you include the corresponding monitor files
; on your disc. Only include the files on the disc if you actually use the
; modes these monitors provide. Each monitor included eats up memory
; and takes time to load and run.
DEVS:Monitors/NTSC
DEVS:Monitors/PAL
DEVS:Monitors/DblNTSC
DEVS:Monitors/DblPAL

; here you just start your title
SYS:WestChesterWarrior

*******

Note that using explicit path specifications (C:Assign instead of just Assign)
avoids a lot of head seeking and can make a noticeably performance difference.

Note that use of Workbench files other than plain text files such as
startup-sequence on a CD requires a Workbench License Agreement with Commodore.
Contact CATS for licensing information.

Another important consideration in bootup time is the size and complexity of
the title itself. It is highly desirable to get your title on the screen as
soon as possible. This can become difficult with very large titles which take a
long time to load in memory. The solution to this problem involves either the
use of overlays, or the use of a separate loader program.

The simpler approach is the loader program. This is a very small program that
gets run in the startup-sequence (just like WestChesterWarrior in the example
above). This loader program contains some animation and music sequences that
can quickly be put onto the screen. While this animation is playing, the loader
program can then proceed to load in the rest of the title into memory and
activate it. See the DOS RunCommand() or System() functions for information on
how the loader program can invoke the main title.


Startup Environment
-------------------

While the system is loading a title into memory, there is a small animation
playing on the screen. This animation tells the user that the system is busy
and will be done shortly. This animation consumes little memory, and exits the
system as soon as the title indicates it is ready to run.

There are several things to consider when initializing a title with respect to
interactions with the system:

  - The startup animation runs in an Intuition screen. In order for the exit
    sequence of the animation to work correctly, titles must not take over
    the display prior to the animation shutting down.

  - The startup animation may use the audio.device. It is therefore important
    that titles do not start to play any music prior to the animation
    shutting down.

  - When the animation exits the system, it leaves the display completely
    black. You can assume this in your titles, and fade up from black, or
    whatever other transitions you wish to do.

Once your title is in memory and wishes to display something on the screen,
it needs to do:

    FreeAnimBase = OpenLibrary("freeanim.library", 0);

This tells the startup animation to begin shutting down, which involves
removing itself from the display and freeing the resources it uses. This may
take awhile, up to about a second. While the startup animation is shutting
down, your application can prepare data to display and generally initialize
itself. This parallelism reduces the time the user has to wait for the titles
to start.

Once you have done what you need in order to display your title screen, you
must do the following:

    CloseLibrary(FreeAnimBase);

This waits for the animation to complete its shutdown. Once this function has
returned, it is now safe to display your title screen.

If you have no need to process information while the animation is shutting
down, the following will work:

    CloseLibrary(OpenLibrary("freeanim.library", 0));

It is important to note that on a computer equipped CD-ROM, there will not be a
freeanim.library around, so your code should not fail when freeanim.library
cannot be opened. This is easy to do, just don't put any error checking in:

    FreeAnimBase = OpenLibrary("freeanim.library",0);

    /* initialize application stuff */

    CloseLibrary(FreeAnimBase);

    /* start application animation/display */

CloseLibrary() accepts a NULL pointer and ignores it. Therefore if
OpenLibrary() fails and returns NULL, CloseLibrary() will still work.


Running Your Titles From Workbench
----------------------------------

If Commodore decides to make AmigaCD compatible hardware available for its
Amiga computer systems, it would be highly desirable if AmigaCD titles could be
run from Workbench on those systems. This requires only a few lines of code to
support:

  1 - You should handle disc insertion and removal events. On a computer
      system, it would not be wise to enable the "reboot on removal"
      feature of cd.device. When running on an AmigaCD, you should
      generally reboot when a disc is removed, but in a computer environment,
      you should deal with this eventuality and handle it gracefully by
      putting up an error message, or at least quiting the title. Crashing
      is never a good idea.

  2 - Your code should handle the Workbench startup environment

  3 - Your CD should include icons for the appropriate files

  4 - Your code should be able to run without having the startup-sequence
      from your CD run. This is important. If your program relies on
      assignments made in the startup-sequence, these will not be present
      when running from Workbench. An easy solution is to provide an
      IconX script on the CD that gets run from WB and loads in the title.
      The script can do the needed assigns.

  5 - Your code should multitask nicely. It should release all its
      resources on exit. If this is not possible, you must warn the
      user in the documentation and in the software itself that starting
      this title will take over the system and cause any unsaved work
      to be lost.

The startup environment in a computer system will different from the normal
AmigaCD environment. freeanim.library will not be there, so your code should
not fail if freeanim.library cannot be opened. In addition, there will not be a
startup animation in the system, so the initial display will not be a black
screen and will instead be the user's Workbench screen.

It is also a nice touch to make the software runnable when not on the CD. This
allows users to copy the title to their hard drive, enabling potentially much
faster operation.

Finally, if you wish your application to look right at home on a CD-ROM
equipped Amiga computer, you should avoid direct references to "AmigaCD" within
your title. The title might actually be running on an A1200 or A4000.


Preferences
-----------

CDs should not contain any preferences files. This specifically means that the
following files should not be on your CDs:

   Devs/system-configuration
   Prefs/env-archive/sys/#?

Including any preferences file can cause odd behavior during the startup
animation sequence.

In addition, many CDTV titles used to call the Intuition SetPrefs() function in
order to change screen positioning and centering. This is not the correct
method and will lead to problems with the startup animation. See the section
below on screen positioning for information on how to do it right.


Screen Positioning
------------------

In the days of 1.3, there was originally no officially supported way to control
overscan and only limited official screen positioning control. Numerous ad hoc
techniques of varying quality were invented, and most are still supported,
though some only work because of kludges designed to keep them operational.

V36 introduced the graphics database and a host of structures to help you
control your screen position.  For correct operation on the AmigaCD, we
strongly urge you to use these functions and structures.

First, some definitions to help you along:

Text Overscan:  this is the old "morerows" size from 1.3, and has been called
the "Text Overscan" or "Text Size" in V37+ Overscan Prefs. This rectangle
defaults to a nominal-sized rectangle, but can be enlarged and/or repositioned
by the user via Overscan Prefs. If this rectangle is correctly set, no pixel in
Text Overscan is supposed to be masked by the bezel of the monitor.

Display Clip:  a rectangle that describes the desired displayable portion of
your screen. The coordinates are in screen-pixels, and the origin is the
upper-left corner of the Text Overscan rectangle.

It is best to center your own display with respect to the Text Overscan
rectangle. Here's a piece of code to handle it:

/* CenterDClip( displayID, width, height, rect )
 * Fills in the struct Rectangle pointed to by rect with a DisplayClip
 * whose size is the width and height specified, and whose position
 * is centered around the Text Overscan rectangle for the specified
 * displayID.
 *
 * Returns TRUE if it succeeds, FALSE if it doesn't (usually caused
 * by an invalid displayID).
 */

BOOL CenterDClip(ULONG displayID, ULONG width, ULONG height,
                 struct Rectangle *rect )
{
BOOL success = FALSE;
LONG excess_x;
LONG excess_y;

    if ( QueryOverscan( displayID, rect, OSCAN_TEXT ) )
    {
    excess_x = ( width - ( rect->MaxX - rect->MinX + 1 ) );
    excess_y = ( height - ( rect->MaxY - rect->MinY + 1 ) );
    rect->MinX -= excess_x / 2;
    rect->MaxX = rect->MinX + width - 1;
    rect->MinY -= excess_y / 2;
    rect->MaxY = rect->MinY + height - 1;

    success = TRUE;
    }
    return( success );
}

Sample use:

    #define MY_DISPLAY_ID   HIRES_KEY
    #define MY_WIDTH    whatever
    #define MY_HEIGHT   whatever

    ...

    struct Rectangle dclip;

    if ( CenterDClip( MY_DISPLAY_ID, MY_WIDTH, MY_HEIGHT, &dclip ) )
    {
        if ( myscreen = OpenScreenTags( NULL,
        SA_DClip, &dclip,
        ...
        TAG_DONE ) )
    }

Centering yourself using CenterDClip() could theoretically give you a
Display Clip which extends outside of the OSCAN_MAX Display Clip,
which represents the hardware limits.  If you would rather be slightly
off-center than clipped, you may wish to shift the resulting DClip
to fit within Maximum Overscan.  The easiest way to get that rectangle
is:
        QueryOverscan( displayID, &maxrect, OSCAN_MAX );

A word on screen size/position defaults:

The default values for the LeftEdge, TopEdge, Width, and Height of a screen can
be a bit complex. This is partially due to the fact that they are implicitly
specified by supplying a non-NULL NewScreen pointer to OpenScreenTagList(), and
by the fact that while Intuition does define STDSCREENWIDTH and STDSCREENHEIGHT
magic constants, there are no corresponding constants for standard left-edge
and standard top-edge (which are likely non-zero!). The most foolproof strategy
is to use a NULL NewScreen structure, using only tags to specify screen
attributes, and omit all of SA_Left, SA_Top, SA_Width, and SA_Height. The
correct defaults will come from the SA_DClip.

The other way (which is required if using a non-NULL NewScreen), is to supply
the correct values for each component (either in the appropriate members of the
NewScreen structure or with the appropriate tags). The correct values are:

    left   = dclip.MinX
    top    = dclip.MinY
    width  = dclip.MaxX - dclip.MinX + 1
    height = dclip.MaxY - dclip.MinY + 1

Note that if your screen is shorter than the nominal height, other screens that
are behind you can potentially be visible above your screen.

You are strongly encouraged to use Intuition screens instead of custom Views
and ViewPorts. However, if you go that route, you can install a DisplayClip
into your ViewPort's ViewPortExtra structure.

A word on sprites:

User preference can shift the Intuition View origin, which in turn can affect
the number of sprites available on your screen. If you use sprites other than
the pointer sprite, you must take this into consideration, because a change in
the upper-left corner of the Text Overscan rectangle shifts the entire
coordinate system, so even carefully-chosen fixed coordinates may get you into
trouble. The upper-left corner of the OSCAN_MAX rectangle is the one which is
at a fixed position with respect to the hardware fetch cycle (for a given
mode). You can base a positional calculation on this if necessary.

There are some Display Clip positions for which the number of possible sprites
varies depending on which fetch mode is used. The higher fetch modes have the
advantage of reducing chip RAM bus saturation, but you may prefer a lower
bandwidth with more sprites.  Graphics will drop the bandwidth to bring
allocated sprites into view if MakeVPort() runs after you obtain your sprites
via GetExtSprite(). In an Intuition environment, you should use RemakeDisplay()
after you get your sprites (not call MakeVPort() directly).


Avoiding The Workbench Screen
-----------------------------

If you use Intuition screens in your titles, it is important to note an
Intuition "feature" which can get in the way. Whenever the last screen opened
in the system is closed, Intuition automatically opens up the Workbench screen.
This can be quite cumbersome in titles which perform transitions between
different screens. Here is a short code sample for SAS/C that defeats this
Intuition feature. Simply call CloseScreenQuiet() instead of calling
CloseScreen() and all will be fine:

ULONG __asm OpenWorkBenchPatch(void)
{
    return(0);
}

VOID CloseScreenQuiet(struct Screen *screen)
{
APTR OWB;

    if (screen)
    {
        OWB = SetFunction(IntuitionBase, -0xd2, (APTR)OpenWorkBenchPatch);
        CloseScreen(screen);
        SetFunction(IntuitionBase, -0xd2, OWB);
    }
}

Note that the above code will only work starting with V39 Kickstart. Previous
Kickstarts did not support this. The AmigaCD has a V40 Kickstart.


V40 Graphics Issues
-------------------

The graphics.library for V39 gained many features that make creating system
friendly, fast moving graphics and animations much easier. graphics.doc is a
good source of information on these new features. Some examples are also
available from the 1993 DevCon notes. Of specific interest is the new
double-buffering support which is even available in Intuition screens.

V40 of graphics.library which is present in the AmigaCD ROM contains a few
extra features specifically targeted at improving the performance of animation
and rendering code. These features are relatively new and not well known, so
they are briefly explained below.

  - ViewPorts that don't load colors.
    ViewPorts normally always have a copper list associated with them which
    loads up all the colors needed for their depth. This new feature makes
    it so that only color 0 gets loaded within the copper list. For deep
    screens, this will substantially reduce the copper list length.
    The rest of the colors are inheritied from the viewport above.
    This feature is useful to reduce the inter-viewport gap, and for faster
    color cycling. See the VC_NoColorPaletteLoad tag in
    <graphics/videocontrol.h> and in the graphics.library/VideoControl()
    autodoc entry.

  - Preventing intermediate copper list updating.
    Speeds up LoadRGB(), SetRGB(), ScrollVPort(), and ChangeVPBitMap().
    See the VC_IntermediateCLUpdate tag in <graphics/videocontrol.h> and
    in the graphics.library/VideoControl() autodoc entry.

  - Dual-playfield disable.
    Disables setting of the dual playfield bit in dual-playfield viewports.
    Odd and even bit-planes will still scroll separately, but will be fed
    directly to the palette. This can be used for instance to do cross-fades
    between independently scrolling images. See the VC_DUALPF_Disable tag in
    <graphics/videocontrol.h> and the graphics.library/VideoControl() autodoc
    entry.

  - AmigaCD includes very fast chunky-to-planar conversion hardware. This
    allows for high-quality 3D graphics. Consult the documentation for the
    WriteChunkyPixels() function in the graphics.doc autodoc file for more
    information.

Note that the above new ViewPort features work fine in Intuition screens as
well. You just need to pass the screen's ViewPort as a parameter to the
VideoControl() function.

You might also wish to look at the documentation for the SA_VideoControl and
SA_MinimizeISG tags for the OpenScreenTagList() function. These two tags offers
Intuition-level control of the new features outlined above. There are also the
SA_Interleaved, SA_Exclusive, and SA_Draggable tags which can greatly help
writing animation and graphics code. All these tags are documented in
<intuition/screens.h> and in the intuition.doc autodoc.

V40 also added four new display modes which are useful for animation support.
These modes are defined in <graphics/modeid.h> as:

/* Added for V40 - may be useful modes for some games or animations. */
#define LORESSDBL_KEY                   0x00000008
#define LORESHAMSDBL_KEY                0x00000808
#define LORESEHBSDBL_KEY                0x00000088
#define HIRESHAMSDBL_KEY                0x00008808

These new V40 display modes allow you to display a given raster at twice its
height. Each line of the raster automatically gets repeated on the screen as
it is displayed. This allows a 128 pixel high raster to be displayed as if
it was 256 pixels tall. This can be very helpful in order to create full
screen animations. For example, a 128 pixel high CDXL animation can
suddenly occupy the full screen.

When you use one of these new modes, you DO NOT allocate more raster or open a
taller screen - i.e. you still allocate a raster of the smaller height (128 for
example), and if you are blitting the image you still only blit 128 lines. The
effect which doubles the height of your image is a visual video effect
accomplished by the AA hardware.

These modes work on a composite monitor or TV.


Mode Promotion
--------------

Mode promotion is a feature which converts 15kHz screens into 31kHz screens,
in order to remove interlace flicker, or inter-line gaps. It is the AA
equivalent of the A3000 display enhancer hardware.

Mode promotion can be turned on or off by the user. When turned on,
screens opened using the 1.3 OpenScreen() function, or opened using a
default monitor ID will end up promoted to a 31kHz mode. The promotion
process is not totally transparent as certain aspects of the copper list,
the refresh rate, and available overscan dimensions are different
for the promoted state than for the unpromoted state. These differences
can be crucial for many developers.

To avoid promotion affecting your titles, you simply need to open your screens
with explicit mode IDs using the PAL_MONITOR_ID and NTSC_MONITOR_ID, instead
of using the DEFAULT_MONITOR_ID.

Note that promotion is not turned on in AmigaCD systems, the problem can only
surface in a computer environment.

Consult the ROM Kernel manuals, the 1991 and 1993 DevCon notes for more
information on mode ids and mode promotion.


Localization
------------

There are two ways software can be localized on AmigaCD systems.
lowlevel.library offers a very simple mechanism that returns the number of the
user's current preferred language. The second method involves using
locale.library which provides a complete suite of localization services.

If your title does not use locale.library, it is best to remove it from your
CDs.

If your title does use locale.library, you must have a directory structure such
as:

      Locale (dir)
        Catalogs (dir)
          français (dir)
            westchesterwarrior.catalog
          deutsch (dir)
            westchesterwarrior.catalog
          italiano (dir)
            westchesterwarrior.catalog
        Languages (dir)
          français.language           italiano.language
          deutsch.language

That is, you need a Locale directory which contains a Catalogs and a Languages
subdirectory. There is no need to include the Countries directory that comes
with the Workbench disk, this directory is only of use to the Locale
preferences editor.

The Catalogs directory must contain one subdirectory for each language that
your title supports. Within these directories, you can put the standard
locale.library catalog files (see locale.doc and catcomp.doc for more
information).

The Languages directory must contain a language driver for each of the
languages your title supports. These language drivers are supplied with
Workbench 3.1 on the Locale disk.

The LOCALE: assignment must exist if you use locale.library. It should be made
to the Locale directory described above.


nonvolatile.library
-------------------

The purpose of this library is to allow an application to save small amounts of
information to nonvolatile memory or a disk device without having to burden the
application with determining which device to use. It provides a simple, common
access method to whichever device the user chooses. This library also provides
access to true nonvolatile memory present on the game machine.

See the nonvolatile.doc autodoc for more information on this library.


lowlevel.library
----------------

lowlevel.library provides many functions useful for game developers. Among
other things, this library provides the only means to read the AmigaCD
game controller.

Aside from the game controller code, the main purpose for lowlevel.library was
to provide functions that are often needed by game developers. Most of the
functions in this library should not be used in a multitasking environment and
is intended purely for simple programs. Aside from the game controller code,
everything else in lowlevel.library can be better done using other system
modules, though more work may be involved.

See the lowlevel.doc autodoc for more information on this library.
