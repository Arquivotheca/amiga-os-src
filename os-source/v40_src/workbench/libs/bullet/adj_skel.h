/*
*  adjust_skel.h
*
*  Copyright (C) 1987, All Rights Reserved, by
*  Compugraphic Corporation, Wilmington, Ma.
* 
*  This software is furnished under a license and may be used and copied
*  only	in accordance with the terms of such license and with the
*  inclusion of the above copyright notice. This software or any other
*  copies thereof may not be provided or otherwise made available to any
*  other person. No title to and ownership of the software is hereby
*  transferred.
* 
*  The information in this software is subject to change without notice
*  and should not be construed as a commitment by Compugraphic
*  Corporation.
* 
* 
*  History:
*  ---------
*    4-Feb-88    changed num_seg_p_loop from UWORD to WORD (tbh)
*   23-Jan-88    incorporated intermed in adjust_skel_type struct (tbh)
*    1-Nov-87    Initial Release
* 
*/

/*      IC1.0           Adjusted Skeletal Data                        */

typedef struct
        {
        WORD    original;       /*  original value of skel point      */
        WORD    intermed;	/*  intermediate "original" value     */
        WORD    adjusted;       /*  current abs skel point coord valu */

        WORD    pixel_grid_line;
                                /*  index of pixel (grid line number) */
                                /*  ( -1 indicates skel not aligned   */

        UBYTE   processing_status;
                                /*  indicator of whether skel point   */
                                /*  has been processed to final loc-  */
                                /*  ation ( = 1) of not ( = 0)        */
        UBYTE   parent;         /*  assoc'd point's skeletal parent   */
        UBYTE   round_i;        /*  "R" type index for association    */

        }  adjusted_skel_type;
