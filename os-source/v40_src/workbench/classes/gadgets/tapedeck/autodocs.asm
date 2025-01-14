
******* gradientslider.gadget/gradientslider.gadget **************************
*
*   NAME
*	gradientslider.gadget -- create a slider with a range of colors as its
*				 background. (V39)
*
*   FUNCTION
*	The gradientslider gadget class is a type of non-proportional slider.
*	The primary feature of the gradient slider is it's appearance. Unlike
*	normal sliders, a gradient slider can display a "spread of colors" or
*	"color gradient" in the slider container box. The "knob" or "thumb"
*	of the slider appears to slide on top of this color gradient.
*
*	The color gradient effect is built-up using a combination of multiple
*	pens and half-tone dithering. The application must tell the slider
*	exactly which pens to use in creating the gradient effect, and in
*	what order to use them. Essentially, it does this by passing an
*	array of pens (terminated by ~0, just like a PenSpec) to the slider.
*	The first pen in the array is used as the color at the top of
*	the slider (or left, if it is horizontal), and the last color in the
*	array is used at the bottom (or right). The other pens will be
*	used at evenly spaced intervals in between. Dithering is used to
*	smoothly fade between the pens, allowing the illusion of a continuous
*	change in color.
*
*   TAGS
*	GRAD_MaxVal (ULONG) - Set the maximum value that the slider can
*		represent, in the range 0..$FFFF. Default for this tag is
*		$FFFF. Applicability is (ISGU). (V39)
*
*	GRAD_CurVal (ULONG) - Set or get the current value of the slider, in
*		the range 0..$FFFF. Default for this tag is 0. Applicability is
*		(ISGNU). (V39)
*
*	GRAD_SkipVal (ULONG) - Set the amount by which the slider should
*		advance whenever the user clicks on either side of the knob.
*		Default for this tag is $1111. Applicability is (ISGU).
*		(V39)
*
*	GRAD_KnobPixels (UWORD) - Set the size of the knob in pixels.
*		Default for this tag is 5. Applicability is (I).
*		(V39)
*
*	GRAD_PenArray (UWORD *) - Specifies an array of pens that the slider
*		should use to create its gradient background. The array can
*		contain any number of pens, and is terminated with a pen
*		value of ~0. These pens can be allocated as shared, since
*		their RGB value is not altered by the slider. The first pen
*		is used on the top or left of the slider, and the last pen is
*		used on the bottom or right. All other pens are evenly spaced
*		out and used in between. Dithering is used between the pens
*		to enhance the smoothness of the gradient transition.
*
*		A NULL pen array causes the background of the slider to be
*		rendered in the screen's background color. A pen array
*		containing only a single pen causes the background to be
*		rendered using that pen.
*
*		Default for this tag is NULL. Applicability is (ISU).
*		(V39)
*
*	PGA_FREEDOM (ULONG) - Determines the orientation of the slider. This
*		can be LORIENT_HORIZ for a horizontal slider, or LORIENT_VERT
*		for a vertical one. Default for this tag is LORIENT_HORIZ.
*		Applicability is (I). (V39)
*
******************************************************************************

