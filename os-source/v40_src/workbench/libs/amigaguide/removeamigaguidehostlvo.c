/* RemoveAmigaGuideHostLVO.c
 *
 */

#include "amigaguidebase.h"

/****** amigaguide.library/RemoveAmigaGuideHostA ****************************
*
*    NAME
*	RemoveAmigaGuideHostA - Remove a dynamic node host.     (V34)
*
*    SYNOPSIS
*	use = RemoveAmigaGuideHostA (key, attrs)
*	d0			     a0   a1
*
*	LONG RemoveAmigaGuideHostA (AMIGAGUIDEHOST, struct TagItem *);
*
*	use = RemoveAmigaGuideHost (key, tag1, ...);
*
*	LONG RemoveAmigaGuideHost (AMIGAGUIDEHOST, Tag, ...);
*
*    FUNCTION
*	This function removes a dynamic node host, that was added by
*	AddAmigaGuideHost(), from the system.
*
*    INPUTS
*	key - Key that was returned by AddAmigaGuideHost().
*
*	attrs - Additional attributes.  None are defined at this time.
*
*    RETURNS
*	use - Number of outstanding clients of this database.  You
*	    can not exit until use==0.
*
*    SEE ALSO
*	AddAmigaGuideHostA()
*
**********************************************************************
*
* Created:  03-Aug-90, David N. Junod
*
*/

/* see stubs.c */
