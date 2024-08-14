#ifndef	MATHRESOURCE_H
#define MATHRESOURCE_H

#ifndef	EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_NODES_H
#include <exec/nodes.h>
#endif

/*
*       The 'Init' entries are only used if the corresponding
*       bit is set in the Flags field.
*
*       So if you are just a 68881, you do not need the Init stuff
*       just make sure you have cleared the Flags field.
*
*       This should allow us to add Extended Precision later.
*
*       For Init users, if you need to be called whenever a task
*	opens this library for use, you need to change the appropriate
*	entries in MathIEEELibrary.
*/

struct MathIEEEResource
{
	struct	Node	MathIEEE_Node;
	unsigned short	MathIEEE_Flags;
	unsigned short	*MathIEEE_BaseAddr;	/* ptr to 881 if exists */
	void	(*MathIEEE_DblBasInit)();
	void	(*MathIEEE_DblTransInit)();
	void	(*MathIEEE_SglBasInit)();
	void	(*MathIEEE_SglTransInit)();
	void	(*MathIEEE_ExtBasInit)();
	void	(*MathIEEE_ExtTransInit)();
}

/* definations for MathIEEE_FLAGS */
#define	MATHIEEEF_DBLBAS	(1<<0)
#define	MATHIEEEF_DBLTRANS	(1<<1)
#define	MATHIEEEF_SGLBAS	(1<<2)
#define	MATHIEEEF_SGLTRANS	(1<<3)
#define	MATHIEEEF_EXTBAS	(1<<4)
#define	MATHIEEEF_EXTTRANS	(1<<5)

#endif RESOURCES_MATHRESOURCE_H

