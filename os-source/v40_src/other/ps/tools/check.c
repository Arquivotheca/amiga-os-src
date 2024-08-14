//***********************************************************************
// Check.c 	(C) JULY 1991 Commodore-Amiga
// -------	-----------------------------
//***********************************************************************

#include	"stdio.h"
#include	"proto/dos.h"
#include	"exec/memory.h"

#define LF	0x0A

//------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------

BOOL load_file		(char * fname, int * size, char **buffer);
extern BOOL equal_blocks	(char *block1, char * block2, int size);

//-----------------------------------------------------------------------
void main() {

 register char * fname2, *ptr;
 char *script, *scriptbase;
 char *f1base;
 char *f2base;
 int scriptsize, f1size, f2size;

 printf("Quick Check V1.0 (C) 1991 Commodore-Amiga.\n");

 if ( ! load_file ("ps:script", &scriptsize, &scriptbase) ) {
	printf("Couldn't find script file on PS: !\n");
	exit(10);
 }

 script = scriptbase;

 while (*script != LF) {		// while not end-of-file,

	ptr = script+1;
	while (*ptr++ != ' ') ;
	*(ptr - 1) = '\0'; 			//zero terminate filename 1
	fname2 = ptr;

	while (*ptr++ != LF) ;
	*(ptr - 1) = '\0'; 			//zero terminate filename 2

	if (! load_file( script, &f1size, &f1base) ) {		// load file 1
		printf("Failed to load file [%s]\n",script);
		exit(10);
	}

	if (! load_file( fname2, &f2size, &f2base) ) {		// load file 2
		printf("Failed to load file [%s]\n",script);
		exit(10);
	}

	script = ptr;				// point to next line for next pair of files

	if (f1size == f2size) {

		if (! equal_blocks(f1base, f2base, f1size) )
			printf ("File %s differs.\n", fname2);

		FreeMem (f2base, f2size);
		FreeMem (f1base, f2size);		// f1size is unusable !

	} else {

		printf ("File %s isn't the same size.\n", fname2);
		FreeMem (f2base, f2size);
		FreeMem (f1base, f1size);
	}
 }

 printf ("\nQuick Check DONE! (have a fast day !) \n");
 
}
//-----------------------------------------------------------------------

/*

BOOL equal_blocks (char *block1, char * block2, int size) {

 register char *f1 = block1;
 register char *f2 = block2;
 register int i = size;

 while (i--) {
	if (*f1++ != *f2++) return FALSE;
 }

 return TRUE;

}

*/

//-----------------------------------------------------------------------
BOOL load_file(char *fname, int *size, char **buffer) {

int fhandle;

	fhandle = Open( fname, MODE_OLDFILE);
	if (! fhandle) {
		printf("Could not open file %s\n", fname);
		exit(10);
	}

	Seek(fhandle, 0, OFFSET_END);
	*size = Seek(fhandle, 0, OFFSET_BEGINNING);

	*buffer = (char *) AllocMem(*size, MEMF_CLEAR);
	if (! *buffer) {
		Close ( fhandle );
		return FALSE;
	}

	Read( fhandle, *buffer, *size);		// cache whole file
	Close( fhandle );

	return TRUE;
}
//-----------------------------------------------------------------------
