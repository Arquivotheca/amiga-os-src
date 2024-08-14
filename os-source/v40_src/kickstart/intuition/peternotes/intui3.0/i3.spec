---------------------------------------------------------------------------

string gadget always-gadget-up
MakeVP failure API implications

DEFER?
DrameImageFrame() and IA_FramePlease?
GetGadgetAttrsA(): have singular GetGadgetAttr via DoGadgetMethodA()
Promotion:
	Unknown modes (monitor not added)
	Flicker-fix: what to do with non-lace DPF, super-hires
	State that monitor does not support default
	Promotion override
	Gfx database flag meaning "this mode is undesirable", that would
	be true for default mode with a FST, say.

DONE:
Gadget special relativity resizing stuff
Tablet
pointer stuff
Menu-lending
	- But has bugs
Double-buff:
	Need gfx signal/notification
DoGadgetMethod
	- What about glist locking?  What about ISM?
High pens ( UI / Intuition )
	- What about high pens elsewhere than the WB?
Gadget help
Gadget-damage based on bounds

POST-BETA:
Attached screen to begin vert. autoscroll at the top of the topmost
child!
ScrollWindowRaster() issues
Sprite Pens (gfx interface - done)
(can this wait?) SA_Interleaved and SA_Pens ->prefs
Public access to zooming
VPOrigin() and auto-top/left
Underscoring LABELIMAGE, itexticlass

OPEN SPRITE ISSUES (*=solved, D=post-beta)
 - test hedley sprites
 - per-sprite pens
 - per-sprite bank
 - RemakeDisplay() doesn't rethink sprite
 - Mysterious timing-related crash
*- pointer.h and .i
*- ECS and 2x machine issues
*- Intended sprite word-width
*- IPrefs busy-pointer
D- forced scaling down to affect sprite res

DENIED
erase gadget method/call

---------------------------------------------------------------------------



(1) New Look Menus and Screen Bar		10d	 DOC   DONE
(1) AA Misc Support Issues			5d	 DOC
(2) AA Sprite support				5d	(doc)
(2) AA Palette/color support			2d	 DOC
(3) Mode Promotion					      (done)
(3) Attached Screens				3d	 DOC   DONE
(3) Intuition's use of AllocRemember()		2d	 DOC   DONE
(3) Fix depth-gadget for fully-visible windows	.5d	 DOC   DONE
(4) IDS_DISABLED Support for gadget images	3d	 DOC   DONE
(4) System support for Double-Buffering		3d	 DOC  (done)
(5) Intuition-friendly Screen-locking		3d	 DOC
(5) Sub-menu delay				1d	 DOC  (done)
(6) Embed WindowPort into XWindow structure	.5d	 DOC   DONE

(4) Enhanced Tablet Support			5d	(doc) DEFERRED
(4) Non-activatable ToolBox windows		2d	(doc) DEFERRED
(5) Screen-depth notification			2d	      DEFERRED
(5) Menu positioning and control hooks		3d	      DEFERRED
(5) Off-screen windows				3d	(doc) DEFERRED
(5) Unobscured Screen Bar / Screen-Gap Trick	4d	      DEFERRED
(6) Menu-button cancel for bools, props		1d	 DOC  DEFERRED
(7) Eliminate flashing checkmarks		3d	 DOC  DEFERRED
(8) Sub-menu box precalculation			3d	      DEFERRED


TITLE:  Intuition 3.0 project

DESCRIPTION:  Make necessary and desirable modifications to Intuition
for the 3.0 (AA) ROM.  A number of changes will be required by AA
hardware and graphics.library changes, including:
    - direct support for the new modes
    - support for certain new graphics features (eg. palette sharing)
As well, there are a number of other enhancements not directly
required by AA but nevertheless desirable for an AA system.  For
example, tablet-vendors are seeking fancier input-events.  They want
to be able to write more sophisticated drivers, which harmonizes well
with the concept of the AA-machine, which is a graphics-station.




TITLE:  One line description of what is to be accomplished

DESCRIPTION:  More detailed description, if necessary

BENEFITS:  Why this is desirable

COSTS:  Code space, development/testing time, speed impact, etc.
(Compatibility handled separately).

COMPATIBILITY BURDEN:  What the cost will be to developers wanting to
work under both V37 and V38.  What the risks are for software written
for V34 or V37.

DEPENDS:  What else it depends on, and what depends on it




TITLE:  (1) DONE: New Look Menus and Screen Bar

DESCRIPTION:  Allow the programmer more control over the color of the
menus and screen bar, with the specific intent of allowing black text
on white menus, which looks best but was hard to achieve under V37.
Along with this, we intend to provide scaling of the checkmark and
Amiga-key images.

BENEFITS:  Completes the New Look revamping of the system appearance,
which didn't touch the menus under V37.  Completes the support for
arbitrary fonts in menus.  Required in order to achieve the
"inter-screen-gap trick".

COSTS:  Order of 700 bytes code, about 2 weeks work (mostly done
already).  No other impacts.

COMPATIBILITY BURDEN:  For the normal case of an application using
GadTools, the same code runs automatically under V37.  For special
cases under GadTools, or for applications doing their own menu layout,
some simple versionated code is required.

Pre-existing software will be largely unaffected by the change.  For
old "visitor" windows on new-style public screens (eg.  Workbench),
the screen bar color may be different from before, but apps never
render into the screen bar.  For old windows, the bar reverts to its
old colors when the menu button is pressed.

