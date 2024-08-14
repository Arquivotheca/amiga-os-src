/*
 * Amiga Grand Wack
 *
 * parse.h -- Definitions related to parsing.
 *
 * $Id: parse.h,v 39.1 93/06/02 16:09:50 peter Exp $
 *
 */

#include <exec/types.h>

/* Maximum length of token we parse out.  Buffers to parseStringArg()
 * should have this many bytes in them:
 */
#define MAXTOKENLEN	64
