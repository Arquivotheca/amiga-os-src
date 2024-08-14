#ifndef LIBRARIES_MATHLIBRARY_H
#define LIBRARIES_MATHLIBRARY_H 

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif

struct MathIEEEBase
{
	struct Library MathIEEEBase_LibNode;
	unsigned char	MathIEEEBase_Flags;
	unsigned char	MathIEEEBase_reserved1;
	unsigned short	*MathIEEEBase_68881;
	APTR	MathIEEEBase_Syslib;
	APTR	MathIEEEBase_SegList;
	struct	MathIEEEResource *MathIEEEBase_Resource;
	int	(*MathIEEEBase_TaskOpenLib)();
	int	(*MathIEEEBase_TaskCloseLib)();
	/*	This structure may be extended in the future */
};
/*
* MathResources may need to know when a program opens or closes this
* library. The functions TaskOpenLib and TaskCloseLib are called when 
* a task opens or closes this library. They are initialized to point to
* local initialization pertaining to 68881 stuff if 68881 resources 
* are found. To override the default the vendor must provide appropriate 
* hooks in the MathIEEEResource. If specified, these will be called 
* when the library initializes.
*/

#endif LIBRARIES_MATHLIBRARY_H
