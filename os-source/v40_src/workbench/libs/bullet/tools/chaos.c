/*
 *	CHAOS -- CHange Amiga ObjectS
 */
#ifdef DEBUG
#define	D(a)	printf a
#else
#define D(a)
#endif

#include	<stdio.h>
#include	<setjmp.h>

/*	Amiga object definitions */
#define HUNK_FIRST    999
#define hunk_unit     999
#define hunk_name    1000
#define hunk_code    1001
#define hunk_data    1002
#define hunk_bss     1003
#define hunk_reloc32 1004
#define hunk_reloc16 1005
#define hunk_reloc8  1006
#define hunk_ext     1007
#define hunk_symbol  1008
#define hunk_debug   1009
#define hunk_end     1010
#define hunk_header  1011
#define hunk_cont    1012
#define hunk_overlay 1013
#define hunk_break   1014
#define	hunk_dreloc32 1015
#define	hunk_dreloc16 1016
#define	hunk_dreloc8 1017
#define	hunk_lib_hunk 1018
#define	hunk_lib_index 1019
#define HUNK_LAST    1019

#define ext_symb	0
#define ext_defr	1
#define ext_defa	2
#define ext_ref32     129
#define ext_common    130
#define ext_ref16     131
#define ext_ref8      132
#define	ext_dref32    133
#define	ext_dref16    134
#define	ext_dref8     135

#define	TYPEMASK	0x3fffffff
#define	MEMMASK		0xc0000000


/*
 * 	in-memory objects structures:
 *
 *	Object Unit List
 *	  Unit Name
 *	  Unit #
 *	  Hunk List
 *	    Hunk #
 *	    Name
 *	    Type
 *	    Contents
 *	      Size
 *	      Data
 *	    Relocations List
 *	      Hunk #
 *	      Offset
 *	    Definitions List
 *	      Type
 *	      Name
 *	      Value
 */

#define MAXSYMSIZE	64	/* bytes */

struct Reloc {
    struct Reloc *next;
    long offset;
    short hunkNum;
    short hunkNum2;
    short type;
};

struct ExtDef {
    struct ExtDef *next;
    long value;
    char name[MAXSYMSIZE];
    short type;
    long references[1];		/* actually [value] */
};
    
struct Hunk {
    struct Hunk *next;
    struct Hunk *next2;
    struct Unit *unit;
    char name[MAXSYMSIZE];
    long size;
    long *data;
    struct Reloc *relocHead;
    struct ExtDef *extDefHead;
    long origin;
    short hunkNum;
    short hunkNum2;
    short type;
};

struct Unit {
    struct Unit *next;
    char name[MAXSYMSIZE];
    struct Hunk *hunkHead;
};

/*	global variables */
char *ProgName;			/* program name */
char *OptArg;			/* current argument, used in error reporting */
FILE *InFile = 0;		/* current object input file */
FILE *OutFile;			/* final object output file */
jmp_buf Env;			/* for error recovery */

long StartOffset = 0;

struct Unit *UnitHead = 0;
char HunkName[MAXSYMSIZE];
char TempName[MAXSYMSIZE];

struct Hunk HunkHead2 = { 0, 0 };	/* only next2 pointer is valid */

#define MAXNEWHUNKS	40
#define MAXHUNKS	200
short NewHunks[MAXNEWHUNKS];
long HunkOffsets[MAXHUNKS];

/*	error handling */
#define	error_usage	 1
#define error_option	 2
#define	error_fatal	 3
#define	error_nomemory	 4
#define	error_iopen	 5
#define	error_ierr	 6
#define	error_iseek	 7
#define	error_ieof	 8
#define	error_oopen	 9
#define	error_oerr	10
#define	error_oseek	11
#define error_namesize	12
#define	error_badhunk	13
#define	error_badext	14
#define error_memspec	15
#define error_nounitname 16
#define error_maxhunks	17
#define error_coderef	18
#define error_smallref	19
#define error_extref	20

char *ErrorMsgs[] = {
    (char *) 0, "",
    "invalid option %s"
    "this is a fatal error",
    "Out of Memory",
    "Cannot Open <executable> Input File %s",
    "Input File Error",
    "Input Seek Error",
    "Premature End of Input File",
    "Cannot Open Output File %s",
    "Output File Error",
    "Output Seek Error",
    "name too long",
    "HUNK %s not supported",
    "EXT %ld not supported",
    "Memory specification not supported",
    "Missing unit name",
    "Too many hunks (at %s)",
    "DATA/BSS relocation to code hunk in %s",
    "DATA/BSS non 32-bit relocation to code hunk in %s",
    "DATA/BSS external reference in %s"
};

