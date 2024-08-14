#define D(a)	printf a

#include <stdio.h>

#define  UNUSED		-32768
short relseek[11] = {
    UNUSED, UNUSED, UNUSED, UNUSED, UNUSED,
    UNUSED, UNUSED, UNUSED, UNUSED, UNUSED, UNUSED
};
short reldata[11] = {
    UNUSED, UNUSED, UNUSED, UNUSED, UNUSED,
    UNUSED, UNUSED, UNUSED, UNUSED, UNUSED, UNUSED
};

main(int argc, char *argv[])
{
    FILE *hIn, *bOut;
    int nibble, byte, loc;
    char lowNibble, charMode, escapeChar;

    lowNibble = 0;
    charMode = 0;
    escapeChar = 0;
    loc = 0;
    if (argc == 3) {
	if (hIn = fopen(argv[1], "r")) {
	    if (bOut = fopen(argv[2], "w")) {
		byte = 0;
		while ((nibble = fgetc(hIn)) != EOF) {
		    if (charMode) {
			if ((!escapeChar) && (nibble == '"')) {
			    nibble = 0;
			    charMode = 0;
			}
			fputc(nibble, bOut);
			loc++;
			continue;
		    }
		    if (!isspace(nibble)) {
			if (nibble == '"') {
			    charMode = 1;
			    if (lowNibble != 0) {
				printf("ERROR: odd nibble %lx before \","
					" data lost\n", byte);
				lowNibble = 0;
			    }
			    continue;
			}
			if (nibble == '<') {
			    if (((nibble = fgetc(hIn)) == EOF) ||
				    (!((('0' <= nibble) && (nibble <= '9')) ||
				    (nibble == '-')))) {
				printf("ERROR: invalid relocation definition"
					"0x%x\n", nibble);
				continue;
			    }
			    if (nibble == '-') {
				D(("<- defined $%lx\n", loc));
				reldata[10] = loc;
			    }
			    else {
				D(("<%lc defined $%lx\n", nibble, loc));
				reldata[nibble-'0'] = loc;
			    }
			    continue;
			}
			if (nibble == '>') {
			    if (((nibble = fgetc(hIn)) == EOF) ||
				    (!((('0' <= nibble) && (nibble <= '9')) ||
				    (nibble == '-')))) {
				printf("ERROR: invalid relocation definition"
					"0x%x\n", nibble);
				continue;
			    }
			    if (nibble == '-') {
				if (((nibble = fgetc(hIn)) == EOF) ||
					(!(('0' <= nibble) &&
					(nibble <= '9')))) {
				    printf("ERROR: invalid relative"
					    "relocation definition 0x%x\n",
					    nibble);
				    continue;
				}
				D((">-%lc referenced $%lx\n", nibble, loc));
				relseek[nibble-'0'] = -loc;
			    }
			    else {
				D((">-%lc referenced $%lx\n", nibble, loc));
				relseek[nibble-'0'] = loc;
			    }
			    fputc(0, bOut);
			    fputc(0, bOut);
			    loc += 2;
			    continue;
			}
			if ((('0' <= nibble) && (nibble <= '9')) ||
				(('a' <= nibble) && (nibble <= 'f')) ||
				(('A' <= nibble) && (nibble <= 'F'))) {
			    byte <<= 4;
			    if (('0' <= nibble) && (nibble <= '9'))
				byte |= nibble-'0';
			    else
				byte |= (nibble&0x7)+9;
			    if (lowNibble++) {
				fputc(byte, bOut);
				loc++;
				byte = 0;
				lowNibble = 0;
			    }
			}
			else {
			    printf("ERROR: illegal input 0x%lx '%c'\n",
				    nibble, nibble);
			}
		    }
		}
	    }
	    else
		printf("ERROR: cannot open <output-file> \"%s\"\n", argv[2]);
	}
	else
	    printf("ERROR: cannot open <hex-file> \"%s\"\n", argv[1]);
    }
    else
	printf("USAGE: shex <hex-file> <output-file>\n");
    if (lowNibble)
	printf("ERROR: odd nibble %lx, data lost\n", byte);
    for (loc = 0; loc < 10; loc++) {
	if (relseek[loc] != UNUSED) {
	    if (relseek[loc] >= 0) {
		fseek(bOut, relseek[loc], 0);
		fwrite((char *)&reldata[loc], 2, 1, bOut);
		D(("patched [$%lx] to $%04lx\n", relseek[loc], reldata[loc]));
	    }
	    else {
		fseek(bOut, -relseek[loc], 0);
		reldata[loc] -= reldata[10];
		fwrite((char *)&reldata[loc], 2, 1, bOut);
		D(("patched [$%lx] to $%04lx\n", -relseek[loc], reldata[loc]));
	    }
    	}
    }
}