It may be possible to implement things in a way that most users
of GadTools will automatically get NewLook menus.  It would need
to be worked out.  It would clearly be a less orderly design.
It would mean that GadTools applications with text-only menus
on the Workbench screen would get the full New Look.  GadTools
applications on custom screens will get scaled checkmarks and
amiga keys, but not the new colors.

DEPENDS:  GadTools needs to know about and support this.  The changes
are simple, and completed.




TITLE:  (1) AA Support Issues

DESCRIPTION: The collection of issues required to support the AA hardware
and 3.0 graphics.library features.  These include:
DONE	- support for the new modes
(done)	- extensions to mode coercion (coexistence of screens with
	  different scan rates)  This includes the MouseScaleY fudge.
(done)	- support for mode-promotion (de-interlacing by using a higher mode)
	- Sprite changes (listed separately)
SKIP	- Inter-screen gap trick and other ISG issues (listed separately)
	- support of new palette functions (listed separately)

BENEFITS: Makes the new hardware and software features available under
Intuition.

COSTS:  Support for the new modes and extension to coercion is both
cheap and easy.  The mode promotion will not be too hard for me if
the hard parts go into graphics.library.  All told, give about a week,
not counting sprite, palette, and ISG issues.  Should get by for 300
bytes.

COMPATIBILITY BURDEN:  Hopefully small, for programs that are
gfx-database driven.  In most cases, we're talking about applications
that are 3.0-aware.

DEPENDS:  Some design and implementation can proceed based on
discussions with the graphics.library engineers, but final
implementation and testing will require a basically working 3.0
graphics.library.




TITLE:  (2) AA Sprite support

DESCRIPTION:  The AA chipset provides significantly more flexible
sprites, including wider and finer sprites.  Graphics.library will
grow support for these new sprite attributes.  It would be nice
if Intuition could take advantage of that for the pointer, allowing
both the user and applications access to fancier sprites.  It
would be sensible to add a SetBusyPointer() call at this point.

BENEFITS: Permits a better-looking pointer, which is the last chunky
element of our UI.

COSTS:  Most of the hard part should be in graphics.  I expect them
to deliver a nice black-box set of sprite functions.  Intuition's
wrapper shouldn't be hard or large.

COMPATIBILITY BURDEN:  Versionated code required to take advantage of
new sprite functions.  If we allow new-mode sprites to be set as the
default pointer, we have to investigate any potential compatibility
problems with existing software.

DEPENDS:  Graphics.library sprite interface design required.  Will
require enhancements to Pointer preferences.




TITLE:  (2) AA Palette/color support

DESCRIPTION:  3.0 Graphics provides new palette functions for color
matching and palette sharing.  We need a design and implementation
within Intuition which passes these beneficial functions along to the
user.  This is particularly important for shared screens including the
Workbench screen.  Also, we need to use deeper color-setting functions.

BENEFITS:  Integrate these important functions with Intuition.

COSTS:  Perhaps two days, 100 bytes.

COMPATIBILITY BURDEN:  New programs should be able to take advantage
of the Intuition part without conditional code, but they'll need
conditional code to handle the graphics stuff.  No impact on existing
code.

DEPENDS:  3.0 Graphics.




TITLE:  (3) Mode Promotion

DESCRIPTION:  Because AA machines will not contain a de-interlacer, the
software needs to promote non-VGA modes to VGA modes, under the right
circumstances.

BENEFITS:  Allow a flicker-free display for software that uses old modes.

COSTS:

COMPATIBILITY BURDEN:  New double-rate NTSC and PAL modes won't be 100%
compatible, owing to non-interlaced copper lists, overscan limits not
identical, etc.

DEPENDS:



TITLE:  (3) Attached Screens

DESCRIPTION:  An application's display space may be composed of several
screens (eg. for a paint program, a canvas, a control panel, and a palette
screen).  Under V34, they could be made not draggable (no drag bar), but
V37's mouse-screen drag circumvents that.  What many of these people
really want is a collection of screens that can be dragged as a unit.

BENEFITS:  A lot of interesting AA software will depend on deep screens
HAM screens, or screens where colors are determined by the imagery to
be displayed.  None of those are ideal conditions for a control panel,
which need only be two or three planes deep, and with fixed colors.  So
control-panel screens will be more common.  This will give them the
behavior they want.

COSTS:  500 bytes, three days.  (2 days?) No performance change.

COMPATIBILITY BURDEN:  You have to request this feature to get it, so
old applications are unaffected.  A tag for OpenScreenTags() is
envisioned, so V37/V38 software automatically works under both
versions without versionated code.

DEPENDS:  None.




TITLE:  (3) DONE: Intuition's use of AllocRemember()

DESCRIPTION:  Intuition uses AllocRemember() and FreeRemember() for a
lot of its internal tracking.  In particular, it uses
FreeRemember(FALSE) for its screens and windows, which frags memory
and makes the cleanup code unnecessarily complex.  The idea here is
to simplify the cleanup code and avoid fragging memory.

BENEFITS:  Less ROM space, less RAM fragmenting.  No performance
change.

COSTS:  Two days.

COMPATIBILITY BURDEN: None.  External AllocRemember() won't change,
unless we choose to pull the headers from a pool.

DEPENDS: None.




TITLE:  (3) DONE: Fix depth-gadget for fully-visible windows

DESCRIPTION:  Starting in 2.0, Intuition uses a single depth gadget for
window depth arrangement.  If a window is the frontmost one, it will
be sent to the back, else it will be brought to the front.  The problem
is that a window can be fully revealed, even if not frontmost, and
this type of window comes to the front when you click on the depth
gadget (which has absolutely no effect on the visuals).  The window
should go to the back instead.

