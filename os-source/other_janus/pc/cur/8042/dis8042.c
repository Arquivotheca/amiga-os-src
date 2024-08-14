/*
 * dis8042.c
 *
 * intel 8041/8741/8042/8742 microcontroller disassembler
 *
 * input:  binary dump from eprom programmer
 * output: assembly .lst style output (assembleable if you delete the first
 *		   16 bytes of each line)
 *
 * 01/08/91	rsd	created
 */

#include <stdio.h>

#define FALSE 0
#define TRUE !FALSE

typedef unsigned char UBYTE;
typedef unsigned int UWORD;

#define	CLASS_NUL	0x001	/* */
#define CLASS_ACC	0x002	/* A */
#define	CLASS_D8	0x004	/* #xx  (2nd byte) */
#define CLASS_A8	0x008	/* xx  (2nd byte) */
#define CLASS_A11	0x010	/* xxx  (2nd byte and bits 7..5 of 1st byte) */
#define CLASS_R3	0x020	/* Rx  (bits 2..0 of 1st byte) */
#define	CLASS_R1	0x040	/* Rx  (bit 0 of 1st byte) */
#define CLASS_IMP	0x080	/* */
#define CLASS_IND	0x100	/* @ */
#define CLASS_P2	0x200	/* Px  (bits 1..0 of 1st byte) */

struct inst {
	UBYTE	inst;		/* first byte of the instruction */
	char	*mnem;		/* mnemonics */
	UWORD	classl;		/* type of left operand */
	UWORD	classr;		/* type of right operand */
};

