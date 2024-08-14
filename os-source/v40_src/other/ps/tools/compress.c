//***********************************************************************
// Compress.c 	(C) JULY 1991 Commodore-Amiga
// ----------	-----------------------------
// This Amiga Postscript build utility takes a description of Postscript
// system names (and operators) and generates another 'C' file from it which
// has all this information in a compressed format.
// INPUT file : sysenc.dat in current directory
// OUTPUT file: systemnames.c in current directory
//
// The compression algorithm basically creates a stream of substrings from
// an alphabetically sorted stream of input strings.
// The compressed output is structured as follows:
// CODE,c1,c2,c3...,cn		(c1..cn are unique tail-end characters in string)
// CODE = $80+N where N is 0..31 and means N head chars of this string are
// common with the N first chars in the previous string
// Since the compressed output is in the form of a 'C' file and since an
// array of operator function pointers and external declarations are required,
// 3 separate files are build and then JOINed together before exiting.
//***********************************************************************

#include	"stdio.h"
#include	"proto/dos.h"
#include	"exec/memory.h"

#define	OUT_FNAME	"systemnames.c"

#define TAB 0x09
#define LF	0x0A

// Escape character bit fields
#define	SPECIAL		0x80
#define	INDEX		0x40
#define	OPERATOR	0x20
#define	COMMON		0x1F // 5 bits set aside to tell howmany common chars
						 // with start of previous string

//------------------------------------------------------------------------
// Globals
//------------------------------------------------------------------------

char * text;		// start of file cache buffer
char * ptr;			// running ptr to input file
int fsize,buffer;

int fh1,fh2,fh3;	// file handles for parallel files

//------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------

BOOL load_file( char *);
BOOL sort_file();
BOOL prime_file(char * fname, char* str, int* fh);
int  getnumber();
unsigned char hexchar( unsigned char c);
BOOL write_byte( int file, unsigned char byte);
void write_char(int fh, unsigned char c);

//-----------------------------------------------------------------------
void main() {

 char charbuf;
 char *name,*lastname,*funcname;
 char ch;
 BOOL operator,firsttime=TRUE,firstvec=TRUE;
 int index,i;

// The input file needs to be sorted for the compression algorithm to work...
	printf("Sorting sysenc.dat...\n");
	if( ! sort_file()) exit(10);

// Cache the entire file into RAM
	if( ! load_file("ram:sorted") ) exit(20);

// Open the 3 parallel files and give them their required headers.
	prime_file ("ram:encdat", "unsigned char encdat[]={\n",	&fh1);
	prime_file ("ram:vecdat", "void *opvectors[]={\n", &fh2);
	prime_file ("ram:import", "// External function references\n", &fh3);

	printf("Generating 'C' file with compressed names..\n");

	ptr = text;

// Skip all comment lines and empty lines (which were stuffed at the beginning
// when sorting the input file.

	while (*ptr=='/' || *ptr==LF)	// any line that starts with // or LF
		while (*ptr++ != LF) ;		// skip to EOL

//---------------------------------------------------------------------
// Parse a line. get pointers to strings and get numeric values.

	while (ch = *ptr) {				// while not EOF (which is \0 )

		name = ptr;					// save ptr to start of system name

		while (*ptr++ != ',') ;		// skip to operator/name flag
		operator = *ptr++ - '0';	// 1/2 = operator, 0 = name

		while (*ptr++ != ',') ;
		index = getnumber();		// get index

		if (operator == 2) {		// get optional operator name if type=2
			while (*ptr++ != ',') ;
			funcname = ptr;			// remember ptr to function name
		}

		while (*ptr++ != LF) ;		// skip to EOL

// AT this point we've got all the information for one entry, convert and
// compress this info and append to the 3 parallel files.

//		printf ("Name='%.5s' Type=%d, Index=%d \n",name,operator,index);


// The very first entry doesn't have any characters in common with any previous
// strings, so output first string completely.

	if (firsttime) {

			firsttime = FALSE;

			charbuf = SPECIAL;
			if (operator)
				charbuf |= OPERATOR;
			if (index <256 ) 
				charbuf |= INDEX;

			write_byte(fh1, charbuf);
			write_char (fh1, ',');				// delimit bytes with commas

			if (index <256) {
				write_byte(fh1, (unsigned char) index);
				write_char (fh1, ',');			// delimit bytes with commas
			}

			lastname = name;
			write_name(fh1, name);

			if(operator) {					// if name is an operator name,
				if (operator==1) {

// in file 2, add an entry like "ps_setdash,"

					write_string(fh2, "ps_");	// add ps_name entry to parallel
					write_string(fh2, name);	// file

// in file 3, add an entry like "extern void ps_setdash();"

					write_Cstring(fh3, "extern void ps_");
					write_string	(fh3, name);
					write_Cstring	(fh3, "(void);\n");

				} else if (operator==2) {
					write_string(fh2,funcname);

					write_Cstring(fh3, "extern void ");
					write_string(fh3, funcname);
					write_Cstring(fh3, "(void);\n");
				}
			}

	} else {

			write_char (fh1, ',');				// delimit bytes with commas
			write_char (fh1, LF);				// and a new line

			for (i=0; i< 200; i++) {		// find howmany chars prev and curr
				if (name[i] != lastname[i]) break;	// strings have in common
			}

			charbuf = SPECIAL | i;				// construct code byte
			if (operator)
				charbuf |= OPERATOR;			// OR in operator flag
			if (index <256 ) 
				charbuf |= INDEX;				// OR in index present flag

			write_byte(fh1, charbuf);
			write_char (fh1, ',');				// delimit bytes with commas

			if (index <256) {					// write out index byte if nec.
				write_byte(fh1, (unsigned char) index);
				write_char (fh1, ',');			// delimit bytes with commas
			}

			lastname = name;					// keep ptr to string for next time
			write_name(fh1,name+i);

			if(operator) {
				if (!firstvec) {
					write_char(fh2,',');
					write_char(fh2,LF);
				}
				firstvec = FALSE;

				if (operator==1) {
					write_string(fh2, "ps_");	// add ps_name entry to parallel
					write_string(fh2, name);	// file

					write_Cstring(fh3, "extern void ps_");
					write_string	(fh3, name);
					write_Cstring	(fh3, "(void);\n");

				} else if (operator==2) {
					write_string(fh2,funcname);

					write_Cstring(fh3, "extern void ");
					write_string(fh3, funcname);
					write_Cstring(fh3, "(void);\n");
				}
			}
	}
}

// Finish off encoded names file
	write_Cstring(fh1, ",\n0 };  // end of table\n\n"); // add a zero to end table.
	Close(fh1);
// Finish off operator function address array file
	write_char(fh2,'}'); write_char(fh2,';'); write_char(fh2,LF); Close(fh2);
// Finish external definitions file
	write_char(fh3,LF); Close(fh3);

//--------------------------------------------------------------------
// At this point we've created 3 valid 'C' files containing two different
// arrays and a list of external declarations.
// Merge these 3 files into one.

	fh1 = Open(OUT_FNAME,MODE_NEWFILE);

	load_file("ram:encdat");
	Write(fh1, text, fsize);

	load_file("ram:import");
	Write(fh1, text, fsize);

	load_file("ram:vecdat");
	Write(fh1, text, fsize);

	Close (fh1);

	FreeMem(text,buffer);

	printf("Compressed file made.\n");
}
//-----------------------------------------------------------------------
int getnumber( void ) {

	int accum =0;
	char c;

	while (*ptr == ' ' || *ptr == TAB)			// skip white space
		ptr++;

	while (TRUE) {		
		c= *ptr++;								// get character
		if (c >= '0' && c <= '9')
			accum = accum*10 + (c - '0');
		else break;
	}

	ptr--;					// point back to offending char

	return accum;
}
//-----------------------------------------------------------------------
BOOL load_file(char * fname) {

int fhandle;

	fhandle = Open( fname, MODE_OLDFILE);

	Seek(fhandle,0,OFFSET_END);
	fsize = Seek(fhandle,0,OFFSET_BEGINNING);
	buffer = 2*fsize;

	if (!text) {
		text = (char *) AllocMem(buffer, MEMF_CLEAR);
		if (!text) {
			Close (fhandle);
			return FALSE;
		}
	}

	Read(fhandle,text,fsize);		// cache whole file

	Close(fhandle);

	return TRUE;
}
//-----------------------------------------------------------------------
// This routine just creates a sorted version of the data file

