/*
**  $Header: V39:aug/src/fixheader/RCS/fixheader.c,v 1.10 92/09/04 11:34:35 peter Exp $
**
**  Main FixHeader entrypoint. 
**
**  (C) Copyright 1989 Commodore-Amiga, Inc.
**      All Rights Reserved
*/

/***************************************************************\
*								*
*	FixHeader.c (part of FixHeader program)			*
*	-------------------------------------------------------	*
*								*
*	Table of Contents:					*
*								*
*	Variables Defined:					*
*	------------------					*
*								*
*		Infile 		File Descriptor for input	*
*		TempFile 	File descriptor for scratchpad	*
*				use				*
*		StringBuffer	Pointer to temporary string	*
*				store				*
*		OutFilename	Pointer to output file name	*
*		InFilename	Pointer to input file name	*
*								*
*	Functions defined:					*
*	------------------					*
*								*
*		main(argv,argc)					*
*			Entrypoint to program			*
*								*
*		HelpThem()					*
*			Output a "help" type message		*
*								*
*		BuildTextString(argc, argv, StringPointer)	*
*			Builds a string into StringPointer from	*
*			arguments 2-N of the invoking command	*
*			line.					*
*								*
*		CreateTempFile()				*
*			Creates and opens a temporary file.	*
*			The file name is created through	*
*			contatenating "TMP" with the current	*
*			time (returned from the time()		*
*			call).					*
*								*
*		Cleanup()					*
*			Performs the last minute cleanup for	*
*			FixHeader just before exiting to the	*
*			operating system.			*
*								*
*	External Functions Used:				*
*	------------------------				*
*								*
*		ltoa(char *, long)				*
*		ProcessFile(FILE *, FILE *)			*
*								*
\***************************************************************/

#include	<time.h>
#include "fixheader.h"
#include "fixheader_rev.h"

static char version[] = VERSTAG;

/***************************************************************\
*  some of the constants we use throughout the program		*
\***************************************************************/

#define	MAXFILENAMESIZE	(128)

/***************************************************************\
*  The input and temporary file descriptor pointers.  We don't	*
*  need an output file descriptor because the temporary file	*
*  is renamed to the input file upon successful termination of	*
*  the program (see function EndProcess() in ProcessFile.c)	*
\***************************************************************/

FILE	*InFile		= NULL,
	*TempFile	= NULL;

/***************************************************************\
*  Pointers to global, dynamically allocated, string buffers	*
\***************************************************************/

char	*StringBuffer = NULL;
char	*OutFilename = NULL;
char	*InFilename = NULL;
char	TempFilename[MAXFILENAMESIZE] = {"\0\0\0\0\0\0\0\0\0\0\0\0\0\0"};
long    copyrightyear = 0;

extern unsigned int	ErrorFlag;
extern unsigned int	ENTAB;

/***************************************************************\
*  Function prototypes...					*
\***************************************************************/

char			 RealOutputFile = 0;
int stcd_l(char *, long *);

int main ( int argc, char *argv[] );
static void HelpThem ( void );
static FILE *CreateTempFile ( void );
static void BuildTextString ( int argc, char *argv[], char **StringBuffer );

/***************************************************************\
*****************************************************************
*********** The program begins execution here... ****************
*****************************************************************
\***************************************************************/

/***************************************************************\
*	The command line is of the following form:		*
*								*
*	FixHeader <infile> [-o <outfile> <string>]		*
*								*
*	Note that <string> may appear without the -o <outfile>	*
*	but that the -o <outfile> requires the <string>.	*
*								*
\***************************************************************/

#ifdef LATTICE
int CXBRK(void) { return(0); }
void chkabort(void) { return; }
#endif

