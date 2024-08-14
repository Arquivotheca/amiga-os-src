*	TTL    '$Id: endmodule.asm,v 1.2 90/07/27 02:19:05 bryce Exp $'
**********************************************************************
*								     *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.	     *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030				     *
*								     *
**********************************************************************
*
*   Source Control
*   --------------
*   $Id: endmodule.asm,v 1.2 90/07/27 02:19:05 bryce Exp $
*
*   $Locker:  $
*
*   $Log:	endmodule.asm,v $
*   Revision 1.2  90/07/27  02:19:05  bryce
*   The #Header line is a real pain; converted to #Id
*   
*   Revision 1.1  90/04/06  19:23:35  daveb
*   for rcs 4.x header change
*   
*   Revision 1.0  87/08/21  17:26:54  daveb
*   added to rcs
*   
*   Revision 1.0  87/07/29  09:52:28  daveb
*   added to rcs
*   
*   Revision 25.0  85/06/13  18:53:18  kodiak
*   added to rcs
*   
*
**********************************************************************

	SECTION		printer

	XDEF	PEndModule
PEndModule:
		DS.W    0

		END
