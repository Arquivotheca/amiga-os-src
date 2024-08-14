/*******************************************************************
 *
 *  Copyright 1991, Hewlett-Packard Company
 *  Right to distribute this program, in source of object form, to
 *  commercial or individual users, is granted, provided the program
 *  is used strictly to improve print quality of Hewlett-Packard printers.
 *
 *  This program is distributed as is; no warranty for any particular
 *  purpose is expressed or implied.  Hewlett-Packard company will not
 *  provide support for this program.
 *
 * NAME
 *  GreyBalance()
 *
 * DESCRIPTION
 *  Grey balance an RGB triple for better print quality with HP51625A
 *  plain paper color print cartridge.
 *
 * "UnGreyness" = (max(c, m, y) - min(c, m, y)) / max(c, m, y).
 *    0 <= UnGreyness <= 1; 0 is grey, 1 is not grey.
 *                
 *  The arithmetic is performed with integer math to improve performance.
 *  
 *  We know a pretty good grey is 1 part M + 1 part Y + 2/3 part C.  The
 *  actual ratios will vary with print medium, halftoning technique,
 *  environment, personal preference, and other factors.  If 
 *  UnGreyness is 0, leave cyan alone.  As UnGreyness goes to 1, linearly
 *  reduce cyan to 2/3 of its original value.  This means the adjusted 
 *  value for cyan is:
 *      range = maxPart - minPart;
 *      cPart = cPart * (range/(maxPart*3) + 2/3);
 *      
 *  The biggest range can be is maxPart, so range/(maxPart*3) ranges from 
 *  0 to 1/3.  This means cPart is multiplied by a number between 2/3 and 1,
 *  thereby affecting the reduction in cyan.
 *  
 *  Conversion to integer suitable form is accomplished by multiplying 
 *  terms by maxPart*3 and dividing the whole mess by maxPart*3 later.  So,
 *  we have:
 *      cPart = cPart * (range + 2*maxPart) / maxPart*3;
 *
 *  Rounding is acheived by doubling the numerator, adding one, and 
 *  dividing by a doubled denominator.
 *  
 *  Gotta be careful here because subsequent gamma compensation will 
 *  cause these parts (computed in percent dots) to be interpreted 
 *  as (roughly) perceptual change in lightness.
 *  
 *  Let's assume the gamma compensaion curve sez requesting 
 *  4/5 part C will (after compensation) cause 2/3 of dots C 
 *  to be fired.  The equations become:
 *      cPart = cPart * (range/(maxPart*5) + 4/5)
 *  and
 *      cPart = cPart * (range + 4*maxPart) / maxPart*5;
 *
 * ASSUMPTIONS
 *  Subsequent gamma compensation uses a particular schedule.  If 
 *  different, multiple, or no gamma compensation curves are used, the
 *  code will have to be modified.
 *
 *  Byte is defined as an unsigned char (8 bits).
 *
 *  A short has 16 bits.
 *
 *  A long has 32 bits.
 *
 * INPUTS
 *  Pointers to RGB triple to be balanced.  Input values range from 
 *  0 (minimum intensity) to 255.
 *
 * OUTPUTS
 *  Input RGB triple is replaced with a grey balanced RGB triple.  Actually,
 *  only cyan (red) is affected; green and blue are not altered.
 *
 * LIMITATIONS or DEPENDENCIES
 *  Use of register variables could improve performance with some compilers.
 *  This code has not been optimized for speed.
 *
 *  This version has never been compiled.  What you get is what you get.
 *
 * AUTHOR
 *  willa -- Will Allen, Hewlett-Packard VCD
 *
 * SEE ALSO
 *  Computer Graphics Principles and Practice
 *  Foley, van Dam, et al.
 *  Addison-Wesley
 *  ISBN 0-201-12110-7
 *
 *  Pages neighboring 592, especially RGB_To_HSV() routine.  My concept of
 *  "UnGreyness" is the same as the S (saturation) in HSV.
 *
 * HISTORY
 *  willa   Winter 90-91    Developed and refined based on floating point
 *                          arithmetic.
 *
 *  willa   April 3, 1991   Changed to integer arithmetic for improved 
 *                          performance.
 *
 *  willa   July 25, 1991   Original release of version for third party
 *                          support of HP printers.
 *
 *  willa   Aug 22,  1991   Cleaned up third party version a bit.
 *
 ******************************************************************/


#define RGB_MAX     (255)           /* Largest value for inputs */


void GreyBalance(Byte *red, Byte *green, Byte *blue) {  
                                      
    /* LOCALS */                                     
    short
        cPart, mPart, yPart;        /* CMY space version of inputs */

    short
        maxPart,                    /* largest of cPart, mPart and yPart */
        minPart;                    /* smallest of cPart, mPart and yPart */
    
    long
        range;                      /* range of cPart, mPart, and yPart */
                                    /* (max - min)                      */
        
        
    /* CODE */
    
    /* Special case white because it's so common, no balancing necessary */
    if ((*red & *green & *blue) == (Byte)0xFF) {
        return;
    }
    
    /* Convert to CMY */
    cPart = RGB_MAX - *red;
    mPart = RGB_MAX - *green;
    yPart = RGB_MAX - *blue;
        
    /* Determine max, min, range, and balance */
    maxPart = cPart > mPart ? cPart : mPart;
    maxPart = yPart > maxPart ? yPart : maxPart;
    if (maxPart > 0) {          /* watch out for white, don't divide by 0 */
        minPart = cPart < mPart ? cPart : mPart;
        minPart = yPart < minPart ? yPart : minPart;
        range = maxPart - minPart;
        
        /*  Double numerator, add 1, and double denominator for rounding.
            Use long arithmetic to prevent overflow.
        */
        cPart = 
            ((long)(cPart*2) * (range + (long)(4*maxPart)) + 1) / 
            (long)(maxPart*10);
        
        /* Only cyan (red) gets adjusted, so don't alter green and blue */
        *red  = RGB_MAX - cPart;
    }
} /* GreyBalance() */