BOOL sort_file() {

	int error;

	error = Execute("c:sort sysenc.dat TO ram:sorted",0,0);

	if (error != -1 ) {
		printf("Execute() returned %d\n",error);
		return FALSE;
	}

	return TRUE;
}
//-----------------------------------------------------------------------
BOOL prime_file(char * fname, char* str, int* fh) {

	int size,num;

	*fh = Open(fname ,MODE_NEWFILE);
	size = strlen(str);

	num = Write( *fh, str, size);	
	if (num != size) {
		printf ("Error priming file %s\n", fname);
		return FALSE;
	}

	return TRUE;
}
//-----------------------------------------------------------------------
BOOL write_byte( int fh, unsigned char byte) {

	write_char(fh, '0');
	write_char(fh, 'x');
	write_char(fh, hexchar(byte>>4));
	write_char(fh, hexchar(byte & 0x0f) );

	return TRUE;
}
//-----------------------------------------------------------------------
unsigned char hexchar( unsigned char c) {

	return (unsigned char) (c>9 ? ('A'+ c-10) : ('0'+c));

}
//-----------------------------------------------------------------------
// Write out a string terminated by white-space (incl LF) or a comma as
// single comma-separated chars.
// E.g. name -> 'n','a','m','e'

write_name(int fh, char *str) {

	int i=0;
	char * base;
	register char c;

	base = str;

	c = *str;
	while ( c !=' ' && c !=TAB && c !=',' && c !=LF) {
		i++;
		c = *(++str);
	}

	while (i--) {
		write_char(fh,'\'');
		write_char(fh,(unsigned char) *base++);

		if (i) {
			write_Cstring(fh,  "\',"  );	// write ',  in one go
		} else {
			write_char(fh,'\'');			// write '   only
		}
	}
	return 0;
}
//-----------------------------------------------------------------------
write_string(int fh, char *str) {

	int i=0;
	char * base;

	base = str;

	while (*str!=' ' && *str!=TAB && *str!= 0 && *str!=LF) {
		i++;
		str++;
	}

	while (i--) {
		write_char(fh,(unsigned char) *base++);
	}

	return 0;
}
//-----------------------------------------------------------------------
write_Cstring(int fh, char *str) {

	int written,i=0;

	i = strlen(str);

	written = Write(fh, str, i);
	if (written != i) {
		printf("write_Cstring(%s) failed\n", str);
		exit(20);
	}

	return 0;
}
//-----------------------------------------------------------------------
void write_char(int fh, unsigned char c) {

	unsigned char buff;
	int written;

	buff = c;

	written = Write(fh, &buff, 1);
	if (!written) {
		printf("write_char(%c) failed\n", c);
		exit(20);
	}
}
//-----------------------------------------------------------------------
