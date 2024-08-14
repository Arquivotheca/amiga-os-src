Specification for Gadget Toolkit:

Created:  5-Oct-89, Peter Cherna


WHAT IS IT?

The Gadget Toolkit shall be a shared library of custom gadget classes to
allow programmers (in particular ours) easy access to sophisticated user-
interface objects.  It should use the new Intuition Object Oriented Gadget
Classes.


WHAT DOES IT CONTAIN?

The following types of custom gadget have been identified:

- Button (command) gadget.
- Check Box (on-off).
- Simple Mutually Exclusive (one-of-n).
- Slider (level control).
- Scroller (Positional control).
- String (keyboard entry).
- Text Area (multi-line keyboard entry).
- Scrolling List.
- Palette (Pick a color).
- SketchPanel (Drawing area).
- N-Way Toggle.
- High-Low Slider


Key to Feature List:
    *	Does so already.
    -	Needs to be added.
    ?	Should be considered.
    ??	Strange idea, worth noting.

Key to Tag/Attribute List:

    I	Attribute may be set at Init time (NewGadget())
    S	Attribute may be set at any time (SetGadgetAttrs())
    G	Attribute may be gotten (GetGadgetAttrs())
    G*  Attribute may be gotten before init time.  The idea here is that
	some layout routines could inquire about what a gadget's "desired"
	size is and make some global decisions based on such info.

Key to Tag/Attribute Names:

G_...	This is expected to be a standard Intuition gadget attribute.
_...	This attribute is introduced by the Toolkit.


COMMAND GADGET

This gadget consists of a text (or graphic?) label enclosed by a border
(currently a raised rectangular box).  It has the following properties:

    *	Label centered within the button
    ?	Label may be left or right aligned in the button
		_LABELPOS (LEFT_ALIGN | RIGHT_ALIGN | CENTER, I)
    ??  Allow label beside the button, so other label or image can
	go in it.
		_LABELPOS (LEFTSIDE | RIGHTSIDE, I)
    -	Button can figure out its own size
		_AUTOWIDTH (boolean, I)
		_AUTOHEIGHT (boolean, I)
    ?	Special border available to denote a button that ends a requester
	or program (ideas - rounded box, or thicker box, or others)
		_EXITBUTTON (boolean, I)

