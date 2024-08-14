#include <stdio.h>
#include <sys/file.h>

#define	LINELENGTHMAX	128	/* maximum characters per line */
#define	FILENAMEMAX	128	/* maximum characters in file name */
#define SOURCEMAX	128	/* maximum number of source files */
#define	NESTWARN	14	/* number of nests before warned */
#define	NESTERROR	16	/* number of nests before error */
#define INCLDIRSMAX	4	/* maximum number of include directories */
#define INCLFILESMAX	128	/* maximum number of distinct included files */

FILE *fopen();

char *sourceFiles[SOURCEMAX];
int numSFiles, currSFile;

char *includeDirectories[INCLDIRSMAX];
int numIDirectories;

char includeNest[NESTERROR][FILENAMEMAX];

char includedFiles[INCLFILESMAX][FILENAMEMAX];
int numIFiles;

char *progName;
int build, debug;
    
char *includeCTest(s)
char *s;
{
    char *result;
    int i;

    result = s;
    if (*s++ == '#') {
	if (strncmp(s, "include", 7) == 0) {
	    s += 7;
	    while (*s != '\0') {
		if ((*s == ' ') || (*s == '\t')) s++;
		else
		    if ((*s == '"') || (*s == '<')) {
			s++;
			result = s;
			while (*s != '\0')
			    if ((*s == '"') || (*s == '>')) {
				*s = '\0';
				if (debug)
				    fprintf(stderr, "includeTest: %s\n",
					result);
				return(result);
			    }
			    else s++;
		    }
		    else break;
	    }
	    fprintf(stderr, "%s: ill formed #include directive:\n%s\n",
		progName, result);
	}
    }
    return((char *) NULL);
}

char *includeAssemTest(s)
char *s;
{
    char *result;
    int i;

    result = s;
    while (*s != '\0') {
	if ((*s == ' ') || (*s == '\t')) s++;
	else {
	    if (((strncmp(s, "INCLUDE", 7) == 0) ||
		(strncmp(s, "include", 7) == 0)) &&
		((s[7] == ' ') || (s[7] == '\t'))) {
		s += 7;
		while (*s != '\0') {
		    if ((*s == ' ') || (*s == '\t')) s++;
		    else
			if ((*s == '"') || (*s == '\'')) {
			    s++;
			    result = s;
			    while (*s != '\0')
				if ((*s == '"') || (*s == '\'')) {
				    *s = '\0';
				    if (debug)
					fprintf(stderr, "includeTest: %s\n",
					    result);
				    return(result);
				}
				else s++;
			}
			else break;
		}
		fprintf(stderr, "%s: ill formed INCLUDE directive:\n%s\n",
		    progName, result);
	    }
	    else break;
	}
    }
    return((char *) NULL);
}