BENEFITS:  Fixes serious inconsistency and source of confusion in the
user interface.

COSTS:  Very slight.  Perhaps a half-day and a few dozen bytes.

COMPATIBILITY BURDEN:  None.

DEPENDS:  The grunt work belongs in a new function in layers.library,
UnobscuredLayer().



TITLE:  (4) DONE: IDS_DISABLED Support for gadget images

DESCRIPTION:  Boopsi images can already be rendered in a "disabled"
state, where they take care of their own ghosting appearance (instead
of letting Intuition spray the ghosting pattern).  However, the way
gadgets are implemented, ghosting is always done by rendering the
gadget image in the regular state, then overlaying the dot-pattern.
There needs to be a way to let the imageclass do the disabled-rendering
itself.

BENEFITS:  Better-looking rendering.  Completes the intended scope
of custom images.

COSTS:  Almost free, once the design is figured out.  May require
a precious gadget flag bit.  Give three days for design and implementing.

COMPATIBILITY BURDEN:  Should be none.  When running under V37,
ghosting may appear differently, since it's done the old way.  Under
V38, the improved appearance (usually color of dots or area covered by
dots) will result.

DEPENDS:  None.




TITLE:  (4) System support for Double-Buffering

DESCRIPTION:  It's hard to do Intuition-friendly double-buffering.  We
could provide a mechanism to handle this better.  This becomes
increasingly important as we move towards device independence.  We
would provide some kind of "ChangeScreenBitMap()" call that would be
ignored or block during menu or gadget activity, else it would call
some "ChangeVPBitMap()" call in graphics.library.

BENEFITS:  Makes it easy to have menus and some gadgets on a
double-buffered Intuition screen.

COSTS:  Perhaps three days (design & coding), not much space at all.

COMPATIBILITY BURDEN:  No impact on pre-V38 software.  Application
writers who want to take advantage of it would have to code specifically.

DEPENDS:  Requires existence of viewport-double-buffering support in
graphics.library.




TITLE:  (5) Intuition-friendly Screen-locking

DESCRIPTION:  Sometimes an application needs to lock the screen (eg.
to allow draggable icons or pop-up menus).  They typically need to
keep Intuition running (eg.  for mouse-motion information), but they
run the risk of a deadlock.  They currently need to install a deadlock
task.  A set of calls would be added which puts Intuition into a
"locked visuals" state, which would obtain the layer locks(?  - on the
callers task!).  In that state, Intuition events would be deferred
much as they are in menu-state.  The application would do the drastic
rendering, then release Intuition.

BENEFITS:  Enables later addition of nice UI features without ROM
change.

COSTS:  Fairly little code once investigation is done.  Perhaps 3 days.

COMPATIBILITY BURDEN:  Only risk seems to be if a pirate window used
this on a non-public screen.

DEPENDS:  Investigation of several issues, including safety of such a
thing, whether a new ...VERIFY class is needed, etc.



TITLE:  (5) Sub-menu delay

DESCRIPTION:  When the mouse travels across a menu item, the submenu
springs forth or goes away immediately.  It would be more friendly if
the submenu didn't pop up or go away if the mouse is travelling
quickly.  That would allow you to "cut the corner" on your way
to a menu item.

BENEFITS:  Makes menus less frustrating.

COSTS:  A little bit of code, perhaps one day.

COMPATIBILITY BURDEN:  None

DEPENDS: Legal issues?




TITLE:  (6) DONE: Embed WindowPort into XWindow structure

DESCRIPTION:  At ModifyIDCMP() time, the WindowPort may need to be
created or destroyed.  It would be simpler to create an instance of
WindowPort in the XWindow structure, and use that.  It would also
eliminate one of the most frequent cases of ModifyIDCMP() failure.

BENEFITS:  More robust.  Equivalent to fixing a memoration problem.

COSTS: Less than a day.  ROM size similar.  Possible marginal increase
in RAM use.

COMPATIBILITY BURDEN:  None.

DEPENDS:  None.



DEFERRED ITEMS:


TITLE:  (4) Enhanced Tablet Support

DESCRIPTION:  Tablet device drivers work by sending input events to
Intuition, which interprets them.  A number of obvious features are
missing, such as a pressure component in the event, access to the fine
resolution of the tablet (vs.  the coarse resolution of the screen),
control over aspect ratio, etc.  As well, there are some unresolved
issues concerning autoscroll screens.

BENEFITS:  Tablets are popular with the graphic-design set, for
whom AA is targeted.

COSTS:  Design will take some time.  Implementation of the chosen
design should be straightforward and compact.  Give it five days
of total work.

COMPATIBILITY BURDEN:  Applications receiving events require
versionated code for new features (eg.  pressure), but not for
improvements (eg.  autoscroll behavior,a aspect ratio control).
Device drivers need to be rev'd.  No backwards compatibility
risk.

DEPENDS:  Investigation of alternatives with bix:peabody and
bix:dlovell.




TITLE:  (4) Non-activatable ToolBox windows

DESCRIPTION:  Because of the slow and more noticable rendering when a
window goes active or inactive, having a toolbox window has become
awkward.  Clicking in the toolbox means that the main window's border
and the toolbox's border both have to be redrawn, then a second time
when the user returns to the main window.  A toolbox window is one
which is associated with some set of other windows.  If you click in
it while one of those other windows is active, the activation
rendering doesn't change.  The toolbox window remains active as long
as the selected gadget is active.

