#ifndef OBJECTS_H
#define OBJECTS_H

/*
 * the interpreter deals entirely with the following kinds of objects
 * IMMED is a special case for the scanner only, it's the value used
 * for the binary token encoding of an immediately evaluated name.
 */
enum object_types {
	TYPE_NULL,		// $00 a NULL object, does nothing (also free dict entry)
	TYPE_INT,		// $01 a 32 bit signed integer
	TYPE_REAL,		// $02 a single precision float
	TYPE_NAME,		// $03 a name to be looked up in dictionaries
	TYPE_BOOL,		// $04 a boolean with 0=FALSE or 1=TRUE
	TYPE_STRING,	// $05 array of uchars (don't confuse with 'C' strings)
	TYPE_IMMED,		// $06 //name (only used by the scanner)
	TYPE_DICT,		// $07 a dictionary
	TYPE_PACKED,	// $08 a packed array of postscript objects
	TYPE_ARRAY,		// $09 a normal array of postscript objects (or proc)
	TYPE_MARK,		// $0A a special stack marker for counttomark etc.
	TYPE_FILE,		// $0B actually a stream object or executable file
	TYPE_OPERATOR,	// $0C direct pointer to operator function entry point
	TYPE_FONTID,	// $0D a font dictionary FID object
	TYPE_SAVE,		// $0E a save level

// DPS Extended Types
	TYPE_LOCK,		// $0F context locking semaphore
	TYPE_GSTAT,		// $10 Graphics State handle
	TYPE_COND,		// $11 Condition
	TYPE_STOP,		// $12 Special object type to force Interp() to return immediately
};

/* each object is literal or executable and has access attributes */

#define TYPE_MASK 			0x1F	// mask off the type field
#define ATTR_MASK 			0x80	// mask off the attribute field
#define ACCESS_MASK 		0x60	// mask off the access field

#define ATTR_LITERAL 		0x00	// gets pushed on the operand stack
#define ATTR_EXECUTE 		0x80	// get pushed on the execution stack

#define ACCESS_UNLIMITED 	0x00	// can do anything to this object
#define ACCESS_READ_ONLY 	0x20	// can only read this object
#define ACCESS_EXEC_ONLY 	0x40	// can only execute this object
#define ACCESS_NONE 		0x60	// no access at all to this object

#define	SAVE_MASK			0x1F	// save level 00..31
#define	__FREE_BIT__		0x20	//
#define	TRACKED				0x40	// obj value is allocated and in tracked VM
#define ALLOCATED			0x80	// obj value is allocated (vs. static)

/* most operations can work on a generic object without knowing the exact type */
typedef struct {
	uchar type;	   // enumerated object_type
	uchar tag;	   // used for save level & allocation bits 
	uword len;	   // length of composite or hashvalue of name objects
	unsigned long val; /* simple object or ptr to composite object */
} g_obj;

/* composite objects need separate definitions for the stuff pointed to but
 * we'll make a union for each possible kind of object to make coding easy.
 *
 * *NOTE* need to fix definitions that are currently g_obj * but really need
 * to be pointers to that kind of object.  Do it when they're defined.
 */
typedef struct {
    uchar type;		/* type and exec/literal attributes + access */
    uchar tag;
    uword len;		/* size of this object (hashvalue for name objects) */
    union {
	int    nullval;		/* null,type=0,tag=0,len=0 */
	int    intval;		/* integer,type=1,tag=0,len=0 */
	float  realval;		/* real,type=2,tag=0,len=0 */
	uchar  *nameval;	/* name,type=3,tag=namelength,len=hashvalue */
	int    boolval;		/* boolean,type=4,tag=0,len=0  */
	uchar  *stringval;	/* string,type=5,tag=0,len=stringlength */
	int    immediate;	/* immediately evaluated name  (scanner only) */
	g_obj  *dictval;	/* dictionary,type=7,tag=DICT_TYPE,len=maxentries */
	uchar  *packval;	/* packed array, type=8,tag=0,len=total size */
	g_obj  *arrayval;	/* array,type=9,tag=0,len=maxentries */
	int    markval;		/* mark,type=10,tag=0,len=0 */
	stream *streamval;	/* file,type=11,tag=0,len=0 */
	int    (*operval)(void *);	/* operator,type=12,tag=0,len=0 */
	g_obj  *fontval;	/* fontID,type=13,tag=0,len=0 */
	g_obj  *gstateval;	/* gstate,type=14,tag=0,len=0 */
	g_obj  *lockval;	/* lock,type=15,tag=0,len=0 */
	g_obj  *saveval;	/* save,type=16,tag=0,len=0 */
	int    guardval;	/* special stack guard entry */
    } obj;
} ps_obj;

#endif OBJECTS_H
