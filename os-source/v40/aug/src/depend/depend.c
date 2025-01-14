/*
**	Utility to create depend files for modules.
**
**	Written by Kevin-Neil Klop
**
**	$Id: depend.c,v 1.2 90/11/27 17:38:48 kevin Exp $
**
**	$Log:	depend.c,v $
 * Revision 1.2  90/11/27  17:38:48  kevin
 * made target extension '.o' instead of '.obj'
 * 
 * Revision 1.1  90/11/27  16:54:42  Kevin
 * Initial revision
 * 
**
*/

#include	<stdio.h>
#include	<ctype.h>

char	*CExtensions[] = {
	".c", ".h", ".cpp"};

char	*AExtensions[] = {
	".i", ".a", ".asm"};

#define	INFILE	(1)
#define	OUTFILE	(2)
#define	STATUS	unsigned int

typedef	STATUS (*ProcessingFunction)();

/*
**	If a file has an extension other than one of the above, it's of
**	unknown type, and will be rejected.
*/

void HelpThem(char *StartupName);
void EndItAll(STATUS, FILE *, FILE *);
void BurnLine(FILE *InFile);
void FindCommentEnd(FILE *InFile, char *CommentDelimiter);

STATUS ProcessFile(FILE *, FILE *, char *);
STATUS ProcessCFile(FILE *InFile, FILE *OutFile);
STATUS ProcessAFile(FILE *InFile, FILE *OutFile);
STATUS ProcessIncludeFile(FILE *InFile, FILE *OutFile, unsigned char InChar, ProcessingFunction Processor);

main(int argc, char *argv[])
{
	FILE	*InFile, *OutFile;

	if(argc != 3)
		HelpThem(argv[0]);
	else
	{
		InFile = fopen(argv[INFILE], "r");
		OutFile = fopen(argv[OUTFILE], "a");
		if(!InFile || !OutFile)
		{
			if(!InFile)
				fprintf(stderr,"Could not open %s for input.\n", argv[INFILE]);
			if(!OutFile)
				fprintf(stderr,"Could not open %s for output.\n", argv[OUTFILE]);
			EndItAll(20, InFile, OutFile);
		}
		else
			EndItAll(ProcessFile(InFile, OutFile, argv[INFILE]), InFile, OutFile);
	}
}

void
HelpThem(char *StartupName)
{
	printf("\n\nformat of command:\n\n     %s <infile> <outfile>\n\n", StartupName);
	exit(10);
}

void
EndItAll(STATUS statuscode, FILE *InFile, FILE *OutFile)
{
	if(InFile)
		fclose(InFile);
	
	if(OutFile)
		fclose(OutFile);
	
	exit(statuscode);
}

STATUS
ProcessFile(FILE *InFile, FILE *OutFile, char *InFileName)
{
	unsigned char	i,j;
	unsigned char	Done;
	STATUS			ReturnCode;
	
	Done = 0;
	for(j = strlen(InFileName) - 1; (InFileName[j] != '.') && (j > 0) ; j--);
	if(InFileName[j] != '.')
	{
		fprintf(stderr, "invalid input file name - can not process it.\n");
		return(20);
	}
	
	InFileName[j] = '\0';
	
	fputs("\n", OutFile);
	fputs(InFileName, OutFile);
	fputs(".o : ", OutFile);
	InFileName[j] = '.';
	fputs(InFileName, OutFile);
	
	for(i = 0; !Done && (i < sizeof(CExtensions)/sizeof(char *)); i++)
	{
		if(strcmp(CExtensions[i], &InFileName[j]) == 0)
		{
			ReturnCode = ProcessCFile(InFile, OutFile);
			Done = 1;
		}
	}
	
	for(i = 0; !Done && (i < sizeof(AExtensions)/sizeof(char *)); i++)
	{
		if(strcmp(AExtensions[i], &InFileName[j]) == 0)
		{
			ReturnCode = ProcessAFile(InFile, OutFile);
			Done = 1;
		}
	}
	
	fputc('\n', OutFile);
	return(ReturnCode);
}

static unsigned char	*IncludeKey	  = "include";

