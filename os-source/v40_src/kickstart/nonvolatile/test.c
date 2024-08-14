/*****************************************************************************
*
*	$Id: test.c,v 40.9 93/04/08 14:19:24 brummer Exp Locker: vertex $
*
******************************************************************************
*
*	$Log:	test.c,v $
 * Revision 40.9  93/04/08  14:19:24  brummer
 * Add h for help command
 *
 * Revision 40.8  93/04/08  14:07:39  brummer
 * Fix x command (read rsvd data) and source indents
 *
 * Revision 40.7  93/03/29  11:26:11  brummer
 * Add option k/K to toggle kill requester setting.
 *
 * Revision 40.6  93/03/18  11:14:28  brummer
 * Add code for metascope testing that forces the APP/Item name to
 * something.  This code is in comment brackets until needed.
 *
 * Revision 40.5  93/03/11  10:08:06  brummer
 * add read and write of reserved data area.  However, read always gets
 * zeros.  This is a test problem, the code is working fine.
 *
 * Revision 40.4  93/03/08  18:05:41  brummer
 * Add KillReq parameter to the nonvolatile.library function calls
 *
 * Revision 40.3  93/02/25  08:55:04  Jim2
 * Works with the newer nonvolatile types.
 *
 * Revision 40.2  93/02/23  12:58:32  Jim2
 * Handles new return value type from GetNVList.
 *
 * Revision 40.1  93/02/18  11:08:50  Jim2
 * Prints delete protection for GetNVList.  Added two new command
 * to allow the setting and clearing of the delete protection.
 *
 * Revision 40.0  93/02/16  13:23:24  Jim2
 * Used the new structure for return values of the GetNVList.
 *
 * Revision 39.0  93/02/03  11:20:36  Jim2
 * Initial Release - Tested.
 *
*
*	(C) Copyright 1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************/
#include <exec/nodes.h>
#include <exec/tasks.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dosextens.h>
#include <utility/tagitem.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>

#include "nonvolatile.h"
#include <clib/nonvolatile_protos.h>
#include <pragmas/nonvolatile_pragmas.h>

#define EXECBASE (*(struct ExecBase **)4)
#define Rsvd_data_length 16
struct ExecBase *SysBase = NULL;