int	main(argc,argv)
int	 	 argc;
char	*argv[];
{
    int	ArgCntr;

    if (argc == 1)
	HelpThem();

    for(ArgCntr = 1; ArgCntr < argc; ArgCntr++)
    {
	if(strcmp(argv[ArgCntr], "-y") == 0)
	{
	    ArgCntr++;
	    stcd_l( argv[ArgCntr], &copyrightyear );
	    copyrightyear %= 100;
	}

	else if(strcmp(argv[ArgCntr], "-o") == 0)
	{
	    ArgCntr++;
	    OutFilename = argv[ArgCntr];
	}
	else if(strcmp(argv[ArgCntr],"-t") == 0)
		ENTAB = 1;
	else if(!InFilename)
	    InFilename = argv[ArgCntr];
	else if(!StringBuffer)
	    StringBuffer = argv[ArgCntr];
	else
	{
	    printf("Error in argument %d: \"%s\"\nStringBuffer \"%s\"\n",
		ArgCntr, argv[ArgCntr], StringBuffer);
	    HelpThem();
	};
    };

    if(OutFilename && !StringBuffer)
	HelpThem();

    if ((InFile = fopen(InFilename,"r")) == NULL)
    {
	ErrorFlag++;
	Cleanup();
	printf ("-----> File \"%s\" not found\n",InFilename);
	exit(20);
    };

    if(OutFilename)
    {
	if(strcmp(InFilename, OutFilename) == 0)
	{
	    if((TempFile = CreateTempFile()) == NULL)
	    {
		ErrorFlag++;
		printf("----->Could not open up a temporary work file");
		Cleanup();
		exit(20);
	    };
	}
	else if((TempFile = fopen(OutFilename, "w")) == NULL)
	{
	    ErrorFlag++;
	    Cleanup();
	    printf ("-----> Could not open output file \"%s\"\n",&argv[2][1]);
	    exit(20);
	};
    }

    ProcessFile(InFile,TempFile);

    if(OutFilename && (strcmp(InFilename, OutFilename) == 0))
    {
	rename(TempFilename, OutFilename);
    };

    Cleanup();
    return 0;
}

/***************************************************************\
*****************************************************************
**************** Give the user a little help ********************
*****************************************************************
\***************************************************************/
static void HelpThem( void )
{
    printf ("Format of FixHeader command is:\n\n");
    printf ("         FixHeader <infilename> [-t] [-o <outfilename>] [-y <year>] [<string>]\n\n");

    printf("(Note that <string> can appear without -o <outfilename>, but that\n");
    printf("if you specify -o <outfile>, then you MUST specify <string> as well.\n");
    printf("\nThe -t option causes FixHeader to try and convert runs of spaces to\n");
    printf("tabs.\n");
    printf("The -y option is the copyright year (eg. '92' or '1992') that the files\n");
    printf("are to have.\n");
    exit(20);
}

/***************************************************************\
*****************************************************************
***************** Create a temporary file for use ***************
*****************************************************************
\***************************************************************/

static FILE *CreateTempFile( void )
{
    long	timeval;
    char	StringBuffer[32];
    unsigned char BufferPtr;

    for(BufferPtr = 0; BufferPtr < 32; BufferPtr++)
	StringBuffer[BufferPtr] = '\0';

    timeval = time(NULL);
    strcpy (TempFilename, "TEMP");
    ltoa(StringBuffer, timeval);
    strcat(TempFilename, StringBuffer);
   
    return (fopen(TempFilename,"w"));
}

/***************************************************************\
*****************************************************************
******* End of program processing that must always be done ******
*****************************************************************
\***************************************************************/

void Cleanup( void )
{
    if (ErrorFlag)
	printf (", Input file = \"%s\"\n",InFilename);

    if (InFile)
	fclose(InFile);

    if (TempFile)
	fclose(TempFile);

    return;
}

/***************************************************************\
*****************************************************************
*** Dynamically build the header string from the command line ***
*****************************************************************
\***************************************************************/

static void	BuildTextString(argc, argv, StringBuffer)
int		 argc;
char	*argv[];
char	**StringBuffer;
{
    int	ArgPtr, BufferSize = 0;
    int	StartPoint = 2;

    if (RealOutputFile) StartPoint = 3;

    for (ArgPtr = StartPoint; ArgPtr < argc; ArgPtr++)
	BufferSize += strlen (argv[ArgPtr]) + 1;

    if ((*StringBuffer = (char *)calloc(1,BufferSize+2)) == NULL)
    {
	Cleanup();
	printf ("-----> Could not create temporary memory buffer\n");
	exit(20);
    }

    for (ArgPtr = StartPoint; ArgPtr < argc; ArgPtr++)
    {
	strcat(*StringBuffer,argv[ArgPtr]);
	strcat(*StringBuffer," ");
    };
}
