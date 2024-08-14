/*
**  $Id: processfile.c,v 1.18 93/08/12 15:01:37 peter Exp $
**
**  Conversion routines used in FixHeader
**
**  (C) Copyright 1989 Commodore-Amiga, Inc.
**      All Rights Reserved
*/

#define D(x)	;

/***************************************************************\
*                                                               *
*   ProcessFile.c                                               *
*   ----------------------------------------------------------  *
*                                                               *
*   Table of Contents:                                          *
*                                                               *
*       Variables Used:                                         *
*       ---------------                                         *
*           InFilename      Input Filename                      *
*                                                               *
*       Functions Defined:                                      *
*       ------------------                                      *
*                                                               *
*           ProcessFile(FILE *Input, FILE *Output)              *
*               Processes the input file to the output file as  *
*               per E-Mail message from kodiak (Bob Burns) to   *
*               andy (Andy Finkel) and kevin (Kevin Klop) dated *
*               01-May-89 19:50:51, "fixheader spec".           *
*                                                               *
*****************************************************************
*                                                               *
*                       Revision History                        *
*                                                               *
*   05-May-89   Original Coding                     Kevin Klop  *
*                                                               *
\***************************************************************/

#include    "FixHeader.h"


#define IsComment(x)    (!(strncmp(x,"**",2) && strncmp(x,"/*",2)))
#define DeleteFile(x)   unlink(x)
#define RenameFile(x,y) rename(x,y)

#define	MINREVNUMBER	36

/***************************************************************\
*   Prototype the functions that we use.                        *
\***************************************************************/

static IsKeyword ( char *aLine, char *Keyword );
static void ProcessHeader ( char *LineBuffer, FILE *Infile, FILE *Outfile );
static int IsCopyright ( char *aLine );
static int IsCommodore( char *aLine );
static void ProcessCopyright( char *aLine, FILE *infile, FILE *outfile );

/***************************************************************\
*   The external variables that we use                          *
\***************************************************************/

extern char     *InFilename;
extern char     *StringBuffer;
extern long	copyrightyear;

unsigned int    ErrorFlag = 0;
static   char   FoundHeader = FALSE;
unsigned int	TheRevision = 0;

/***************************************************************\
*****************************************************************
*********************** ProcessFile *****************************
*****************************************************************
\***************************************************************/

void    ProcessFile(Infile, Outfile)
FILE    *Infile, *Outfile;
{
    char    *InputLine;
    char    *WhichPtr;

    /***********************************************\
    * Allocate a line buffer from dynamic memory    *
    \***********************************************/

    FoundHeader = FALSE;

    if ((InputLine = (char *)malloc(MAXINPUTLINELEN)) == NULL)
    {
        printf ("-----> Could not allocate a line input buffer\n");
        Cleanup();
        exit(20);
    };

    /***********************************************\
    * Process the input file line by line.  This    *
    * Exits to the caller of ProcessFile internally *
    \***********************************************/

    while(1)
    {
        if (fgets(InputLine, MAXINPUTLINELEN-1, Infile))
        {
	    if(InputLine[strlen(InputLine)-1] == '\n')
		InputLine[strlen(InputLine)-1] = '\0';

        /***************************************\
        * Process the header line if we found   *
        * one.                                  *
        \***************************************/

            if (IsComment(InputLine))
            {
                if(IsKeyword(InputLine, "Header") ||
                   IsKeyword(InputLine, "Id"))
                    ProcessHeader(InputLine, Infile, Outfile);
                else if (IsKeyword(InputLine, (WhichPtr = "Filename")) ||
                         IsKeyword(InputLine, (WhichPtr = "Release")) ||
                         IsKeyword(InputLine, (WhichPtr = "Revision")) ||
                         IsKeyword(InputLine, (WhichPtr = "Date")))
                    printf("-----> ProcessFile: %s exists\n",WhichPtr);
                else if ( (IsCopyright(InputLine)) && (IsCommodore(InputLine)) )
                    ProcessCopyright(InputLine, Infile, Outfile);
                else if(Outfile)
		{
		    entab(InputLine);
                    fputs(InputLine, Outfile);
		    fputc('\n',Outfile);
		}
            }
            else if(Outfile)
	    {
		entab(InputLine);
                fputs(InputLine, Outfile);
		fputc('\n', Outfile);
	    }
        }

        /***************************************\
        * Stop if we've hit the end of input    *
        \***************************************/


        if (feof(Infile))
        {
            if (!FoundHeader)
            {
                printf("----->\n");
                printf("-----> ProcessFile: No $Id line found\n----->\n");
            };
#if 0
            if(TheRevision < MINREVNUMBER)
            	printf("-----> WARNING: File has too low a major revision number of %d\n", TheRevision);
#endif
            	
            return;
        }
    }
}