void
abEndGame(errorCode)
int errorCode;
{
    if (errorCode > error_usage) {
	fprintf(stderr, "%s: ERROR -- ", ProgName);
	fprintf(stderr, ErrorMsgs[errorCode], OptArg);
	fprintf(stderr, "\n");
    }
    fprintf(stderr,
	    "Usage: %s <start-offset> <out-assem> <in-object-list-file>\n",
	    ProgName);
    fprintf(stderr,
	    "   or: %s o <out-object> <in-object>\n", ProgName);
    exit(1);
}

/*	support routines */

void *
mymalloc(size)
int size;
{
    void *result;

    result = (void *) malloc(size);
    if (result == 0)
	longjmp(Env, error_nomemory);
    return (result);
}


char *HunkNames[HUNK_LAST-HUNK_FIRST+1] = {
    "unit", "name", "code", "data", "bss", "reloc32", "reloc16", "reloc8", 
    "ext", "symbol", "debug", "end", "header", "cont", "overlay", "break", 
    "dreloc32", "dreloc16", "dreloc8", "lib_hunk", "lib_index"
};

void
errhunk(hunk)
int hunk;
{
    if ((HUNK_FIRST <= hunk) && (hunk <= HUNK_LAST))
	OptArg = HunkNames[hunk-HUNK_FIRST];
    else {
	sprintf(TempName, "unknown (%d)", hunk);
	OptArg = TempName;
    }
    longjmp(Env, error_badhunk);
}

long
getlong()
{
    long result;

    fread( (char *) &result, 1, 4, InFile);
    if (feof(InFile))
	longjmp(Env, error_ieof);
    if (ferror(InFile))
	longjmp(Env, error_ierr);

    return (result);
}

void
readlongs(target, size)
long *target;
long size;
{
    fread( (char *) target, size, 4, InFile);
    if (feof(InFile))
	longjmp(Env, error_ieof);
    if (ferror(InFile))
	longjmp(Env, error_ierr);
}

void
readname(target, size)
long size;
char *target;
{
    if ((size*4) >= MAXSYMSIZE)
	longjmp(Env, error_namesize);
    readlongs(target, size);
    target[size*4] = '\0';
    D(("readname \"%s\" %ld %ld\n", target, size*4, size));
}


void
tosslong(size)
long size;
{
    if (fseek(InFile, size * 4, 1) == -1)
	longjmp(Env, error_iseek);
}


void
putlong(value)
long value;
{
    fwrite((char *) &value, 1, 4, OutFile);
    if (feof(OutFile))
	longjmp(Env, error_oerr);
    if (ferror(OutFile))
	longjmp(Env, error_oerr);
}

void
putlongs(buffer, size)
long *buffer;
long size;
{
    fwrite((char *) buffer, size, 4, OutFile);
    if (feof(OutFile))
	longjmp(Env, error_oerr);
    if (ferror(OutFile))
	longjmp(Env, error_oerr);
}

void
putname(name, type)
char *name;
char type;
{
    short len, longs;

    len = strlen(name);
    longs = (len+3)/4;
    for (; len & 3; len++)
	name[len] = '\0';
    D(("putname \"%s\" %ld %ld\n", name, len, longs));
    putlong((type<<24)|longs);
    putlongs(name, longs);
}


/*	output writer */

long hunkRTypes[] = {
    hunk_reloc32, hunk_reloc16, hunk_reloc8,
    hunk_dreloc32, hunk_dreloc16, hunk_dreloc8
};
long hunkETypes[] = {
    ext_ref32, ext_ref16, ext_ref8,
    ext_dref32, ext_dref16, ext_dref8
};

