#ifndef IPREFS_H
#define IPREFS_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


#define IPREFSNAME "« IPrefs »"


#define SignalParent() if (parentTask) Signal(parentTask,SIGF_CHILD);

#pragma libcall IntuitionBase SetIPrefs 240 10803
APTR SetIPrefs(APTR ptr, LONG size, LONG type );

#pragma libcall LocaleBase SetDefaultLocale a8 801
struct Locale *SetDefaultLocale(struct Locale *locale);

#pragma libcall WorkbenchBase WBConfig 54 1002
ULONG WBConfig(unsigned long tag, unsigned long value);


/*****************************************************************************/


#endif /* IPREFS_H */
