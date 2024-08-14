/*
 *		derr.c
 *
 *	Display DOS error messages as some form of readable text rather than
 *	as a cryptic number.
 *
 *	Isn't there a dos call that does this?
 *
 *
 *	Revision History:
 *
 *	lwilton	07/11/93:
 *		General cleanup and reformatting to work with SAS 6.x and the
 *		standard header files.
 *	lwilton 07/19/93:
 *		Deleted Manx/Aztec code.  Now SAS C only.  Also rewrote as a switch
 *		statement to make it more readable.
 */


/*	derr.c - dos error text in layperson terms */


char *DosErrorText (long num)
{
	switch (num)
	{
	case  -1:
	case 103:	return "Not enough memory for installation to continue";
	case 105:	return "Too many DOS tasks";

	case 120:	return "Command line for program too long";
	case 121:	return "Could not run a program -- was not an object module";
	case 122:	return "Invalid resident library";

	case 201:	return "Installation problem -- no default drawer";
	case 202:	return "Unable to access a file or drawer -- object in use";
	case 203:	return "Unable to create a file or drawer -- "
							"object with same name already exists";
	case 204:	return "Unable to locate a drawer";
	case 205:	return "Unable to locate a file or drawer";
	case 206:	return "Invalid window specification for a console";
	case 207:	return "Could not run a program -- insufficient memory";
	case 208:
	case 209:	return "Could not perform a DOS operation -- "
							"action not available";
	case 210:	return "Invalid name for a file or drawer";
	case 211:	return "Invalid lock on a file or drawer";
	case 212:	return "Could not perform operation on a file or drawer -- "
							"not of required type";
	case 213:	return "Unable to write to a disk -- disk not validated";
	case 214:	return "Unable to write to a disk -- disk was write protected";
	case 215:	return "Unable to rename a file or drawer -- "
							"attempted to rename across devices";
	case 216:	return "Could not delete a drawer -- drawer was not empty";
	case 217:	return "Unable to create drawer - nesting too deep";
	case 219:	return "Unable to properly access a file -- seek error";
	case 220:	return "Unable to copy a filenote - comment too big";
	case 221:	return "Could not write to disk - the disk was full";
	case 222:	return "Unable to delete a file or drawer -- "
							"it was delete protected";
	case 223:	return "Unable to write to a file or drawer -- "
							"it was write protected";
	case 224:	return "Unable to access a file or drawer -- "
							"it was read protected";
	case 225:	return "Unable to access a disk -- not in AmigaDOS format";
	case 218:	
	case 226:	return "Unable to access disk -- disk was not in drive";

	case -1001:	/* return "User aborted install"; */
	
	default:	return "Cause of disk-related error unknown.";
	}
}


#if 0

/* #define ERROR_NO_MORE_ENTRIES		232 */
#define ERROR_IS_SOFT_LINK		  		233
#define ERROR_OBJECT_LINKED		  		234
#define ERROR_BAD_HUNK			  		235
#define ERROR_RECORD_NOT_LOCKED		  	240
#define ERROR_LOCK_COLLISION		  	241
#define ERROR_LOCK_TIMEOUT		  		242
#define ERROR_UNLOCK_ERROR		  		243

#define ERROR_BAD_TEMPLATE		  		114
#define ERROR_BAD_NUMBER		  		115
#define ERROR_REQUIRED_ARG_MISSING	  	116
#define ERROR_KEY_NEEDS_ARG		  		117
#define ERROR_TOO_MANY_ARGS		  		118
#define ERROR_UNMATCHED_QUOTES		  	119

#endif