void
genObject()
{
    struct Hunk *hunk;
    struct Reloc *reloc;
    struct ExtDef *extDef;

    int codeCount, hunkNum, i, hunkType, hunkCount, hunkMax;
    char relocFlag, drelocFlag;

    if ((!UnitHead) || (!UnitHead->name[0]))
	longjmp(Env, error_nounitname);
	
    putlong(hunk_unit);
    OptArg = UnitHead->name;
    putname(OptArg, 0);
    hunkMax = -1;
    codeCount = 0;
    for (hunk = UnitHead->hunkHead; hunk; hunk = hunk->next) {
	hunkMax++;
	if (hunkMax >= MAXNEWHUNKS)
	    longjmp(Env, error_maxhunks);
	if (strcmp(hunk->name, "__MERGED"))
	    /* not a __MERGED data hunk */
	    NewHunks[hunk->hunkNum] = hunk->hunkNum2 = codeCount++;
	else
	    /* a __MERGED data hunk */
	    NewHunks[hunk->hunkNum] = hunk->hunkNum2 = -1;
    }
    D(("%ld non-__MERGED\n", codeCount));
    for (hunk = UnitHead->hunkHead; hunk; hunk = hunk->next) {
	if (hunk->hunkNum2 >= 0) {
	    if (hunk->name[0]) {
		putlong(hunk_name);
		putname(hunk->name, 0);
	    }
	    putlong(hunk->type);
	    putlong(hunk->size);
	    if (hunk->data)
		putlongs(hunk->data, hunk->size);
	    relocFlag = drelocFlag = 0;
	    for (reloc = hunk->relocHead; reloc; reloc = reloc->next) {
		reloc->hunkNum2 = NewHunks[reloc->hunkNum];
		if (reloc->hunkNum2 >= 0) {
		    relocFlag = 1;
		}
		else
		    drelocFlag = 1;		
	    }
	    if (relocFlag) {
		for (i = 0; i < 6; i++) {
		    hunkType = hunkRTypes[i];
		    relocFlag = 0;
		    for (reloc = hunk->relocHead; reloc;
			    reloc = reloc->next) {
			if ((reloc->hunkNum2 >= 0) &&
				(reloc->type == hunkType)) {
			    relocFlag = 1;
			    break;
			}
		    }
		    if (relocFlag) {
			putlong(hunkType);
			for (hunkNum = 0; hunkNum <= hunkMax; hunkNum++) {
			    hunkCount = 0;
			    for (reloc = hunk->relocHead; reloc;
				    reloc = reloc->next)
				if ((reloc->hunkNum == hunkNum) &&
					(reloc->type == hunkType))
				    hunkCount++;
			    if (hunkCount) {
				putlong(hunkCount);
				putlong(NewHunks[hunkNum]);
				for (reloc = hunk->relocHead; reloc;
					reloc = reloc->next)
				    if ((reloc->hunkNum == hunkNum) &&
					    (reloc->type == hunkType))
					putlong(reloc->offset);
			    }
			}
			putlong(0);
		    }
		}
	    }
	    if ((drelocFlag) || (hunk->extDefHead)) {
		putlong(hunk_ext);
		hunkMax = -1;
		for (reloc = hunk->relocHead; reloc; reloc = reloc->next)
		    if (hunkMax < reloc->hunkNum)
			hunkMax = reloc->hunkNum;
		for (hunkNum = 0; hunkNum <= hunkMax; hunkNum++) {
		    if (NewHunks[hunkNum] < 0) {
			sprintf(TempName, "_%s_%d", UnitHead->name, hunkNum);
			for (i = 0; i < 6; i++) {
			    hunkType = hunkRTypes[i];
			    hunkCount = 0;
			    for (reloc = hunk->relocHead; reloc;
				    reloc = reloc->next)
				if ((reloc->hunkNum == hunkNum) &&
					(reloc->type == hunkType))
				    hunkCount++;
			    if (hunkCount) {
				putname(TempName, hunkETypes[i]);
				putlong(hunkCount);
				for (reloc = hunk->relocHead; reloc;
					reloc = reloc->next)
				    if ((reloc->hunkNum == hunkNum) &&
					    (reloc->type == hunkType))
					putlong(reloc->offset);
			    }
			}
		    }
		}
		for (extDef = hunk->extDefHead; extDef;
			extDef = extDef->next) {
		    putname(extDef->name, extDef->type);
		    putlong(extDef->value);
		    if ((extDef->type != ext_defa) &&
			    (extDef->type != ext_defr))
			putlongs(extDef->references, extDef->value);
		}
		putlong(0);
	    }
	    putlong(hunk_end);
	}
    }
}