Other Attributes:
	G_WIDTH (#, IG*)
	G_HEIGHT (#, IG*)
	G_LABEL (text, I)
	G_LABELFONT	(TextAttr, I)


CHECK BOX

This gadget consists of a raised rectangular box, either blank or
containing a checkmark.  A label is placed to the side of the box.

    -	Label is automatically positioned to the left or the right
	of the box, as desired.
		_LABELPOS (LEFTSIDE | RIGHTSIDE, I)
    -	State can be specified/controlled.
		_STATE (#, ISG)
    ?	Imagery depends on font size.
    ?	Allow "indeterminate" state (suggest a ? for imagery).

Other Attributes:
	G_WIDTH (#, IG*)
	G_HEIGHT (#, IG*)
	G_LABEL (text, I)
	G_LABELFONT (TextAttr, I)

SIMPLE MUTUAL EXCLUSIVE

This gadget is a set of choices, only one of which may be selected
at a time.  Selecting one precludes the previous choice.  Its imagery
is a raised blank circle for the unselected choices or a sunken
circle with a spot for the selected choice.

    -	Label is automatically positioned to the left or the right
	of the box, as desired.
		_LABELPOS (LEFTSIDE | RIGHTSIDE, I)
    *	Can specify/control the selected one.
		_STATE (#, ISG)  /* The active one */
    ?	Imagery depends on font size.
    ?	Layout considerations.  (Currently can place them in a column)
    ?	Allow "indeterminate" state - suggest no buttons active.

Other Attributes:
	G_WIDTH (#, IG*)
	G_HEIGHT (#, IG*)
	G_LABEL (text, I)
	G_LABELFONT (TextAttr, I)

Questions/Notes:
A set of MX gadgets greatly resembles a set of checkboxes, except the
checkboxes are independent of each other, so the custom gadget 'checkbox'
will be only one checkbox.  Yet we seem to need a custom gadget  which is
one _set_ of mx gadgets.  How do we handle this inconsistency?  Do we have
a new custom gadget 'CheckBoxSet' which is more like 'MXSet', except that the
caller is likely to directly use a single custom gadget 'CheckBox' but
unlikely to directly use a single custom gadget 'MX' (I guess unless
rolling his own more complex MX cases).  There's a layout issue lurking in
here too, perhaps:  Why shouldn't a 'MXSet' lay itself out?


SLIDER

This is a proportional gadget used to indicate/choose a level or amount.
It has a boring autoknob inside a raised rectangle.

    *	Horizontal or vertical.
		G_FREEDOM (FREEHORIZ | FREEVERT, I), or could be
		inferred as the longer axis!
    *	Caller supplies min, max, current level as any signed word.
		_MINLEVEL (#, ISG)
		_MAXLEVEL (#, ISG)
		_LEVEL (#, ISG)
    ?	Provide step size as well, so that you may ask for
	min = 100, max = 1000, step = 10 (currently you would
	ask for min = 10 max = 100, and a level calculation function
	that returns 10x.
		_LEVELSTEP (#, ISG)
    *	Only reports when level actually changes.
    *	Optional display of level beside slider (not done yet for vertical).
		_DISPLAYLEVEL (NULL, I)
    *	Hook for level calculation function to allow non-linear progression
	of level indicator (function returns converted number for display).
		_DISPLAYLEVEL (function, I)
    -	Optional level display plus editing field beside slider.
		_EDITLEVEL (function, I)  (as for _DISPLAYLEVEL)
    ?	Hook for level display function that can display whatever it likes.
	(May not be needed.  You can do what you like when the level
	changes anyway).
	_CLIENTDISPLAY (boolean, I)
    *	Optional extra report upon commencing/finishing.
		G_GADGIMMEDIATE (boolean, I)
		G_RELVERIFY (boolean, I)
    ?	Jump to nearest integral level upon release.  Would move slider
	to an "integral" position when done (may not be optional).
		_SNAPTOINTEGRAL (boolean, I)
    ?	Allow integral positions only.  Slider would "leap" to next position
	as you slid the mouse.
		_ALWAYSINTEGRAL (boolean, I)
    -	Plain ticks along the scale.
		_TICKS (boolean, I)
    ?	Numbered ticks along the scale.
		_NUMBEREDTICKS(?, I)

Other Attributes:
	G_WIDTH (#, IG*)
	G_HEIGHT (#, IG*)
	G_LABEL (text, I)
	G_LABELFONT (TextAttr, I)


SCROLLER

This is a proportional gadget used to indicate/choose a position
(within a list or in an area, etc.) It has a boring autoknob inside a
raised rectangle.

    *	Horizontal or vertical.
		G_FREEDOM (FREEHORIZ | FREEVERT, I), or could be
		inferred as the longer axis!
    *	Caller supplies firstvisible, visible amount, and total amount
	as any signed word.
		_FIRST (#, ISG)
		_VISIBLE (#, ISG)
		_TOTAL (#, ISG)
    -	Optional repeating arrows (or always supplied, space permitting?).
		_ARROWS (boolean, I)
    *	Only reports when position actually changes.
    *   Optional extra report upon commencing/finishing.
		G_GADGIMMEDIATE (boolean, I)
		G_RELVERIFY (boolean, I)
    ?	Jump to nearest integral level upon release.  Would move slider
	to an "integral" position when done (may not be optional).
		_SNAPTOINTEGRAL (boolean, I)
    ?	Allow integral positions only.  Slider would "leap" to next position
	as you slid the mouse.
		_ALWAYSINTEGRAL (boolean, I)
    ?	Optional "anchors" (go to top, go to bottom gadgets).
		_ANCHORS (boolean, I)

Other Attributes:
	G_WIDTH (#, IG*)
	G_HEIGHT (#, IG*)


STRING

This is a text entry field.  Its imagery is a raised underscore.  The
imagery may become a sunken box if we give up the depth rule.

    -	Label is automatically positioned to the left or the right
	of the box, as desired.
		_LABELPOS (LEFTSIDE | RIGHTSIDE | TOPSIDE?, I)
    *	Optional numeric only.
		_INTEGER (boolean, I)
    -	Other input restrictions (possible visual clues as to field type,
	too): Date, Time, Hex, ...
		_FORMAT (DATE | TIME | HEX | ..., I)
    ?	Direct support for "history" via up/down arrows in custom string
	gadget.
    ?	Direct support for linked string gadgets via TAB, BACK-TAB.

Other Attributes:
	G_WIDTH (#, IG*)
	G_HEIGHT (#, IG*)
	G_LABEL (text, I)
	G_LABELFONT (TextAttr, I)
	(contents attributes like font, color, activefont, activecolor, etc)
	(maxlen, and stuff)


TEXT FIELD

This is a multi-line text entry field.

Attributes: ?

Questions/Notes:  No real work has gone into this one yet.


SCROLLING LIST

This is a list of items plus a Scroller.  The imagery is a raised box
if you may click an item, or a recessed box if the list is for view only.

    *	Real-time scrolling via Scroller provided and handled for caller.
    *	Caller supplies an Exec list, ln_Name's are displayed.
		_LABELLIST (list, IS)
    *	Position within list.
		_TOP (#, ISG)
    *	Caller hears only the selection of the user.
    -	"Drag-Scrolling" within list area.
    -	Optional current selection highlighting.
		_HIGHLIGHTCURRENT (boolean, I(S?))
    ?	Optional extended selection (should use shift-click metaphor).
		_EXTENDSELECT (boolean, I(S?))
    ?	Key equivalents for all options (may be always supplied).
		_KEYEQUIV (boolean, I)
    ?	Set current choice
		_CURRENT (node, ISG)
    ?	Optional display of current choice.
		_DISPLAYCURRENT (boolean, I)
    ?	String gadget for display/editing of choice.
		_EDITCURRENT (boolean, I)
    ?	Auto-Scroll to entry closest to string gadget contents.
		_COMPLETION (boolean, I)
    *	Read-only or clickable
		_READONLY (boolean, I(S?))

Other Attributes:
	G_WIDTH (#, IG*)
	G_HEIGHT (#, IG*)
	G_LABEL (text, I)
	G_LABELFONT (TextAttr, I)


PALETTE

Displays 2^n color squares, and allows user to pick.  Imagery is a set
of squares in a raised box.

    *	Any power of 2 number of choices.
		_DEPTH (#, I)
    *	Begin at any color register (i.e. could do 16-19 for sprites).
		_FIRSTCOLOR (#, I(S?))
    *	Optional current-color indicator above or to left of palette.
		_INDICATORWIDTH (boolean, I)
		_INDICATORHEIGHT (boolean, I)
    *	State is a color.
		_STATE (#, ISG) [or _COLOR (#, ISG)]

Other Attributes:
	G_WIDTH (#, IG*)
	G_HEIGHT (#, IG*)
	G_LABEL (text, I)
	G_LABELFONT (TextAttr, I)


SKETCHPANEL

This is an area into which the user may draw with the mouse.  It is drawn
on a raised rectangle.

    *	Caller chooses size and aspect.
    -	Optional grid.
    *	Follows mouse and reports pixel changes.


Attributes:
	PenColor (ISG)

Questions/Notes:  This one has been written, but may be too esoteric for
the toolkit, although WBPattern prefs and Pointer prefs will use the code.


N-WAY TOGGLE

This is similar in function to mutually exclusive choices, only a single
choice is displayed on the button's face, and every mouse click advances
the choice.  A shifted click cycles the choices in reverse.  The image is a
recessed rectangle nestled immediately inside a raised one, so that the
field is at the same "level" as the background.

    *	Current state
		_STATE (#, ISG)
    -   Allow label beside or above the button.
		_LABELPOS (LEFTSIDE | RIGHTSIDE | TOPSIDE, I)
    ?	States may be left or right aligned, or centered, in the button
		_STATELABEL (LEFT_ALIGN | RIGHT_ALIGN | CENTER, I)
    -	Button can figure out its own size
		_AUTOWIDTH (boolean, I)
		_AUTOHEIGHT (boolean, I)

Other Attributes:
	G_WIDTH (#, IG*)
	G_HEIGHT (#, IG*)
	G_LABEL (text, I)
	G_LABELFONT (TextAttr, I)








Attributes related to the resolution/depth/layout problem:

_INTERWIDTH	preferred horizontal spacing between "elements".
_INTERHEIGHT	preferred vertical spacing between "elements".
_THICKNESS_H,V	preferred thickness of graphic strokes.
_...COLOR	preferred pens for highlighting, fields, shadows, labels,
		etc.