/**************************************************************\
****************************************************************
************************* IsKeyword ****************************
****************************************************************
\**************************************************************/

static IsKeyword(aLine,Keyword)
char    *aLine;
char    *Keyword;
{
    if (strncmp(aLine,"**",2))
        return(0);
    aLine += 2L;
    while((*aLine == ' ') || (*aLine == '\t')) aLine++;
    aLine++;
    return(strncmp(aLine,Keyword,strlen(Keyword)) ? 0 : 1);
}

/*************************************************************\
***************************************************************
************************** ProcessHeader **********************
***************************************************************
\*************************************************************/

static void ProcessHeader(LineBuffer, Infile, Outfile)
char    *LineBuffer;
FILE    *Infile, *Outfile;
{
    char    WhiteSpaceLeader[256];
    int     Index = 0;
    char    Outbuffer[MAXINPUTLINELEN];
    char    *OutLoc;
    int     datecount;
    char    datebuf[ 8 ];

    /***************************************************\
    * Get the leading whitespace into a buffer so that  *
    * everything lines up correctly.                    *
    \***************************************************/

    while ((*LineBuffer == ' ')    ||
       (*LineBuffer == '\t')   ||
       (*LineBuffer == '*'))
    WhiteSpaceLeader[Index++] = *(LineBuffer++);
    WhiteSpaceLeader[Index] = '\0';
    
    if (*LineBuffer == '\n')
    {
        printf ("-----> Empty $Header line, ");
        ErrorFlag++;
        return;
    }

    /***************************************************\
    * Parse the $Header line                            *
    \***************************************************/
    
    for ( ; (*LineBuffer != ',') && (*LineBuffer != '\n'); LineBuffer++);

    if (*LineBuffer == '\n')
    {
        printf ("-----> Invalid $Header line in file (could not find \",v\")");
        ErrorFlag++;
        return;
    }
    
    strcpy(Outbuffer, WhiteSpaceLeader);
    strcat(Outbuffer, "$V" );		/* Broke up $VER so c:version won't */
    strcat(Outbuffer, "ER: ");		/* accidentally find this one */
    strcat(Outbuffer, InFilename);
    strcat(Outbuffer, " ");

    OutLoc = &(Outbuffer[strlen(Outbuffer)]);

    LineBuffer++;
    
    if (*LineBuffer == 'v')
    {
        LineBuffer++;
        while (*(LineBuffer++) == ' ');
        LineBuffer--;
        sscanf(LineBuffer, "%d.", &TheRevision);
        while (*LineBuffer != ' ')
	{
	    *OutLoc++ = *LineBuffer++;
	}
        *OutLoc = '\0';
        strcat(Outbuffer, " (");

        OutLoc = &(Outbuffer[strlen(Outbuffer)]);
        
	/* Skip spaces up to the date */
        while (*(LineBuffer++) == ' ');
        LineBuffer--;

	/* RCS date is year/month/day.  We want to emit day.month.year */
	datecount = 0;
        while ( (*LineBuffer != ' ') && ( datecount < 8 ) )
	{
	    datebuf[ datecount++ ] = *LineBuffer++;
	}
	if ( datecount != 8 )
	{
	    printf("-----> Bad RCS $Id date!\n");
	    ErrorFlag++;
	    return;
	}
	else
	{
	    if ( datebuf[ 6 ] != '0' )
	    {
		*OutLoc++ = datebuf[ 6 ];
	    }
	    *OutLoc++ = datebuf[ 7 ];
	    *OutLoc++ = '.';
	    if ( datebuf[ 3 ] != '0' )
	    {
		*OutLoc++ = datebuf[ 3 ];
	    }
	    *OutLoc++ = datebuf[ 4 ];
	    *OutLoc++ = '.';
	    if ( datebuf[ 0 ] != '0' )
	    {
		*OutLoc++ = datebuf[ 0 ];
	    }
	    *OutLoc++ = datebuf[ 1 ];
	}
        *OutLoc = '\0';
        strcat(Outbuffer, ")");
    }
    entab(Outbuffer);
    if(Outfile)
    {
	fputs(Outbuffer, Outfile);
	fputc('\n', Outfile);
    }

    strcpy(Outbuffer, WhiteSpaceLeader);
    strcat(Outbuffer, "Includes Release ");
    if(StringBuffer)
	strcat(Outbuffer, StringBuffer);
    entab(Outbuffer);
    if(Outfile)
    {
	fputs(Outbuffer, Outfile);
	fputc('\n',Outfile);
    }
    
    FoundHeader = TRUE;
    return;
}