struct Hunk *
genAsm2(hunkType, prevHunk2, hunkCount, length)
struct Hunk *prevHunk2;
short hunkType;
short *hunkCount;
long *length;
{
    struct Unit *unit;
    struct Hunk *hunk;

    D(("genAsm2\n"));
    for (unit = UnitHead; unit; unit = unit->next) {
	OptArg = unit->name;
	D(("unit %s\n", OptArg));
	for (hunk = unit->hunkHead; hunk; hunk = hunk->next) {
	    D(("  $%lx\n", hunk));
	    if ((hunk->type == hunkType) && (!strcmp(hunk->name, "__MERGED"))) {
		D(("@ $%lx -> $%lx\n", prevHunk2, hunk));
		prevHunk2->next2 = hunk;
		prevHunk2 = hunk;
		hunk->hunkNum2 = (*hunkCount)++;
		hunk->origin = *length;
		*length += hunk->size*4;
		D(("next2 %s hunk #%ld (was #%ld)\n", unit->name,
			hunk->hunkNum2, hunk->hunkNum));
		if (*hunkCount >= MAXHUNKS)
		    longjmp(Env, error_maxhunks);
		HunkOffsets[hunk->hunkNum2] = hunk->origin;
	    }
	}
    }
    return(prevHunk2);
}


void
genAsm()
{
    struct Unit *unit;
    struct Hunk *hunk;
    struct Reloc *reloc;
    struct ExtDef *extDef;
    long length, dataLength, i;
    short hunkNumber, hunkMax;

    D(("genAsm\n"));
    for (unit = UnitHead; unit; unit = unit->next) {
	if (!unit->name[0])
	    longjmp(Env, error_nounitname);
    }

    /* renumber all DATA & BSS hunks */
    D(("renum DATA/BSS\n"));
    hunkNumber = 0;
    length = StartOffset;
    hunk = genAsm2(hunk_data, &HunkHead2, &hunkNumber, &length);
    dataLength = length;
    genAsm2(hunk_bss, hunk, &hunkNumber, &length);

    D(("renum reloc\n"));
    /* renumber reloc references */
    for (unit = UnitHead; unit; unit = unit->next) {
	OptArg = unit->name;
	D(("  unit %s\n", OptArg));
	hunkMax = -1;
	for (hunk = unit->hunkHead; hunk; hunk = hunk->next) {
	    hunkMax++;
	    D(("  hunk %ld\n", hunkMax));
	    if (hunkMax >= MAXNEWHUNKS)
		longjmp(Env, error_maxhunks);
	    NewHunks[hunk->hunkNum] = hunk->hunkNum2;
	}
	for (hunk = unit->hunkHead; hunk; hunk = hunk->next) {
	    D(("  hunk %ld\n", hunkMax));
	    if (hunk->hunkNum2 >= 0) {
		D(("  -> %ld\n", hunk->hunkNum2));
		for (reloc = hunk->relocHead; reloc; reloc = reloc->next) {
		    reloc->hunkNum2 = NewHunks[reloc->hunkNum];
		}
	    }
	}
    }

    /* output data initialization */
    D(("data output...\n"));
    fprintf(OutFile, "\tXDEF\tCHAOSDataStart\n");
    fprintf(OutFile, "CHAOSDataStart:\n");
    for (i = 0; i < StartOffset/4; i++) {
	if ((i & 0xf) == 0)
	    fprintf(OutFile, "\n\t\tdc.l\t");
	else
	    fprintf(OutFile, ",");
	fprintf(OutFile, "0");
    }
    fprintf(OutFile, "\n");
    for (hunk = HunkHead2.next2; hunk; hunk = hunk->next2) {
	D(("  hunk #%ld (-> %ld)\n", hunk->hunkNum, hunk->hunkNum2));
	fprintf(OutFile, "\n\tXDEF\t_%s_%ld\n", hunk->unit->name,
		hunk->hunkNum);
	fprintf(OutFile, "_%s_%ld\tEQU\t$%04lx", hunk->unit->name,
		hunk->hunkNum, HunkOffsets[hunk->hunkNum2]);
	if (hunk->data) {
	    D(("  output %ld longs\n", hunk->size));
	    for (i = 0; i < hunk->size; i++) {
		if ((i & 3) == 0)
		    fprintf(OutFile, "\n\t\tdc.l\t");
		else
		    fprintf(OutFile, ",");
		fprintf(OutFile, "$%08lx", hunk->data[i]);
	    }
	    fprintf(OutFile, "\n");
	}
    }

    fprintf(OutFile, "\n\n\tXDEF\tCHAOSDATALONGS\n");
    fprintf(OutFile, "CHAOSDATALONGS\tEQU\t$%04lx\t\t; $%04lx bytes\n",
	    dataLength/4, dataLength);
    fprintf(OutFile, "\n\tXDEF\tCHAOSTOTALBYTES\n");
    fprintf(OutFile, "CHAOSTOTALBYTES\tEQU\t$%04lx\n", length);

    /* output relocation information */
    fprintf(OutFile, "\n\n\tXDEF\tCHAOSRelocTable\n");
    fprintf(OutFile, "CHAOSRelocTable:\n");
    length = 0;
    for (hunk = HunkHead2.next2; hunk; hunk = hunk->next2) {
	OptArg = hunk->unit->name;
	for (reloc = hunk->relocHead; reloc; reloc = reloc->next) {
	    if (reloc->hunkNum2 < 0)
		longjmp(Env, error_coderef);
	    if ((reloc->type != hunk_reloc32) && (reloc->type != hunk_dreloc32))
		longjmp(Env, error_smallref);
	    if (length == 0)
		fprintf(OutFile, ";\t\t\taddend,offset\n");
	    fprintf(OutFile, "\t\tdc.w\t$%04lx,$%04lx\n",
		    HunkOffsets[reloc->hunkNum2],
		    HunkOffsets[hunk->hunkNum2]+reloc->offset);
	    length++;
	}
	for (extDef = hunk->extDefHead; extDef; extDef = extDef->next) {
	    if ((extDef->type != ext_defa) && (extDef->type != ext_defr))
		longjmp(Env, error_extref);
	    fprintf(OutFile, "\tXDEF\t%s\n", extDef->name);
	    fprintf(OutFile, "%-23s EQU\t$%04lx\n", extDef->name,
		    HunkOffsets[hunk->hunkNum2]+extDef->value);
	}
    }
    fprintf(OutFile, "\n\tXDEF\tCHAOSRELOCSIZE\n");
    fprintf(OutFile, "CHAOSRELOCSIZE\tEQU\t%ld\n", length);
    fprintf(OutFile, "\n\tEND\n");
    D(("genAsm done.\n"));
}


