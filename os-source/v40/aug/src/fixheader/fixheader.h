#define TABSIZE (8)
#define MAXINPUTLINELEN     (1024)
#define	FALSE		    (0)
#define TRUE		    (!FALSE)

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

void ProcessFile ( FILE *Infile, FILE *Outfile );
void Cleanup ( void );
int entab ( char *Inbuffer );
char *ltoa ( char *String, long	LongVal );
void exit ( int code );
