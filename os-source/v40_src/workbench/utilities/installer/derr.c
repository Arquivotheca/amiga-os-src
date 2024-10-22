/*	derr.c - dos error text in layperson terms */


#ifdef LATTICE
char *det120[] = {
	"Command line for program too long",
	"Could not run a program -- was not an object module",
	"Invalid resident library"
};

char det209[] = "Could not perform a DOS operation -- action not available";
char det218[] = "Unable to access disk -- disk was not in drive";

char *det201[] = {
	"Installation problem -- no default drawer",
	"Unable to access a file or drawer -- object in use",
	"Unable to create a file or drawer -- object with same name already exists",
	"Unable to locate a drawer",
	"Unable to locate a file or drawer",
	"Invalid window specification for a console",
	"Could not run a program -- insufficient memory",
	det209,
	det209,
	"Invalid name for a file or drawer",
	"Invalid lock on a file or drawer",
	"Could not perform operation on a file or drawer -- not of required type",
	"Unable to write to a disk -- disk not validated",
	"Unable to write to a disk -- disk was write protected",
	"Unable to rename a file or drawer -- attempted to rename across devices",
	"Could not delete a drawer -- drawer was not empty",
	"Unable to create drawer - nesting too deep",
	det218,
	"Unable to properly access a file -- seek error",
	"Unable to copy a filenote - comment too big",
	"Could not write to disk - the disk was full",
	"Unable to delete a file or drawer -- it was delete protected",
	"Unable to write to a file or drawer -- it was write protected",
	"Unable to access a file or drawer -- it was read protected",
	"Unable to access a disk -- not in AmigaDOS format",
	det218
};
#else
extern char *det120[], *det201[];
#endif

char *DosErrorText(long num)
{	if (num == 103 || num == -1)
		return "Not enough memory for installation to continue";
	if (num == 105) return "Too many DOS tasks";
	if (num >= 120 && num <= 122) return (det120[num-120]);
	if (num >= 201 && num <= 226) return (det201[num-201]);
	/* if (num == -1001) return "User aborted install"; */
	return "Cause of disk-related error unknown.";
}

#ifndef LATTICE
#asm

		dseg
_det120
		dc.l	det120
		dc.l	det121
		dc.l	det122

det120	dc.b	"Command line for program too long",0
det121	dc.b	"Could not run a program -- was not an object module",0
det122	dc.b	"Invalid resident library",0
		ds.w	0

_det201
		dc.l	det201
		dc.l	det202
		dc.l	det203
		dc.l	det204
		dc.l	det205
		dc.l	det206
		dc.l	det207
		dc.l	det208
		dc.l	det209
		dc.l	det210
		dc.l	det211
		dc.l	det212
		dc.l	det213
		dc.l	det214
		dc.l	det215
		dc.l	det216
		dc.l	det217
		dc.l	det218
		dc.l	det219
		dc.l	det220
		dc.l	det221
		dc.l	det222
		dc.l	det223
		dc.l	det224
		dc.l	det225
		dc.l	det226

det201	dc.b	"Installation problem -- no default drawer",0
det202	dc.b	"Unable to access a file or drawer -- object in use",0
det203	dc.b	"Unable to create a file or drawer -- object with same name already exists",0
det204	dc.b	"Unable to locate a drawer",0
det205	dc.b	"Unable to locate a file or drawer",0
det206	dc.b	"Invalid window specification for a console",0
det207	dc.b	"Could not run a program -- insufficient memory",0
det208	; dc.b	"Could not perform operation on a file or drawer -- not of required type",0
det209	dc.b	"Could not perform a DOS operation -- action not available",0
det210	dc.b	"Invalid name for a file or drawer",0
det211	dc.b	"Invalid lock on a file or drawer",0
det212	dc.b	"Could not perform operation on a file or drawer -- not of required type",0
det213	dc.b	"Unable to write to a disk -- disk not validated",0
det214	dc.b	"Unable to write to a disk -- disk was write protected",0
det215	dc.b	"Unable to rename a file or drawer -- attempted to rename across devices",0
det216	dc.b	"Could not delete a drawer -- drawer was not empty",0
det217	dc.b	"Unable to create drawer - nesting too deep",0
det226
det218	dc.b	"Unable to access disk -- disk was not in drive",0
det219	dc.b	"Unable to properly access a file -- seek error",0
det220	dc.b	"Unable to copy a filenote - comment too big",0
det221	dc.b	"Could not write to disk - the disk was full",0
det222	dc.b	"Unable to delete a file or drawer -- it was delete protected",0
det223	dc.b	"Unable to write to a file or drawer -- it was write protected",0
det224	dc.b	"Unable to access a file or drawer -- it was read protected",0
det225	dc.b	"Unable to access a disk -- not in AmigaDOS format",0
;det226	dc.b	"No disk in drive",0
		ds.w	0

		cseg
#endasm
#endif

#if 0

/* #define ERROR_NO_MORE_ENTRIES		  232 */
#define ERROR_IS_SOFT_LINK		  233
#define ERROR_OBJECT_LINKED		  234
#define ERROR_BAD_HUNK			  235
#define ERROR_RECORD_NOT_LOCKED		  240
#define ERROR_LOCK_COLLISION		  241
#define ERROR_LOCK_TIMEOUT		  242
#define ERROR_UNLOCK_ERROR		  243

#define ERROR_BAD_TEMPLATE		  114
#define ERROR_BAD_NUMBER		  115
#define ERROR_REQUIRED_ARG_MISSING	  116
#define ERROR_KEY_NEEDS_ARG		  117
#define ERROR_TOO_MANY_ARGS		  118
#define ERROR_UNMATCHED_QUOTES		  119

#endif