/* input reader */
struct Unit *
inputUnit(unit)
struct Unit *unit;
{
    struct Unit *unitNext;
    struct Hunk *hunkPrev, *hunk;
    struct Reloc *relocPrev, *reloc;
    struct ExtDef *extDefPrev, *extDef;

    int errorCode;
    long hunkLong, size, ext;
    short hunkNumber;

    int i;

    D(("inputUnit\n"));
    hunkLong = hunk_end;

    /* create new unit entry */
    unitNext = mymalloc(sizeof(struct Unit));
    unit->next = unitNext;
    unit = unitNext;
    unit->next = 0;
    unit->name[0] = '\0';
    unit->hunkHead = 0;
    hunkNumber = 0;
    hunkPrev = (struct Hunk *) &unit->hunkHead;

    for (;;) {
	/* get next hunk */

	/* it's OK to find an EOF here... */
	if (errorCode = setjmp(Env)) {
	    if (hunkLong == hunk_end) {
		/* honorable exit */
		return(unit);
	    }
	    abEndGame(errorCode);
	}
	hunkLong = getlong();

	/* ... it's no longer OK to find an EOF */
	if (errorCode = setjmp(Env))
	    abEndGame(errorCode);

	if (hunkLong & MEMMASK)
	    longjmp(Env, error_memspec);

	switch (hunkLong) {
	  case hunk_unit:
	    size = getlong();
	    readname(unit->name, size);
	    unit->name[size*4] = '\0';
	    break;

	  case hunk_name:
	    size = getlong();
	    readname(HunkName, size);
	    HunkName[size*4] = '\0';
	    break;

	  case hunk_debug:
	    size = getlong();
	    tosslong(size);
	    break;

	  case hunk_code:
	  case hunk_data:
	  case hunk_bss:
	    size = getlong();
	    hunk = mymalloc(sizeof(struct Hunk));
	    hunkPrev->next = hunk;
	    hunkPrev = hunk;
	    hunk->next = 0;
	    hunk->next2 = 0;
	    hunk->unit = unit;
	    hunk->hunkNum = hunkNumber++;
	    hunk->hunkNum2 = -1;
	    hunk->type = hunkLong;
	    hunk->size = size;
	    strcpy(hunk->name, HunkName);
	    HunkName[0] = '\0';
	    hunk->relocHead = 0;
	    relocPrev = (struct Reloc *) &hunk->relocHead;
	    hunk->extDefHead = 0;
	    extDefPrev = (struct ExtDef *) &hunk->extDefHead;
	    if ((hunkLong != hunk_bss) && (hunk->size != 0)) {
		hunk->data = mymalloc(size*4);
		readlongs(hunk->data, size);
		if (hunkLong == hunk_data) {
		    for (i = 0; i < size; i++)
			if (hunk->data[i])
			    goto nonzeroData;
		    /* convert to bss */
		    hunk->type = hunk_bss;
		    free(hunk->data);
		    hunk->data = 0;
nonzeroData:;
		}
	    }
	    else
		hunk->data = 0;
	    break;

	  case hunk_reloc8:
	  case hunk_reloc16:
	  case hunk_reloc32:
	  case hunk_dreloc32:
	  case hunk_dreloc16:
	  case hunk_dreloc8:
	    for (;;) {
		size = getlong();
		if (size == 0)
		    break;
		
		ext = getlong();	/* actually hunk # */
		for (i = 0; i < size; i++) {
		    reloc = (struct Reloc *) mymalloc(sizeof(struct Reloc));
		    relocPrev->next = reloc;
		    relocPrev = reloc;
		    reloc->next = 0;
		    reloc->type = hunkLong;
		    reloc->hunkNum = ext;
		    reloc->hunkNum2 = -1;
		    reloc->offset = getlong();
		}
	    }
	    break;

	  case hunk_ext:
	    do {
		ext = getlong();
		if (ext != 0) {
		    size = ext & 0xffffff;
		    readname(TempName, size);
		    size = getlong();
		    switch ((ext >> 24) & 0xff) {
		      case ext_defr:
		      case ext_defa:
			extDef = (struct ExtDef *)
				mymalloc(sizeof(struct ExtDef));
			break;
		      case ext_ref32:
		      case ext_ref16:
		      case ext_ref8:
		      case ext_dref32:
		      case ext_dref16:
		      case ext_dref8:
			extDef = (struct ExtDef *)
				mymalloc(sizeof(struct ExtDef)+(size*4));
			for (i = 0; i < size; i++)
			    extDef->references[i] = getlong();
			break;
		      default:
			OptArg = (char *) ((ext >> 24) & 0xff);
			longjmp(Env, error_badext);
		    }
		    extDefPrev->next = extDef;
		    extDefPrev = extDef;
		    extDef->next = 0;
		    extDef->type = (ext >> 24) & 0xff;
		    extDef->value = size;
		    strcpy(extDef->name, TempName);
		}
	    }
		while (ext != 0);
	    break;

	  case hunk_symbol:
	    do {
		ext = getlong();
		if (ext != 0) {
		    size = ext & 0xffffff;
		    tosslong(size + 1);
		}
	    }
	    while (ext != 0);
	    break;

	  case hunk_end:
	    break;

	  case hunk_header:
	  case hunk_break:
	  case hunk_overlay:
	  case hunk_cont:
	  default:
	    errhunk(hunkLong);
	}
    }
    fclose(InFile);
}


