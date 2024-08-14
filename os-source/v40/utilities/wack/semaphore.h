/* $Id: semaphore.h,v 1.2 91/04/24 20:53:49 peter Exp $ */

#ifndef GRAPHICS_SEMAPHORE_H
#define GRAPHICS_SEMAPHORE_H
#define SEMAPHORE_SIGNAL 4

#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif

struct JSemaphore
{
	struct List Q;
	SHORT	NestCount;
	struct Task *Owner;
	SHORT	QCount;
};
#endif