STATUS
ProcessCFile(FILE *InFile, FILE *OutFile)
{
	static unsigned char	*CommentBegin = "/*";
	static unsigned char	*CommentEnd	  = "*/";
	STATUS					 ReturnCode;
	unsigned char			 InChar;
	unsigned char			 StringIndex;
	
	ReturnCode = 
		StringIndex =
			0;
	while(!feof(InFile) && !ReturnCode)
	{
		InChar = fgetc(InFile);
		if(InChar == CommentBegin[StringIndex])
		{
			StringIndex++;
			while((InChar = fgetc(InFile)) == CommentBegin[StringIndex++]);
			if(CommentBegin[StringIndex] == '\0')
				FindCommentEnd(InFile, CommentEnd);
			else
				ungetc(InChar, InFile);
		}
		else if(InChar == '#')
		{
			while(isspace( InChar = fgetc(InFile) ));
			if((InChar != EOF) && (InChar != '\n'))
			{
				StringIndex = 0;
				ungetc(InChar, InFile);
				while((InChar = fgetc(InFile)) == IncludeKey[StringIndex])
					StringIndex++;
				ungetc(InChar, InFile);
				if(IncludeKey[StringIndex] == '\0')
				{
					while(isspace(InChar = fgetc(InFile)));
					if(InChar == '"')
						ReturnCode = ProcessIncludeFile(InFile, OutFile, '"', ProcessCFile);
				}
				else
				{
					ungetc(InChar, InFile);
					BurnLine(InFile);
				}
				
				StringIndex = 0;
			}
		}
		else
		{
			ungetc(InChar, InFile);
			BurnLine(InFile);
			StringIndex = 0;
		}
	}
	
	return(0);
}

STATUS
ProcessAFile(FILE *InFile, FILE *OutFile)
{
	STATUS					 ReturnCode;
	unsigned char			 StringIndex;
	unsigned char			 InChar;
	
	ReturnCode = StringIndex = 0;
		
	while(!feof(InFile) && !ReturnCode)
	{
		InChar = fgetc(InFile);
		if(isspace(InChar))
		{
			while(isspace(InChar))
				InChar = fgetc(InFile);
			StringIndex = 0;
			while(tolower(InChar) == IncludeKey[StringIndex])
			{
				StringIndex++;
				InChar = fgetc(InFile);
			}
			ungetc(InChar, InFile);
				
			if( (IncludeKey[StringIndex] == '\0') && (isspace(fgetc(InFile))) )
			{
				while(isspace(InChar = fgetc(InFile)) );
				
				if( (InChar == '\'') || (InChar == '"'))
					ReturnCode = ProcessIncludeFile(InFile, OutFile, InChar, ProcessAFile);
			}
			else
			{
				ungetc(InChar, InFile);
				BurnLine(InFile);
			}
		}
		else
		{
			ungetc(InChar, InFile);
			BurnLine(InFile);
		}
	}
	
	return(ReturnCode);
}

void
BurnLine(FILE *InFile)
{
	unsigned char	InChar;
	
	while(( (InChar = fgetc(InFile)) != '\n') && (InChar != EOF));
	return;
}

void
FindCommentEnd(FILE *InFile, char *CommentDelimiter)
{
	unsigned char	InChar;
	unsigned char	StringIndex;
	

	while(1)
	{
		StringIndex = 0;
		while(( InChar = fgetc(InFile)) != CommentDelimiter[StringIndex])
		{
			if(feof(InFile))
				return;
		}
		ungetc(InChar, InFile);
		while(( InChar = fgetc(InFile)) == CommentDelimiter[StringIndex])
		{
			if(feof(InFile))
				return;
			StringIndex++;
		}
		ungetc(InChar, InFile);
		
		if(CommentDelimiter[StringIndex] == '\0')
		{
			return;
		}
	}
}

STATUS
ProcessIncludeFile(FILE *InFile, FILE *OutFile, unsigned char TerminateChar, ProcessingFunction Processor)
{
	unsigned char	 Path[1024];
	unsigned int	 StringIndex;
	STATUS			 ReturnCode;
	FILE			*NextFile;
	
	ReturnCode = 0;
	StringIndex = 0;
	
	while( ((Path[StringIndex] = fgetc(InFile)) != TerminateChar) && (Path[StringIndex++] != EOF));
	Path[StringIndex] = '\0';
	
	NextFile = fopen(Path, "r");
	if(NextFile)
		ReturnCode = (*Processor)(NextFile, OutFile);
	
	fputs("\t", OutFile);
	fputs(Path, OutFile);
	
	return(ReturnCode);
}
