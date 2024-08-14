/*
	SetCPU V1.5
	by Copyright 1989 by Dave Haynie

	STRING FUNCTION MODULE 

	This module contains some basic string functions.
*/

#include "setcpu.h"

/* ====================================================================== */

/* This replaces the Lattice "stricmp()" function, plus it's a better form
   for my needs here. */
   
BOOL striequ(s1,s2)
char *s1,*s2;
{
   BOOL aok = FALSE;
   
   while (*s1 && *s2 && (aok = (*s1++ & 0xdf) == (*s2++ & 0xdf)));
   return (BOOL) (!*s1 && !*s2 && aok);
}

BOOL strniequ(s1,s2,n)
char *s1,*s2;
unsigned n;
{
   BOOL aok = FALSE;
   
   while (n-- && *s1 && *s2 && (aok = (*s1++ & 0xdf) == (*s2++ & 0xdf)));
   return (BOOL) aok;
}