findInclude(fi, fileName, includeTest, level)
FILE *fi;
char *fileName;
char *(*includeTest)();
{
    FILE *newFile;
    char s[LINELENGTHMAX], *newFileSuffix, newFileName[FILENAMEMAX];
    int f, i, flag, prefixLen;

    if (build)
	printf("******************\n*");
    printf("%s:\t%s\n", sourceFiles[currSFile], fileName);
    if (debug || (level > NESTWARN))
	fprintf(stderr, "%s: include level %d, file %s\n",
	    progName, level, fileName);
    while (fgets(s, LINELENGTHMAX, fi) != NULL) {
	if ((newFileSuffix = includeTest(s)) != (char *) NULL) {
	    flag = 1;
	    for (i = 0; i < numIFiles; i++) {
		if (strcmp(newFileSuffix, includedFiles[i]) == 0) {
		    flag = 0;
		    break;		/* from numIFiles loop */
		}
	    }
	    if (flag) {
		strcpy(includedFiles[numIFiles++], newFileSuffix); 
		for (i = 0; i < numIDirectories; i++) {
		    if (strcmp(includeDirectories[i], ".") == 0) {
			strcpy(newFileName, includeNest[level]);
			strcat(newFileName, "/");
			strcat(newFileName, newFileSuffix);
		    }
		    else {
			strcpy(newFileName, includeDirectories[i]);
			strcat(newFileName, "/");
			strcat(newFileName, newFileSuffix);
		    }
		    while (strncmp(newFileName, "./", 2) == 0)
			strcpy(newFileName, &newFileName[2]);
		    if (debug)
			fprintf(stderr, "    newFileName \"%s\"\n",
			    newFileName);
		    if ((f = open(newFileName, O_RDONLY, 0)) >= 0) {
			newFile = fdopen(f, "r");
			prefixLen = rindex(newFileName, '/');
			if (prefixLen != 0) prefixLen -= (int) newFileName;
			if (debug)
			    fprintf(stderr, "    prefixLen %d\n", prefixLen);
			if (prefixLen == 0)
			    strcpy(includeNest[level+1], ".");
			else {
			    strncpy(includeNest[level+1], newFileName,
				prefixLen);
			    includeNest[level+1][prefixLen] = '\0';
			}
			if (debug)
			    fprintf(stderr, "    includeNext[%d] \"%s\"\n",
				level+1, includeNest[level+1]);
			findInclude(newFile, newFileName, includeTest, level+1);
			flag = 0;
			fclose(newFile);
			break;		/* from numDirectories loop */
		    }
		}
	    }
	    if (flag) {
		fprintf(stderr,
		    "%s: include file \"%s\" not found (from file %s).\n", 
		    progName, newFileSuffix, fileName);
		if (build)
		    exit(100);
		else
		    printf("%s:\t%s\n", fileName, newFileSuffix);
	    }
	}
	else 
	    if (build)
		printf("%s", s);
    }
}
	

    
main(argc, argv)        /* makeinclude.c: find nested include files */
int argc; 
char *argv[]; 
{ 
    char *arg;
    FILE *sourceFile; 
    char **gatherArray, *(*includeTest)();
    int a, f, i, *num, max;

    debug = 0;
    build = 0;
    numSFiles = 0;
    numIDirectories = 0;
    gatherArray = sourceFiles;
    num = &numSFiles;
    max = SOURCEMAX;
    progName = *argv++;
    includeTest = (char *(*)()) NULL;

    for (a = 1; a < argc; a++) {
	arg = *argv++;
	if (*arg == '-') {
	    while (*++arg != '\0')
		switch (*arg) {
		    case 'a':
			includeTest = includeAssemTest;
			break;
		    case 'b':
			build = 1;
			if (debug)
			    fprintf(stderr, "build on.\n");
			break;
		    case 'c':
			includeTest = includeCTest;
			break;
		    case 'd':
			debug = 1;
			fprintf(stderr, "debug on.\n");
			break;
		    case 'i':
			gatherArray = includeDirectories;
			num = &numIDirectories;
			max = INCLDIRSMAX;
			break;
		    default:;
		}
	}
	else
	    if (*num < max) {
		gatherArray[*num] = arg;
		++*num;
	    }
	    else {
		fprintf(stderr, "%s: too many files.\n",
		    progName);
		exit(100);
	    }
    }
    if (debug) {
	fprintf(stderr, "%s parameters:\n", progName);
	fprintf(stderr, "source files:\n");
	for (i = 0; i < numSFiles; i++)
	    fprintf(stderr, "\t%s\n", sourceFiles[i]);
	fprintf(stderr, "include directories:\n");
	for (i = 0; i < numIDirectories; i++)
	    fprintf(stderr, "\t%s\n", includeDirectories[i]);
	if (includeTest == includeCTest)
	    fprintf(stderr, "C include test.\n");
	if (includeTest == includeAssemTest)
	    fprintf(stderr, "assem include test.\n");
    }
    if (numSFiles == 0) {
	fprintf(stderr, "%s: no source files specified.\n", progName);
	exit(100);
    }
    if (includeTest == NULL) {
	fprintf(stderr, "%s: neither -a nor -c option specified.\n",
	    progName);
	exit(100);
    }
    strcpy(includeNest[0], ".");
    for (i = 0; i < numSFiles; i++)
	if ((f = open(sourceFiles[i], O_RDONLY, 0)) < 0) {
	    if (debug)
		fprintf(stderr, "%s: source file \"%s\" not readable.\n",
		    progName, sourceFiles[i]);
	}
	else {
	    sourceFile = fdopen(f, "r");
	    numIFiles = 0;
	    currSFile = i;
	    findInclude(sourceFile, sourceFiles[i], includeTest, 0);
	    fclose(sourceFile);
	}
    return(0);
}