struct inst list[] = {
	{ 0x03,	"ADD",	CLASS_ACC, CLASS_D8 },
	{ 0x68,	"ADD",	CLASS_ACC, CLASS_R3 },
	{ 0x60, "ADD",	CLASS_ACC, CLASS_R1 },
	{ 0x13, "ADDC",	CLASS_ACC, CLASS_D8 },
	{ 0x78, "ADDC",	CLASS_ACC, CLASS_R3 },
	{ 0x70, "ADDC",	CLASS_ACC, CLASS_R1 },
	{ 0x53, "ANL",	CLASS_ACC, CLASS_D8 },
	{ 0x58, "ANL",	CLASS_ACC, CLASS_R3 },
	{ 0x50, "ANL",	CLASS_ACC, CLASS_R1 },
	{ 0x43, "ORL",	CLASS_ACC, CLASS_D8 },
	{ 0x48, "ORL",	CLASS_ACC, CLASS_R3 },
	{ 0x40, "ORL",	CLASS_ACC, CLASS_R1 },
	{ 0xd3, "XRL",	CLASS_ACC, CLASS_D8 },
	{ 0xd8, "XRL",	CLASS_ACC, CLASS_R3 },
	{ 0xd0, "XRL",	CLASS_ACC, CLASS_R1 },
	{ 0x37, "CPL",	CLASS_ACC, CLASS_NUL },
	{ 0x27, "CLR",	CLASS_ACC, CLASS_NUL },
	{ 0x57, "DA",	CLASS_ACC, CLASS_NUL },
	{ 0x07, "DEC",	CLASS_ACC, CLASS_NUL },
	{ 0x17, "INC",	CLASS_ACC, CLASS_NUL },
	{ 0xe7, "RL",	CLASS_ACC, CLASS_NUL },
	{ 0xf7, "RLC",	CLASS_ACC, CLASS_NUL },
	{ 0x77, "RR",	CLASS_ACC, CLASS_NUL },
	{ 0x67, "RRC",	CLASS_ACC, CLASS_NUL },
	{ 0x47, "SWAP",	CLASS_ACC, CLASS_NUL },
	{ 0xe8, "DJNZ",	CLASS_R3, CLASS_A8 },
	{ 0x12, "JB0",	CLASS_A8, CLASS_NUL },
	{ 0x32, "JB1",	CLASS_A8, CLASS_NUL },
	{ 0x52, "JB2",	CLASS_A8, CLASS_NUL },
	{ 0x72, "JB3",	CLASS_A8, CLASS_NUL },
	{ 0x92, "JB4",	CLASS_A8, CLASS_NUL },
	{ 0xb2, "JB5",	CLASS_A8, CLASS_NUL },
	{ 0xd2, "JB6",	CLASS_A8, CLASS_NUL },
	{ 0xf2, "JB7",	CLASS_A8, CLASS_NUL },
	{ 0xf6, "JC",	CLASS_A8, CLASS_NUL },
	{ 0xb6, "JF0",	CLASS_A8, CLASS_NUL },
	{ 0x76, "JF1",	CLASS_A8, CLASS_NUL },
	{ 0x04, "JMP",	CLASS_A11, CLASS_NUL },
	{ 0xb3, "JMPP",	CLASS_IND | CLASS_ACC, CLASS_NUL },
	{ 0xe6, "JNC",	CLASS_A8, CLASS_NUL },
	{ 0xd6, "JNIBF",	CLASS_A8, CLASS_NUL },
	{ 0x26, "JNT0",	CLASS_A8, CLASS_NUL },
	{ 0x46, "JNT1",	CLASS_A8, CLASS_NUL },
	{ 0x96, "JNZ",	CLASS_A8, CLASS_NUL },
	{ 0x86, "JOBF",	CLASS_A8, CLASS_NUL },
	{ 0x16, "JTF",	CLASS_A8, CLASS_NUL },
	{ 0x36, "JT0",	CLASS_A8, CLASS_NUL },
	{ 0x56, "JT1",	CLASS_A8, CLASS_NUL },
	{ 0xc6, "JZ",	CLASS_A8, CLASS_NUL },
	{ 0x05, "EN	I",	CLASS_IMP, CLASS_NUL },
	{ 0x15, "DIS	I",	CLASS_IMP, CLASS_NUL },
	{ 0xe5, "EN	DMA",	CLASS_IMP, CLASS_NUL },
	{ 0xf5, "EN	FLAGS",	CLASS_IMP, CLASS_NUL },
	{ 0xc5, "SEL	RB0",	CLASS_IMP, CLASS_NUL },
	{ 0xd5, "SEL	RB1",	CLASS_IMP, CLASS_NUL },
	{ 0x01, "HALT",	CLASS_NUL, CLASS_NUL },
	{ 0x82, "STOP	Z",	CLASS_IMP, CLASS_NUL },
	{ 0xe2, "STOP	H",	CLASS_IMP, CLASS_NUL },
	{ 0x23, "MOV",	CLASS_ACC, CLASS_D8 },
	{ 0xf8, "MOV",	CLASS_ACC, CLASS_R3 },
	{ 0xf0, "MOV",	CLASS_ACC, CLASS_IND | CLASS_R1 },
	{ 0xc7, "MOV	A,PSW",	CLASS_NUL, CLASS_NUL },
	{ 0xb8, "MOV",	CLASS_R3, CLASS_D8 },
	{ 0xa8, "MOV",	CLASS_R3, CLASS_ACC },
	{ 0xa0,	"MOV",	CLASS_IND | CLASS_R1, CLASS_ACC },
	{ 0xb0, "MOV",	CLASS_IND | CLASS_R1, CLASS_D8 },
	{ 0xd7, "MOV	PSW",	CLASS_NUL, CLASS_ACC },
	{ 0xa3, "MOVP",	CLASS_ACC, CLASS_IND | CLASS_ACC },
	{ 0xe3, "MOVP3",	CLASS_ACC, CLASS_IND | CLASS_ACC },
	{ 0x28, "XCH",	CLASS_ACC, CLASS_R3 },
	{ 0x20, "XCH",	CLASS_ACC, CLASS_IND | CLASS_R1 },
	{ 0x30, "XCHD",	CLASS_ACC, CLASS_IND | CLASS_R1 },
	{ 0xa7, "CPL	C",	CLASS_IMP, CLASS_NUL },
	{ 0x95, "CPL	F0",	CLASS_IMP, CLASS_NUL },
	{ 0xb5, "CPL	F1",	CLASS_IMP, CLASS_NUL },
	{ 0x97, "CLR	C",	CLASS_IMP, CLASS_NUL },
	{ 0x85, "CLR	F0",	CLASS_IMP, CLASS_NUL },
	{ 0xa5, "CLR	F1",	CLASS_IMP, CLASS_NUL },
	{ 0x98, "ANL",	CLASS_P2, CLASS_D8 },
	{ 0x9c, "ANLD",	CLASS_P2, CLASS_ACC },
	{ 0x22, "IN	A,DBB",	CLASS_NUL, CLASS_NUL },
	{ 0x08, "IN",	CLASS_ACC, CLASS_P2 },
	{ 0x0c, "MOVD",	CLASS_ACC, CLASS_P2 },
	{ 0x3c, "MOVD",	CLASS_P2, CLASS_ACC },
	{ 0x90, "MOV	STS",	CLASS_NUL, CLASS_ACC },
	{ 0x8c, "ORLD",	CLASS_P2, CLASS_ACC },
	{ 0x88, "ORL",	CLASS_P2, CLASS_D8 },
	{ 0x02, "OUT	DBB",	CLASS_NUL, CLASS_ACC },
	{ 0x38, "OUTL",	CLASS_P2, CLASS_ACC },
	{ 0xc8,	"DEC",	CLASS_R3, CLASS_NUL },
	{ 0x18,	"INC",	CLASS_R3, CLASS_NUL },
	{ 0x10, "INC",	CLASS_IND | CLASS_R1, CLASS_NUL },
	{ 0x14, "CALL",	CLASS_A11, CLASS_NUL },
	{ 0x83, "RET",	CLASS_NUL, CLASS_NUL },
	{ 0x93, "RETR",	CLASS_NUL, CLASS_NUL },
	{ 0x25, "EN	TCNTI",	CLASS_IMP, CLASS_NUL },
	{ 0x35, "DIS	TCNTI",	CLASS_IMP, CLASS_NUL },
	{ 0x42, "MOV	A,T",	CLASS_NUL, CLASS_NUL },
	{ 0x62, "MOV	T,A",	CLASS_NUL, CLASS_NUL },
	{ 0x65, "STOP	TCNT",	CLASS_IMP, CLASS_NUL },
	{ 0x45, "STRT	TCNT",	CLASS_IMP, CLASS_NUL },
	{ 0x55, "STRT	T",	CLASS_IMP, CLASS_NUL },
	{ 0x00, "NOP",	CLASS_NUL, CLASS_NUL }
};

