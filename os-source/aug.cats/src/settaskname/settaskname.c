;/* settaskname.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 settaskname.c
Blink FROM LIB:c.o,settaskname.o TO settaskname LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <intuition/intuition.h>

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

char *vers = "\0$VER: settaskname 36.11";
char *Copyright = 
  "settaskname v36.11\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";

int strLen(char *s);
void strCpy(char *to, char *from);

void main(int argc, char **argv)
	{
	struct Task *task;
	UBYTE *s, old[80];
	int n,k;

	if((argc < 2)||(argv[1][0] =='?'))
	    {
	    printf("usage: settaskname newname (will be truncated to length of old name)\n");
	    exit(0L);
	    }

	Disable();
	task = FindTask(NULL);
	s = task->tc_Node.ln_Name;

	n = strLen(s);
	strCpy(old,s);
	for(k=0;(k<=n) && (argv[1][k]);k++) s[k] = argv[1][k];
	s[k] = '\0';
	Enable();

	printf("old name: %s\n",old);
	printf("new name: %s\n",s);
	if(!n) printf("note - new name length must be <= old name length\n"); 
	exit(0L);
	}

int strLen(char *s)
   {
   int i = 0;
   while(*s++) i++;
   return(i);
   }


void strCpy(char *to, char *from)
   {
   do
      {
      *to++ = *from;
      }
   while(*from++);
   }