/**************************************************************\
****************************************************************
*********************** IsCopyright ****************************
****************************************************************
\**************************************************************/

static int IsCopyright(aLine)
char    *aLine;
{
    while (*aLine != '\0')
        if (*aLine != 'C')
            aLine++;
        else if (strncmp(aLine, "Copyright", 9))
            aLine++;
        else
            return(TRUE);
    return(FALSE);
}


/**************************************************************\
****************************************************************
*********************** IsCommodore ****************************
****************************************************************
\**************************************************************/

static int IsCommodore(aLine)
char    *aLine;
{
    while (*aLine != '\0')
        if (*aLine != 'C')
            aLine++;
        else if (strncmp(aLine, "Commodore", 9))
            aLine++;
        else
            return(TRUE);
    return(FALSE);
}


/**************************************************************\
****************************************************************
********************* ProcessCopyright *************************
****************************************************************
\**************************************************************/

/* Here's what we want ProcessCopyright to do:
 *
 * Emit lines of the form
 *
 * (C) Copyright 1985-1992, Commodore-Amiga, Inc.
 *
 * The things it needs to worry about are:
 * Include files typically don't have up-to-date copyrights.  What
 * we want it to do is find the first year after the copyright,
 * delete all subsequent years, and add "-1992" to the copyright.
 *
 * However, we have files that are not just copyright by us:
 *
 * 	rexx/*
 *	libraries/asl.*
 *	libraries/translator.*
 *	devices/narrator.*
 *
 *	( libraries/iffparse.* seems to belong to us according to contract )
 *
 * Also, some include files say "Commodore-Amiga, Inc.", but some leave out
 * the comma and some miss the dash.
 *
 * So what this routine should do is only extend the copyright on Commodore
 * lines.  ProcessFile() takes care of that.
 */

static void ProcessCopyright(aLine, infile, outfile)
char    *aLine;
FILE    *infile, *outfile;
{
    char    Outbuffer[MAXINPUTLINELEN];
    char    *out = Outbuffer;
    static  char skip[] = "0123456789- \t,";
    int     done;
    char    decade, year;
    char    cpdecade, cpyear;

    cpdecade = ( copyrightyear / 10 ) + '0';
    cpyear = ( copyrightyear % 10 ) + '0';

    done = !(*aLine != '\0');
    while ( !done )
    {
	/* Copy the copyright line, up until the first date */
	*out++ = *aLine;
	D( printf("%c", *aLine) );

	/* Looks like a date? */
        if (!strncmp(aLine, "19", 2))
        {
	    aLine++;
	    D( printf("%c", *aLine) );
	    *out++ = *aLine++;		/* Copy the "9" */
	    D( printf("%c", *aLine) );
	    decade = *out++ = *aLine++;		/* Copy the "x" */
	    D( printf("%c", *aLine) );
	    year = *out++ = *aLine++;		/* Copy the "x" */
	    while ( strchr( skip, *aLine ) )
	    {
		*aLine++;
	    }
	    if ( ( cpdecade != decade ) || ( cpyear != year ) )
	    {
		*out++ = '-';
		*out++ = '1';
		*out++ = '9';
		*out++ = cpdecade;
		*out++ = cpyear;
	    }
	    *out++ = ' ';
	    D( printf("-19%c%c %s\n", cpdecade, cpyear, aLine) );
	    strcpy( out, aLine );
	    done = 1;
	}
	else
	{
	    aLine++;
	    done = !(*aLine != '\0');
	}
    }

    entab(Outbuffer);
    if(outfile)
    {
	fputs(Outbuffer, outfile);
	fputc('\n', outfile);
    }
}