FILE *fin;		/* input file */
int pc;			/* program counter */

void order_list()
{
	/* arrange list of instructions such that the ones with the most bits
	 * defined show up first
	 */

	return; 
}

int get_a_byte(UBYTE *inst)
{
	if (fread(inst, 1, 1, fin) == 1)
		return 0;	/* good! */
	else
		return 1;	/* bad */
}

int twobytes(UWORD classl, UWORD classr) 
{
	if (classl & CLASS_D8 || classl & CLASS_A8 || classl & CLASS_A11)
		return TRUE;

	if (classr & CLASS_D8 || classr & CLASS_A8 || classr & CLASS_A11)
		return TRUE;

	return FALSE;
}

char *doclass(UWORD class, UBYTE inst1, UBYTE inst2)
{
static char s[80], s1[80];

	if (class & CLASS_NUL)		/* */
		return 0;

	if (class & CLASS_ACC)		/* A */
		strcpy(s, "A");
	
	if (class & CLASS_D8)		/* #xx  (2nd byte) */
		sprintf(s, "#%02xH ;%d", inst2, inst2);

	if (class & CLASS_A8)		/* xx  (2nd byte) */
		sprintf(s, "%04xH", inst2 + (pc & 0xff00));
	
	if (class & CLASS_A11)		/* xxx  (2nd byte and bits 7..5 of 1st byte) */
		sprintf(s, "%04xH", inst2 + ((inst1 & 0xe0) << 3));

	if (class & CLASS_R3)		/* Rx  (bits 2..0 of 1st byte) */
		sprintf(s, "R%d", inst1 & 0x07);
	
	if (class & CLASS_R1)		/* Rx  (bit 0 of 1st byte) */
		sprintf(s, "R%d", inst1 & 0x01);
	
	if (class & CLASS_IMP)		/* */
		strcpy(s, "");

	if (class & CLASS_P2)		/* Px  (bits 1..0 of 1st byte) */
		sprintf(s, "P%d", inst1 & 0x03);

	if (class & CLASS_IND) {
		strcpy(s1, s);
		strcpy(s, "@");
		strcat(s, s1);
	}

	return s;
}