ULONG cmd(VOID)
{
	struct Library *DOSBase, *NVBase;
	SysBase = EXECBASE;
	DOSBase=OpenLibrary("dos.library", 0);
	if ((NVBase = OpenLibrary("nonvolatile.library",0)) != NULL)
	{
		Printf ("Opened library\n");
		if (TRUE)
		{
			UBYTE	Cmd,RsvdByte,AppName[64],*Walk,*Test1,ItemName[64],Data[64],Rsvddata[20];
			int		i;
			LONG 	DataSize;
			BOOL 	KillReq;
			struct	NVInfo	*Test;
			struct	MinList	*Test2;
			struct	NVEntry	*Walk2;

			Walk = Rsvddata;
		     for (i=0; i<Rsvd_data_length; i++) *Walk++ = i;
			KillReq = 1;
			FGetC(Input());
			do
			{
				Printf ("Cmd>");Flush(Output());
				switch (Cmd=FGetC(Input()))
				{
					case 'h' :
					case 'H' :
						FGetC(Input());
						Printf("\nCommands -\n");
						Printf("     d = Delete item\n");
						Printf("     k = toggles Killrequester parameter\n");
						Printf("     l = List items\n");
						Printf("     p = Protect item\n");
						Printf("     r = Read item\n");
						Printf("     w = Write item\n");
						Printf("     u = Unprotect item\n");
						Printf("     x = Read system reserved area\n");
						Printf("     y = Write system reserved area with pattern\n\n");
						break;
					case 'k' :
					case 'K' :
						FGetC(Input());
						KillReq = !KillReq;
						if (KillReq)
							Printf ("Requesters are disabled\n");
						else
							Printf ("Requesters are enabled\n");
						break;
					case 'l' :
					case 'L' :
						FGetC(Input());
						Printf ("List the contents of nonvolatile.\n Choose application:  "); Flush(Output());
						FGets(Input(),AppName,64);
/* AppName[0] = 'd'; AppName[1] = 'a'; AppName[2] = 'v'; AppName[3] = 'e'; AppName[4] = 10; */
						Walk = AppName;
						while (*Walk != 0)
						{
			    				if (*Walk == 10) *Walk = 0;
						    Walk++;
						}
						if (Walk != &(AppName[1]))
							Test2=GetNVList(AppName, KillReq);
						else
							Test2=GetNVList(NULL, KillReq);
						Walk2 = (struct NVEntry *) Test2-> mlh_Head;
						while (Walk2 != (struct NVEntry *) &(Test2-> mlh_Tail))
						{
							if ((Walk2->nve_Protection & NVEF_APPNAME) != 0) Printf("\tAppName=");
							Printf ("%s - %lx,%lc\n",Walk2->nve_Name, Walk2->nve_Size, (Walk2->nve_Protection & NVEF_DELETE) ? 'l' : ' ');
							Walk2= (struct NVEntry *) Walk2->nve_Node.mln_Succ;
			    			}
						FreeNVData(Test2);
						break;
					case 'd' :
					case 'D' :
						FGetC(Input());
						Printf ("Delete a nonvolatile entry.\n\tEnter the application name:  "); Flush(Output());
						FGets(Input(),AppName,64);
/* AppName[0] = 'w'; AppName[1] = 'h'; AppName[2] = 'i'; AppName[3] = 'z'; AppName[4] = 10; */
						Walk = AppName;
						while (*Walk != 0)
						{
							if (*Walk == 10)
							*Walk = 0;
							*Walk++;
						}
						Printf ("\tEnter the item name:  "); Flush(Output());
						FGets(Input(),ItemName,64);
/* ItemName[0] = 'k'; ItemName[1] = 'i'; ItemName[2] = 'd'; ItemName[3] = 10; */
						Walk = ItemName;
						while (*Walk != 0)
						{
							if (*Walk == 10)
							*Walk = 0;
							*Walk++;
						}
						DeleteNV(AppName, ItemName, KillReq);
						break;
					case 'p' :
					case 'P' :
						FGetC(Input());
						Printf ("Protection nonvolatile entry against deletion.\n\tEnter the application name:  "); Flush(Output());
						FGets(Input(),AppName,64);
						Walk = AppName;
						while (*Walk != 0)
						{
							if (*Walk == 10)
							*Walk = 0;
							*Walk++;
						}
						Printf ("\tEnter the item name:  "); Flush(Output());
						FGets(Input(),ItemName,64);
						Walk = ItemName;
						while (*Walk != 0)
						{
							if (*Walk == 10)
							*Walk = 0;
							*Walk++;
						}
						SetNVProtection(AppName, ItemName, 1, KillReq);
						break;
					case 'u' :
					case 'U' :
						FGetC(Input());
						Printf ("Unprotection nonvolatile entry against deletion.\n\tEnter the application name:  "); Flush(Output());
						FGets(Input(),AppName,64);
						Walk = AppName;
						while (*Walk != 0)
						{
							if (*Walk == 10)
							*Walk = 0;
							*Walk++;
						}
						Printf ("\tEnter the item name:  "); Flush(Output());
						FGets(Input(),ItemName,64);
						Walk = ItemName;
						while (*Walk != 0)
						{
							if (*Walk == 10)
							*Walk = 0;
							*Walk++;
						}
						SetNVProtection(AppName, ItemName, 0, KillReq);
						break;
					case 'r' :
					case 'R' :
						FGetC(Input());
						Printf ("Read a nonvolatile entry.\n\tEnter the application name:  "); Flush(Output());
						FGets(Input(),AppName,64);
						Walk = AppName;
						while (*Walk != 0)
						{
							if (*Walk == 10)
							*Walk = 0;
							*Walk++;
						}
						Printf ("\tEnter the item name:  "); Flush(Output());
						FGets(Input(),ItemName,64);
						Walk = ItemName;
						while (*Walk != 0)
						{
							if (*Walk == 10)
							*Walk = 0;
							*Walk++;
						}
						if ((Test1=GetCopyNV(AppName, ItemName, KillReq)) != NULL)
						{
							Printf ("Read >%s< from %s:%s\n",Test1,AppName, ItemName);
							FreeNVData(Test1);
						}
						break;
					case 'x' :
					case 'X' :
						FGetC(Input());
						Printf ("Read reserved entry :\n");
						Walk = AppName;
						*Walk = 0;
						Walk = ItemName;
						*Walk = 0;
						if ((Test1=GetCopyNV(AppName, ItemName, KillReq)) != NULL)
						{
							Walk = Test1;
							for (i=0; i<Rsvd_data_length; i++)
							{
								RsvdByte = *Walk++;
								Printf("  %lx", RsvdByte);
							}
							Printf ("\n");
							FreeNVData(Test1);
						}
						else
							Printf("NULL\n");
						break;
					case 'y' :
					case 'Y' :
						FGetC(Input());
						Printf ("Write reserved entry :\n");
						Walk = AppName;
						*Walk = 0;
						Walk = ItemName;
						*Walk = 0;
						switch (StoreNV(AppName, ItemName, Rsvddata, Rsvd_data_length, KillReq))
						{
							case 0 :
								Printf ("Stored\n");
								break;
							case NVERR_BADNAME:
								Printf ("Error in AppName, or ItemName\n");
								break;
							case NVERR_WRITEPROT:
								Printf ("Nonvolatile storage cannot be written\n");
								break;
							case NVERR_FAIL:
								Printf ("Failure in writing data (FULL)\n");
								break;
							case NVERR_FATAL:
								Printf ("Fatal error.  Possible loss of data\n");
								break;
							default :
								Printf ("Unknown return code\n");
						}
						break;
					case 'w' :
					case 'W' :
						FGetC(Input());
						Printf ("Write a nonvolatile entry.\n\tEnter the data (max 64 bytes):  ");Flush(Output());
						FGets(Input(),Data,64);
						Walk = Data;
						while (*Walk != 0)
						{
							if (*Walk == 10)
							*Walk = 0;
							*Walk++;
						}
						DataSize = Walk-Data;
						if (*--Walk != 0) DataSize++;
						Printf ("\tEnter the application name:  "); Flush(Output());
						FGets(Input(),AppName,64);
						Walk = AppName;
						while (*Walk != 0)
						{
							if (*Walk == 10)
							*Walk = 0;
							*Walk++;
						}
						Printf ("\tEnter the item name:  "); Flush(Output());
						FGets(Input(),ItemName,64);
						Walk = ItemName;
						while (*Walk != 0)
						{
							if (*Walk == 10)
							*Walk = 0;
							*Walk++;
						}
						if ((AppName[0] != 0) && (ItemName[0] != 0) && (DataSize > 1))
						{
							switch (StoreNV(AppName, ItemName, Data, ((DataSize+9)/10), KillReq))
							{
							case 0 :
								Printf("Stored\n");
								break;
							case NVERR_BADNAME:
								Printf ("Error in AppName, or ItemName\n");
								break;
							case NVERR_WRITEPROT:
								Printf("Nonvolatile storage cannot be written\n");
								break;
							case NVERR_FAIL:
								Printf ("Failure in writing data (FULL)\n");
								break;
							case NVERR_FATAL:
								Printf("Fatal error.  Possible loss of data\n");
								break;
							default :
								Printf("Unknown return code\n");
							}
						}
						else
							Printf("Invalid data, not saved\n");
						break;
					case 'i' :
					case 'I' :
						FGetC(Input());
						Test = GetNVInfo(KillReq);
						Printf ("Get nonvolatile size information\n\tMax=%lx bytes, Free=%lx bytes\n\n",Test->nvi_MaxStorage, Test->nvi_FreeStorage);
						FreeNVData(Test);
						break;
				}
			}
			while ((Cmd != 'Q') && (Cmd != 'q'));
		}
		CloseLibrary(NVBase);
		Printf ("Closed library\n");
	}
	CloseLibrary(DOSBase);
	return (0);
} /* main */