BENEFITS:  Cleaner user-interface for programs like DesignWorks.

COSTS:  (E13199) Perhaps a two day effort.

COMPATIBILITY BURDEN:  Only programs that request it get the feature.
If you request the feature, and you're running under V37, things will
work, but without this feature.

DEPENDS:  None.



TITLE:  (5) Screen-depth notification

DESCRIPTION:  There are several reasons that an application might wish
to be notified (asynchronously or synchronously) of screen depth or
front-screen changes.  They include:  commodities that would want to
activate a window on the new front-screen; tablet-drivers that need to
know about the aspect ratio of the front screen; also it would be neat
to be able to integrate a VGA-card which has a video switch so that
its "screen" would join the screen-depth rotation.

BENEFITS:  As outlined in description.

COSTS:  2 days, 200 bytes?

COMPATIBILITY BURDEN:  None.

DEPENDS:  Are we ever going to see a VGA-card with a software
video-switch?




TITLE:  (5) Off-screen windows

DESCRIPTION:  AA Layers may provide support for off-screen layers.  If
this happens, then Intuition will provide matching support to allow
the user to drag a window off the edge of a screen.

BENEFITS:  More flexible use of screen real-estate.

COSTS:  Not too severe.  Bounds-checking for interactive movement is
very easy (if you can drag it there, you can drag it back).  Bounds
checking for programmatic movement is hard if you expect the user to
drag the window back.  Perhaps 1 day if we don't allow programmatic
movement, three days if we do.

COMPATIBILITY BURDEN:  Callers of the old MoveWindow() function will
expect compatibility with old limits, so we would institute a new
call, perhaps AbsMoveWindow() or NewChangeWindowBox() or somesuch.
All draggable windows will be moveable off-screen. 

DEPENDS:  Layers must grow support for off-screen layers.



TITLE:  (5) Unobscured Screen Bar and Inter-Screen Gap Trick

DESCRIPTION:  Because AA may require a larger inter-screen gap,
Intuition could arrange to restrict the objects (and hence the colors)
that are rendered into the screen title area, allowing the screen
title to sit "in" the inter-screen gap, reducing its size.  This would
be accomplished by prohibiting windows from overlapping the screen
drag bar, and by using the New Look Screen Bar, which offers the
necessary control over colors.  At the same time, we'll make the
changes required for variations in the ISG that AA gives.

BENEFITS:  Reduces the unsightly inter-screen gap.  Also, keeping
windows off the screen bar is a feature in its own right, and will be
available independently.  This trick will be optional, so that
applications that want the full size of the canvas can still have it
(albeit with a larger ISG).

COSTS:  < 500 bytes of code, perhaps 4 days of work.  Will have
to be careful about speed, since some ISG work happens inside
mouse processing.

COMPATIBILITY BURDEN:  Existing screens are unaffected.  For V37/V38
programs, the trick is only useful for AA-only modes.  In any case,
V37 Intuition will cleanly ignore the request for this trick.  One
compatibility problem is an ISG-trick public screen, which has the
"keep off the bar" flag.  The usable area for the window is smaller
than the screen size.

DEPENDS:  Graphics will have to cooperate by allowing a screen to
start prematurely (I think it does already), and will have to offer
some guarantee (to Intuition and to application developers) of how
many colors will be available by the time the title-bar begins.
Requires that we have the New Look screen bar.




TITLE:  (5) Menu positioning and control hooks

DESCRIPTION:  Having the menu strip appear relative to the mouse may be
desirable, but beyond the scope of 3.0 ROM changes.  Likewise for
keyboard control and sticky menus.  However, it _might_ be quite easy to
put in some private hooks that would permit this to be added with
a disk-based utility.

BENEFITS:  For fairly little effort, we can enable some really
nice features.

COSTS:  Perhaps 3 days analysis, design and coding.

COMPATIBILITY BURDEN:  None.

DEPENDS:  None.




TITLE:  (6) Menu-button cancel for bools, props

DESCRIPTION:  Pressing the menu button during sizing or dragging
a window cancels that operation.  We could extend this to include
sliding of prop gadgets and pressing boolean gadgets.

BENEFITS:  More consistency, convenient feature.

COSTS:  Should be trivial to code.  1 day.

COMPATIBILITY BURDEN:  Probably none.  A subclass of propgclass that
used the right-mouse button event (if we pass it along?) may no longer
have it.

DEPENDS: None.




TITLE:  (7) Eliminate flash when checkmarks are extend-selected

DESCRIPTION:  If you extend select a checkmarked item in a menu, the
menu panel flashes horribly.  The problem will be worse on deeper
screens.  This can be reduced in the general case and eliminated in
specific cases.

BENEFITS:  More professional appearance.

COSTS:  The specific-case code is written and tested.  It just costs
space to include.  The general-case code needs to be written.  Give
it two days and 200-400 bytes total, depending on whether we do the
lesser general case only, or also the better specific case.

COMPATIBILITY BURDEN:  None (unfortunately -- the attempt at eliminating
the flashing wasn't compatible with a few weirdos; we'll go with a 100%
compatible version).

DEPENDS:  None.




TITLE:  (8) Sub-menu box precalculation

DESCRIPTION:  At SetMenuStrip() time, the size of the menu panels is
pre-calculated, but the sub-menu panels are not pre-calculated.  We
could arrange to pre-calculate sub-menus, for speed.  We could choose
to give this to everyone, for more code, or only to those that know to
ask, which takes less code.  GadTools could be one who knows to ask.