int match(UBYTE inst1, UBYTE inst, UWORD classl, UWORD classr)
{
	/* based on the class of this inst, chop off bits from the original
	 * inst (inst1) and see if the result matches inst.
	 */

	if (classl & CLASS_A11)	/* xxx  (2nd byte and bits 7..5 of 1st byte) */
		inst1 &= ~0xe0;

	if (classl & CLASS_R3)	/* Rx  (bits 2..0 of 1st byte) */
		inst1 &= ~0x07;

	if (classl & CLASS_R1)	/* Rx  (bit 0 of 1st byte) */
		inst1 &= ~0x01;

	if (classl & CLASS_P2)	/* Px  (bits 1..0 of 1st byte) */
		inst1 &= ~0x03;

	if (classr & CLASS_A11)	/* xxx  (2nd byte and bits 7..5 of 1st byte) */
		inst1 &= ~0xe0;

	if (classr & CLASS_R3)	/* Rx  (bits 2..0 of 1st byte) */
		inst1 &= ~0x07;

	if (classr & CLASS_R1)	/* Rx  (bit 0 of 1st byte) */
		inst1 &= ~0x01;

	if (classr & CLASS_P2)	/* Px  (bits 1..0 of 1st byte) */
		inst1 &= ~0x03;

	if (inst1 == inst)
		return TRUE;
	else
		return FALSE;
}

int disassemble()
{
UBYTE inst1, inst2;
char outline[256];
char t[80];
int i;
char *str;

	/*                0123456789012345 */
	sprintf(outline, "%04x :          ", pc);
	
	if (get_a_byte(&inst1))
		return 0;
	pc++;

	sprintf(t, "%02x", inst1);
	memcpy(&outline[7], t, 2);

	for (i = 0; i < sizeof(list) / sizeof(struct inst); i++) {
		if (match(inst1, list[i].inst, list[i].classl, list[i].classr)) {
			strcat(outline, "\t");
			strcat(outline, list[i].mnem);

			if (twobytes(list[i].classl, list[i].classr)) {
				if (get_a_byte(&inst2))
					return 0;
				pc++;
				sprintf(t, "%02x", inst2);
				memcpy(&outline[10], t, 2);
			}
			
			str = doclass(list[i].classl, inst1, inst2);
			if (str) {
				strcat(outline, "\t");
				strcat(outline, str);		
			}
			
			str = doclass(list[i].classr, inst1, inst2);
			if (str) {
				strcat(outline, ",");
				strcat(outline, str);		
			}

			break;
		}
	}	

	/* none of the above */
	if (i == (sizeof(list) / sizeof(struct inst))) {
		sprintf(t, "\tDB\t%02x", inst1);
		strcat(outline, t);
	}

	printf("%s\n", outline);

	return 1;
}

void main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: dis8042 binaryfile\n");
		exit(1);
	}

	if (!(fin = fopen(argv[1], "rb"))) {
		fprintf(stderr, "where is %s?\n", argv[1]);
		exit(1);
	}

	order_list();
	
	pc = 0;
	while (disassemble())
		;

	fclose(fin);
}