main(argc, argv)
int argc;
char *argv[];
{
    FILE *listFile;
    struct Unit *unit;
    int errorCode;
    char chaosType;

    ProgName = *argv++;

    if (errorCode = setjmp(Env))
	abEndGame(errorCode);

    /* gather arguments */
    if (argc != 4)
	longjmp(Env, error_usage);

    OptArg = *argv++;
    if ((sscanf(OptArg, "%ld", &StartOffset) == 1) && (!(StartOffset&3)))
	chaosType = 'a';
    else if ((strlen(OptArg) != 1) || ((chaosType = OptArg[0]) != 'o'))
	longjmp(Env, error_option);

    OptArg = *argv++;
    if (!(OutFile = fopen(OptArg, "w")))
	longjmp(Env, error_oopen);

    OptArg = *argv;
    if (!(InFile = fopen(OptArg, "r")))
	longjmp(Env, error_iopen);

    switch (chaosType) {
      case 'a':
	listFile = InFile;
	unit = (struct Unit *) &UnitHead;
	while (fscanf(listFile, "%63s", TempName) == 1) {
	    D(("name \"%s\"\n", TempName));
	    OptArg = TempName;
	    if (!(InFile = fopen(OptArg, "r")))
		longjmp(Env, error_iopen);
	    unit = inputUnit(unit);
	}
	if (errorCode = setjmp(Env))
	    abEndGame(errorCode);
	genAsm();
	break;
      case 'o':
	inputUnit(&UnitHead);
	if (errorCode = setjmp(Env))
	    abEndGame(errorCode);
	genObject();
	break;
      default:
	longjmp(Env, error_option);
    }
}