BENEFITS:  Sub-menus would appear faster than they do today.

COSTS:  SetMenuStrip() would run slower.  ResetMenuStrip() would
continue to be fast.  Three days.

COMPATIBILITY BURDEN:  None if we take the expensive option.  The
menu-creator (incl. GadTools) would have to be aware otherwise,
but that code would run unaltered on V37 and earlier).

DEPENDS:  None.









TITLE:

DESCRIPTION:

BENEFITS:

COSTS:

COMPATIBILITY BURDEN:

DEPENDS:



TITLE:

DESCRIPTION:

BENEFITS:

COSTS:

COMPATIBILITY BURDEN:

DEPENDS:



---------------------------------------------------------------------------
---------------------------------------------------------------------------

GRAB BAG TO DO:

---------------------------------------------------------------------------
SCREENS / WINDOWS
---------------------------------------------------------------------------

WA_Min/MaxInnerWidth/Height (216 bytes)

LONG figureInner( inner_tag, regular_tag, newwin_value, inner_offset, tags )
ULONG inner_tag;
ULONG regular_tag;
LONG newwin_value;
LONG inner_offset;
struct TagItem *tags;
{
    LONG answer, inner;

    answer = GetUserTagData( regular_tag, newwin_value, tags );
    if ( ( inner = GetUserTagData( inner_tag, ~0, tags ) ) != ~0 )
    {
	answer = inner + inner_offset;
    }
    return( answer );
}

	WindowLimits( window, 
	figureInner( WA_MinInnerWidth, WA_MinWidth, NW( MinWidth ),
	    window->BorderLeft + window->BorderRight, tags ),

	figureInner( WA_MinInnerHeight, WA_MinHeight, NW( MinHeight ),
	    window->BorderTop + window->BorderBottom, tags ),

	figureInner( WA_MaxInnerWidth, WA_MaxWidth, NW( MaxWidth ),
	    window->BorderLeft + window->BorderRight, tags ),

	figureInner( WA_MaxInnerHeight, WA_MaxHeight, NW( MaxHeight ),
	    window->BorderTop + window->BorderBottom, tags ) );


---------------------------------------------------------------------------

DONE: ScrollWindowRaster() that calls borderpatrol and sends refresh.
Also needs to call call ScrollRasterBF() (new gfx lib that uses
backfill hook).

Q:  SWR() needs to choose between doing BorderPatrol() or just sending
the refresh message.  Full BorderPatrol() means that scrolling in
a gadget would give repair, but it's not efficient.  

---------------------------------------------------------------------------

DONE: Menu lending
Issues:
Need to arrange to not activate the other window.
Need to handle menu key equivalents
Need to test when to-window is RMBTRAP

---------------------------------------------------------------------------

MouseScaleY is currently fudged.  What about new monitors?

---------------------------------------------------------------------------

Promotion logic needs to be finished.

---------------------------------------------------------------------------

Pen sharing:

Perhaps all public screens should have pens 0-3 allocated as shared,
in addition to their SA_Pens.

DONE: Console seems to need to know if a screen is pen-aware or not.  If
not, he'll complement as always.  If aware, he'll restrict to 2
planes, keeping him inside 0-3 allocated above.

DONE: Have to decide about high-end pens, like should we obtain pen ~0 etc.?

---------------------------------------------------------------------------

SA_Interleaved -> IPrefs.  Also, how about custom interleaved BMs?

---------------------------------------------------------------------------

DEFERRED: Autoscroll screen to ensure that a new window is visible.

DONE: MakePixelVisible() function that would autoscroll the screen so that the
pixel in question is visible.  Maybe take a rectangle, too.
What about screen depth arrangement?

DONE:  (WA_AutoAdjust) (E7696) A WA_OnScreen or WA_Relative tag which
would attempt to position the window in the visible portion of an
oversized (scrolling) screen.

TODO: Need to autoadjust windows that are too far down or right.

---------------------------------------------------------------------------

DEFERRED: New window manipulation functions taking a friend/foe function.

---------------------------------------------------------------------------

DONE: New screen manipulation functions taking a friend/foe function.

---------------------------------------------------------------------------

Support for light-on-dark text, or some sort of SA_Pens editor.  The
text pen is traditionally pen 1.  The pen-specification prevents the
Workbench screen from having light text without inverting shine and
shadow.  At the very least, it'd be nice to stuff in a pen-spec that
had the uses of black and white reversed.

Or, hooks to add this later.

---------------------------------------------------------------------------

Tim Holloway wants a USERMESSAGE Class for IntuiMessages.  See B12509
and amiga.com/suggestions 532 and that thread.  Randell tells me that

jimm has an elaborately designed scheme.

---------------------------------------------------------------------------

(B14317) Dirty pixels left after SetWindowTitles():

If a window title has a negative extender, and it's changed (via
SetWindowTitles()) to something else, a few pixels of the previous
title remain visible.

---------------------------------------------------------------------------

Public access to the zoomed coordinates.

---------------------------------------------------------------------------

