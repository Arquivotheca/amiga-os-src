/*
 *		xfunctions.h
 *
 * 	xfunctions.h - function prototypes for xlib functions               
 * 	Written January 1990 by Joe Pearce                                  
 * 	Version for Installer & Lattice - December 1990                     
 *
 *
 *	Revision History:
 *
 *	lwilton	07/11/93:
 *		General cleanup and reformatting to work with SAS 6.x and the
 *		standard header files.
 *	lwilton 07/27/93:
 *		Added a number of missing functions from various assembler 
 *		modules.  Also improved formatting for readability.
 */



#ifndef __XFUNCTIONS_H 
#define __XFUNCTIONS_H

#ifndef EXEC_TYPES_H
#include <devices/keymap.h>
#include <graphics/rastport.h>
#include <graphics/text.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#endif


	/* inst.asm */

long __stdargs CmpMemI(void *,void *,LONG);
void __stdargs or_mem(void *,void *,int);
void __stdargs swap_mem(void *a, void *b, LONG size);
long __stdargs a_to_b(char *);

	/* match.asm */

int __stdargs str_mchi(char *,char *);

	/* patch.asm */

LONG __stdargs mySetSoftStyle(struct RastPort *, LONG, LONG);

	/* pmatch.asm */

LONG __stdargs PatternParse(char *,char *,LONG);
LONG __stdargs PatternMatch(char *,char *);

	/* xliba.asm */

long __stdargs a_to_x(char *);


	/* clamp.asm */

long __stdargs clamp(long,long,long);

	/* TransferList.c */

void TransferList(struct List *,struct List *);

	/* zero.asm */

void __stdargs zero(void *,long);

	/* MemAlloc.c */

void *MemAlloc(long,long);
void MemFree(void *);
void AllMemFree(void);
void *RasterAlloc(long,long);
long MemAllocList(void *);

	/* GetFileSize.c */

LONG GetFileSize(char *);

	/* str_rindex.asm */

char * __stdargs str_rindex(char *,long);

	/* str_sub.asm */

char * __stdargs str_sub(char *,char *);

	/* tackon.asm */

long __stdargs TackOn(char *,char *,ULONG);

	/* tolower.asm */

/* char __stdargs tolower(long); */

	/* toupper.asm */

/* char __stdargs toupper(long); */

	/* Sprintf.asm */

int __stdargs Sprintf(char *,char *,...);

	/* str_cat.asm */

char * __stdargs str_cat(char *,char *);
char * __stdargs str_ncat(char *,char *,ULONG);

	/* str_cmpi.asm */

LONG __stdargs str_cmpi(char *,char *);
LONG __stdargs str_ncmpi(char *,char *,ULONG);

	/* str_index.asm */

char * __stdargs str_index(char *,long);

	/* str_lwr.asm */

char * __stdargs str_lwr(char *);

	/* NextNode.asm */

void * __stdargs NextNode(void *);

	/* path.asm */

ULONG __stdargs ExpandPath(struct FileLock *,char *,ULONG);

	/* Printf.asm	*/

ULONG __stdargs VSprintf(char *,char *,void *);
ULONG __stdargs VFprintf(void *,char *,void *);
ULONG __stdargs Fprintf(void *,char *,...);
ULONG __stdargs PrintF(char *,...);

	/* Puts.asm	*/

ULONG __stdargs Fputs(void *,char *);
ULONG __stdargs Puts(char *);

	/* SendPacket.asm */

void __stdargs SendPacket(void *,struct StandardPacket *);

	/* InitPacket.asm */

void __stdargs InitPacket(long,struct StandardPacket *,void *);

	/* InitSimplePacket.asm */

void __stdargs InitSimplePacket(long,struct StandardPacket *);

	/* DiskType.asm */

LONG __stdargs DiskType(struct DeviceNode *);

	/* l_to_a.asm */

char * __stdargs l_to_a(long,char *);

	/* MakeBitMap.asm */

long __stdargs MakeBitMap(struct BitMap *,long,long,long);
void __stdargs UnMakeBitMap(struct BitMap *);

	/* enviroment.c */

long GetDEnv(char *,char *,long);
long SetDEnv(char *,char *);

	/* FileOnly.asm */

char * __stdargs FileOnly(char *);

	/* GetFileDate.asm */

long __stdargs GetFileDate(char *,struct DateStamp *);

	/* GetHead.asm */

void * __stdargs GetHead(void *);

	/* Gets.asm */

long __stdargs Fgets(void *,char *,long);
long __stdargs Gets(char *,long);

	/* GetTail.asm */

void * __stdargs GetTail(void *);

	/* a_to_l.asm */

long __stdargs a_to_l(char *);

	/* CpyBstrCstr.asm */

char * __stdargs CpyBstrCstr(char *,char *);

	/* CpyCstrBstr.c */

char *CpyCstrBstr(char *,char *);

#endif
