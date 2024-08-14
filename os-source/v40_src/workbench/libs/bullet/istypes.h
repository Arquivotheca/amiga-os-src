/*
**
** File: ISTYPES.H
**
**
*/
/*
*  Copyright (C) 1990, All Rights Reserved, by
*  Isogon Corporation, New York, NY.
*
*  This software is furnished under a license and may be used and copied
*  only	in accordance with the terms of such license and with the
*  inclusion of the above copyright notice. This software or any other
*  copies thereof may not be provided or otherwise made available to any
*  other person. No title to and ownership of the software is hereby
*  transferred.
*
*  The information in this software is subject to change without notice
*  and should not be construed as a commitment by Isogon Corporation or
*  Compugraphic Corporation.
*
*
*/




/*
**  Request type definitions
*/

enum IS_REQ_TYPE
{
  IS_EXP_INIT,                          /* IntelliSpace expand init. */
  IS_EXP_TERM,                          /* IntelliSpace expand cleanup */
  IS_EXP_FILEHDRSEG,                    /* Expand File Header Segment */
  IS_EXP_FACEGLOBSEG,                   /* Expand Face Global Segment */
  IS_EXP_CHARSEG,                       /* Expand Character Segment */
  IS_EXP_FACEHDRSEG,                    /* Expand Face Header Segment */
  IS_EXP_FILEDIR                        /* Expand File Directory */
};



/*
**  Function prototypes
*/

/** The following ones are callable from other IntelliFont routines **/

/* IS_COMPRESS_FILE and IS_EXPAND_FILE process a complete library file */

UWORD is_compress_file(BYTE * filename);
UWORD is_expand_file(BYTE * filename);

/* IS_EXPAND processes segments within a compressed library file */

UWORD is_expand(int handle, ULONG position, UBYTE * exp_buffer,
                UWORD exp_buffer_size, enum IS_REQ_TYPE exp_type);



/* The following ones are only callable from within IntelliSpace routines */

UWORD is_compr_file_hdr_seg(UBYTE * in, UWORD size, UBYTE ** out);
UWORD is_compr_face_glob_seg(UBYTE * in, UWORD size, UBYTE ** out, UWORD numchars);
UWORD is_compr_char_seg(UBYTE * in, UWORD size, UBYTE ** out);
UWORD is_compr_face_hdr_seg(struct lib_x_header * in, UWORD size, UBYTE ** out);
UWORD is_compr_file_dir(struct lib_x_file * in, UWORD size, UBYTE ** out);

UWORD is_exp_file_hdr_seg(UBYTE * in, UWORD size, UBYTE ** out);
UWORD is_exp_face_glob_seg(UBYTE * in, UWORD size, UBYTE ** out);
UWORD is_exp_char_seg(UBYTE * in, UWORD size, UBYTE ** out);
UWORD is_exp_face_hdr_seg(struct lib_x_header * in, UWORD size, UBYTE ** out);
UWORD is_exp_file_dir(struct lib_x_file * in, UWORD size, UBYTE ** out);