DONE: Size-only zooming that preserves upper-left corner (better,
upper-right, so zoom gadget doesn't move), for title-bar type
zooming.

---------------------------------------------------------------------------
GADGETS / IMAGERY / MENUS
---------------------------------------------------------------------------

Better underscoring:

Teach gadclass to handle GFLG_LABELIMAGE.  Teach itexticlass to do
underscoring.  Then, GadTools does everything in the usual way with
its gadget labels as IntuiTexts.  In the last step, it does:

	gad->GadgetText = (struct IntuiText *) NewObject( NULL, "itexticlass",

		/* IA_Data takes an IntuiText */
		IA_Data, gad->GadgetText,

		/* Don't override mode & pens */
		IA_NoOverride, TRUE,

		IA_Underscore, '_',
		);
	gad->Flags |= GFLG_LABELIMAGE;

If Intuition starts to recognize GFLG_LABELIMAGE, this would allow
struct Images to be used as labels for classic (bool, prop, string)
gadgets.

Now, the idea here is to teach itexticlass to underscore better than
GadTools can.  The big question is who should worry about removing the
"_" character?  At first, I guessed that this was obviously the job of
itexticlass, however GadTools does want to call IntuiTextLength() and
TextExtent() and such on the IntuiText, and an embedded underscore
will pose a problem.  So from that point of view, it seems that
GadTools needs to strip out the underscore symbol, merely noting
where in the string it was.  Then itexticlass takes the string without
embedded underscore, and takes an IA_UnderscorePosition, NUMBER, which
is the number of the character to underscore.

Is that the way to go?  Or should itexticlass support strings with
and without embedded underscore?

/* This routine isn't really what we want.  But let's keep it for
 * reference, at least for now.
 * For the first IntuiText in a list, scan the text for the
 * underscore character, and remove it.  Note down the starting
 * position and the length for hand-rendering the underscore stroke
 * later.
 *
 * Note 1: munges the IntuiText->IText in place.
 * Note 2: doesn't yet figure out the width of the stroke.
 * Note 3: doesn't know how far below the text to render the stroke.
 */
figureUnderscore( itext, underscore )
struct IntuiText *itext;
char underscore;
{
    char *src, *dest;

    /* Let's examine the actual text: */
    src = itext->IText;

    /* Copy each character in the string, remembering what
     * what we copied:
     */
    while (ch = ( *dest++ = *src++ ) )
    {
	if (ch == underscore)
	{
	    /* Well, we copied the underscore, so let's
	     * back up the destination pointer.  In
	     * addition, we'll temporarily terminate
	     * at that point, so we can measure how far
	     * in the underscore must begin.
	     */

	     *(--dest) = '\0';
	    score_left = IntuiTextLength(itext);
	    score_width = char-length( *src );
	}
    }
}

---------------------------------------------------------------------------

We'd like to have gadget rendering able to use the frame-methods.  The
problem is that there are images which already know how to DRAWFRAME,
but their idea of where to draw with respect to the gadget box may not
be right.  

If I guess right, most custom images using frame-methods just throw
away their own dimensions, and render based on the frame supplied.
Well, gadget frame images need to be smarter.  For example, the frame
of a string gadget sticks out from the gadget itself.  I imagine something
like this, where the image is given dimensions relative to the gadget:
then:

	str_frame = NewObject( NULL, "frameiclass",
		/* Dimensions designed RELATIVE to
		 * gadget select box
		 */
		IA_Left, -6,
		IA_Top, -3,
		IA_Width, 12,
		IA_Height, 6,
		...
		);

Then, Intuition would at some point call:

	DrawImageFrameState( rp, gadget->GadgetRender,
	    /* These have the relativity already evaluated */
	    left, top,
	    width, height,
	    state );

And the image would be expected to ADD its dimensions to the frame
instead of replace its dimensions by the frame.


Now it's clear why it's not good enough for Intuition to just use
frame-methods in place of existing ones.  So we need the gadget to
hold a flag, GMORE_FRAMEIMAGE, which tells Intuition to use
frame-methods.

So do we need a new set of methods IM_DRAW_RELATIVE_FRAME and friends?
Or should it be an image attribute that the gadget somehow sets?

Let's say that it's the gadget class's responsibility to know whether
the image it wants to use supports the relative frame concept.  (Is
that reasonable?) So the gadget sets an image attribute
IA_UseRelativeFrame in the image, so the image knows to do the frame
calculation explained above (add its dimensions as relatives to the
frame box).  The gadget also sets some extended flag GMORE_FRAMEIMAGE,
which tells Intuition to send frame-methods instead of old ones.

We'd probably need new functions:  EraseImageFrame() and
DrawImageFrameState(), or maybe a DrawImageTags(), or IRenderA().

---------------------------------------------------------------------------

buttongclass should throw away a few INTUITICKS before repeating.

---------------------------------------------------------------------------

BOOPSI erase-gadget method, used by Intuition to move GREL gadgets.

---------------------------------------------------------------------------

DONE: GADGETHELP IDCMP message and boopsi method. 

---------------------------------------------------------------------------

DONE:

ULONG DoGadgetMethodA( struct Gadget *gad, struct Window *win,
    struct Requester *req, Msg message ) (A0,A1,A2,A3)
==varargs
ULONG DoGadgetMethod( struct Gadget *gad, struct Window *win,
    struct Requester *req, ULONG MethodID, ... ) (A0,A1,A2,A3)

ULONG DoGadgetMethodA( gad, win, req, message )
struct Gadget *gad;
struct Window *win;
struct Requester *req;
Msg message;
{
    struct GadgetInfo *ginfo;
    LONG retval;

    ginfo = getGI();

    /* OM_NEW, OM_SET, OM_NOTIFY, and OM_UPDATE have the GadgetInfo
     * In the third position.  All other methods must have the
     * GadgetInfo in the second, by decree.
     */
    if ( case == OM_.. )
    {
	/* opSet is an example structure with GadgetInfo
	 * in the second position
	 */
	((struct opSet *)message)->ops_GInfo = ginfo;
    }
    else
    {
	/* gpInput is an example structure with GadgetInfo
	 * in the second position
	 */
	((struct gpInput *)message)->ops_GInfo = ginfo;
    }
    retval = SM( (Object *) g, message );
    
    /* ZZZ:  NULL out ginfo? */
    return( retval );

}

---------------------------------------------------------------------------

DONE: frameiclass to handle the basic frames:  GadTools, moat (indented
groove), fence (like string gadget), icon drop-box.

---------------------------------------------------------------------------

Menu items could look ahead to see if the next TextAttr pointer was
the same, then skip the redundant close/open.

Would require an internal PrintIText() that was smarter.
Also would have to get smarter about command-key font, since we always open
a new font there.  We should skip doing that if the style is plain.

---------------------------------------------------------------------------
OTHER
---------------------------------------------------------------------------

Mouse blanking.

---------------------------------------------------------------------------

Multiple version of GetAttr().  Could be a GetAttrs() with
a tag-list of {tag, &long_data}

Also, could use existing GetAttr() with AttrID of say TAG_MORE,
and the storage-ptr then being a pointer-to-taglist.

Return code should probably be zero if any attrs are unknown.  Alternately,
RC could be the number of attrs successfully filled.  Or maybe number
unsuccessfully filled (which is more useful, but less analogous
to the old GetAttr).

---------------------------------------------------------------------------

Arrow buttons from sysiclass for use anywhere in a window.  They'd
share vectors with the existing arrow-kinds.  They'd have
VIBORD_THICK3D (i.e.  GadTools style BevelBoxes).  The vectorclass
code would need to learn that these guys need to be rectfilled
when selected, and that the border needs to be indented.

Cost: 24 bytes x 4 arrows, 96 bytes.  Then add a bit to vectorclass.

---------------------------------------------------------------------------

EasyRequest() flag to open blocking requester and busy pointer

EasyRequest() flag to not open active.  What about popping screens
at that point?

EasyRequest() on an attached screen may need to do double-popping
(screen group and child screen)

---------------------------------------------------------------------------

(E5914) Default busy pointer:

Intuition should contain a standard busy-pointer, user-editable.

One problem with busy pointers is that sometimes they flash on for a
very short time, because the application can't tell that a particular
operation won't take any time at all this time.

Here's an interesting approach:

	SetBusyPointer(win, ticks)

After ticks INTUITICKS have passed, the busy pointer will come up
unless a SetPointer() or ClearPointer() has occurred.  The idea is
that the very short "busy" spells don't cause the pointer to appear.
The pointer could still appear for a very short time, but only when
there's been a real period of busyness.

Mike wants me to use one of the spare V37 vectors for this.  Sounds
reasonable.

---------------------------------------------------------------------------

DisplayBeep() changes color zero, which can confuse the pen-sharing
code of gfx.  We should hold the pen-share semaphore for that time,
or arrange to poke only the HW.

---------------------------------------------------------------------------

(B11322) DisplayBeep() shouldn't restore color zero if it changed:

Before DisplayBeep() restores the color it changed, it should check
to see if the color it beeped has since been altered.  In that case,
it shouldn't attempt to restore the old color.

---------------------------------------------------------------------------

E11323 - SA_NeverBeep:

DisplayBeep(NULL) has the nasty habit of causing every screen to be
beeped.  I can see applications that might never want their screens
beeped.  This tag would mark a screen as not beepable.

---------------------------------------------------------------------------

#define DBEEP_ERROR	0	/* Regular negative beep: Attention: error happened */
#define	DBEEP_FLASHONLY	1	/* No audio; flash only, eg. letters in an integer gadget */
#define DBEEP_WARNING	2	/* Mild negative feedback, eg. no match in file name completion  */
#define DBEEP_OK	3	/* Pleasant happy short sound, eg. job done */

#define DBEEPMASK		~(0x00000003)	/* safely take 4 kinds */
				~(0x00000007)	/* AllocMem ensures we can take 8 */
				~(0x80000007)	/* Here are 16 kinds */
Talin suggests
"Confirmed"
"Attention"
"Alert"
"Warning"
"Other/Special"

---------------------------------------------------------------------------

(B10457, E11658) Better MrgCop() recovery:

We don't try very hard to survive a MrgCop() failure, though we no
longer pull a guru.  More work could be done here.

---------------------------------------------------------------------------
---------------------------------------------------------------------------

MAYBE DEFERRED or DENIED:

---------------------------------------------------------------------------
---------------------------------------------------------------------------

(B13884) Mouse queue limits cause lost information.

This one sucks big.  Starting with V36, Intuition puts a queue limit
on repeat-keys and mouse-moves.  Well, excess repeat keys (that are
discarded) contain no special information, but discarded mouse-moves
contain up-to-date positions.  There needs to be some way to get this
updated information to the application.  One way would be to disable
the mouse-queue limit (could be effectively done by SetFunctioning
OpenWindow(), OpenWindowTagList(), and SetMouseQueue()).  The other
way would be for Intuition to note that a mouse-move was discarded.
Then, when one of previously sent ones was replied, a fake mouse event
would be sent.

---------------------------------------------------------------------------

(E13782) Non-rendered menus shouldn't be selectable:

When menus fail to be drawn on account of failure to allocate the
memory for a menu panel, you can still select the items if you know
where they are.  This shouldn't be possible.

---------------------------------------------------------------------------

Dual-playfield support:

It would be nice to provide some kind of system-level support for
dual-playfield screens.


---------------------------------------------------------------------------

String gadgets:  Provide a tag which will cause a GADGETUP to be sent
upon any inactivation.

---------------------------------------------------------------------------

A window flag asking for MOUSEMOVE-IDCMP messages originating
from a gadget to have the imsg->IAddress equal the gadget.  Currently,
IAddress points to the window, which is redundant, since
imsg->IDCMPWindow contains that information.

---------------------------------------------------------------------------

Screen-centered autorequests.  Window-relative autorequests.  Also,
successive ones could be offset from each-other.

---------------------------------------------------------------------------

(E11584) 4-color short pointer for GetPrefs():

The old struct Preferences should contain some reasonable 4-color
16-pixel-high pointer, rather than getting a truncated, shallow
version of the real pointer.

---------------------------------------------------------------------------

(B11445) DPaint DClip can be badly positioned

---------------------------------------------------------------------------

B9688 - ImageClass dimensions into IM_DRAW?

---------------------------------------------------------------------------
---------------------------------------------------------------------------

DONE:

---------------------------------------------------------------------------
---------------------------------------------------------------------------

DONE: GFX growing a ColorSpec type structure.  I should support that.
SA_Colors32,table should pass that table to LoadRGB32()

---------------------------------------------------------------------------

DONE: SA_VideoControl -> VideoControl()

---------------------------------------------------------------------------

DONE: B14679: ghosting pattern affects subsequent string gadgets if their
StringExtend fieldpen is non-zero.

---------------------------------------------------------------------------

DONE: SA_Draggable: Non-draggable screens

Under 1.3, you could make a screen effectively non-draggable by
obscuring its drag-bar with a window.  Of course, a screen which was
intended to be draggable could end up awkward to drag if the screen
drag-bar got covered.  2.0 adds mouse-screen-drag, which is intended
to let you drag such screens, but it also lets you drag the screens
that weren't supposed to be draggable.  There may be a few legitimate
uses for a non-draggable screen.

---------------------------------------------------------------------------

DONE: Better arrow imagery.

---------------------------------------------------------------------------

DONE:  Need a new screen tag to supply the screen's backfill hook.
(SA_BackFill).

---------------------------------------------------------------------------

DONE: (B12447) Clicking in no-window doesn't change active screen:

If you click in the "no-window" area of a screen, that screen doesn't
become the active one.  Only the active screen autoscrolls.  This
should be simple to fix, and it's a good idea since it's been known to
confuse people.

---------------------------------------------------------------------------

DONE: (B12397) pointer still blanks in inter-screen gap

---------------------------------------------------------------------------

DONE: B14308 (c++ keyword collision)

---------------------------------------------------------------------------

DONE: Code up a closeWindowCommon() routine.

---------------------------------------------------------------------------

DONE: CHANGEWINDOW notification on window-depthing, so child-windows
can sort of be done.

---------------------------------------------------------------------------

DONE: (B9428) Put new-style mouse-pointer into ROM default preferences:

The ROM default Preferences in Intuition still contain the 1.3 pointer
arrow.

---------------------------------------------------------------------------
---------------------------------------------------------------------------

DEFERRED or DENIED:

---------------------------------------------------------------------------
---------------------------------------------------------------------------

DENIED:  Support for shallow-layers (layers with fewer bit-planes).
It'd be a tag to pass along to layers.

Not necessary because Mike and Bart have determined that shallow
layers don't save much.  Shallow RastPorts are the answer, and
it becomes application specific then (works under 1.3, even).

---------------------------------------------------------------------------

DEFERRED:  Glyphs in Easyrequesters?

---------------------------------------------------------------------------

DEFERRED:  Make public screen names case insensitive.

---------------------------------------------------------------------------

DEFERRED:  Custom autoknob support

---------------------------------------------------------------------------

DEFERRED:  Blinking cursor for string gadgets.

---------------------------------------------------------------------------

DEFERRED:  Notification of changes for PubScreen mgr.

---------------------------------------------------------------------------

DEFERRED:  (E11441 and others) Support for system gadgets in borderless windows:

Many people try to have a borderless window with a system gadget in it
(usually the close gadget).  However, that causes a top-border to
appear.  This was less noticable under 1.3, since it wasn't rendered
upon inactivation.  It'd be nice if the system could handle putting
the system gadgets into a borderless window, and keep the window
borderless.

---------------------------------------------------------------------------

DEFERRED:  (E11370) Make screen-depth gadget look different from window one:

People complain that users confuse the screen depth gadget with the
window depth gadget.  We could change its imagery.  In particular,
it can be made wider, since no gadget occupies the place of the old
screen-to-back gadget.

---------------------------------------------------------------------------

DEFERRED: Off-screen windows clipping:
1.  Remain entirely on-screen.
2.  Ensure part of the drag-bar is on-screen.
3.  Entirely off-screen.
3.  No limits:  can have window partly off-screen, drag-bar entirely off.

---------------------------------------------------------------------------

DENIED:  Get Layers to set a second bit along with LAYERREFRESH.  Then
I could clear that bit when I send REFRESHWINDOW to a window.  This
would prevent duplicate REFRESHWINDOW messages if a window sat damaged
when it wasn't damaged by the most recent layers operation.  The bit
might be called NEWDAMAGE.  It would be set by layers in the exact
place LAYERREFRESH is set.  It would be cleared in BorderPatrol().

We're talking a pretty minor case here.

---------------------------------------------------------------------------
---------------------------------------------------------------------------
