/*
*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************
*/


		/*	Private device options.  	*/



#define NDB_RTNPARMS	4		/* Return pointer to Parms data  */
#define	NDB_KEEPCOEF	5		/* Return COEF buffer		 */
#define NDB_KEEPSYNTH	6		/* Return SYNTH buffer		 */
#define NDB_DEBUG	7		/* Run synthesizer in debug mode */


#define NDF_RTNPARMS	(1 << NDB_RTNPARMS)
#define NDF_KEEPCOEF	(1 << NDB_KEEPCOEF)
#define NDF_KEEPSYNTH	(1 << NDB_KEEPSYNTH)
#define NDF_DEBUG	(1 << NDB_DEBUG)
