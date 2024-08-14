/************************************************************************

  Module :	Postscript "Dictionaries" administration  © Commodore-Amiga

  Purpose:	Initialising system name table, name space, systemdict and
			dictionary stack.
			Managing the creation of dictionaries, adding and removal
            of dict entries, look-up of dict entries.

  Internals: see docs.dict, compress.c

*************************************************************************/

#include "errors.h"
#include "exec/types.h"
#include "pstypes.h"
#include "stream.h"
#include "memory.h"
#include "objects.h"

#include "misc.h"
#include "stack.h"
#include "gstate.h"
#include "context.h"

#include "string.h"
#include "stdio.h"
#include "dict.h"

// Escape character bit fields for compressed names stream.
#define	SPECIAL		0x80
#define	INDEX		0x40
#define	OPERATOR	0x20
#define	COMMON		0x1F // 5 bits set aside to tell howmany chars are in
						 // common with start of previous string

//--------------------------------------------------------------------
IMPORT char *AddName		(DPSContext ctxt, char *);		// (Names.c)

IMPORT unsigned char encdat[];		// compressed system names
IMPORT int 		  opvectors[];		// list of operator function addresses

//--------------------------------------------------------------------
// My exported functions.

GLOBAL error   InitSysDict	(DPSContext);
GLOBAL pso 	  *InitDict		(DPSContext, int);
GLOBAL pso    *NameLookUp	(DPSContext, pso*);
GLOBAL pso    *FindDictEntry(DPSContext, pso*, pso*);
GLOBAL pso	  *FindOperator	(DPSContext, pso*);
GLOBAL BOOL	   Define		(DPSContext, pso*, pso *key, pso *val);

// Private dictionary support functions

static	void 	MakeSysDict (DPSContext);
static	char * AddDictEntry	(DPSContext, pso *dict,int type,char * name,int val);
static dict_entry *FindEmpty(pso *dict,int*);

//**********************************************************************
// Initialize "systemdict","shareddict","userdict" & "errordict"
// This routine creates the initial dictionary environment for Postscript
// programs.
// Currently this means filling systemdict with all the operators and some
// special non-TYPE_OPERATORs and creating the dictionary stack.
// Also create the system name table (array of 256 pointers to names).
//**********************************************************************

error InitSysDict (DPSContext ctxt) {

	pso	*sys_dict,*shr_dict,*usr_dict;	// The three fixed dictionaries
	pso	*err_dict;

// create empty dictionaries...
	sys_dict = InitDict(ctxt, SYS_DICT_SIZE);	// systemdict
		if(!sys_dict) return ERR_VMerror;
	shr_dict = InitDict(ctxt, SHR_DICT_SIZE);	// shareddict
		if(!shr_dict) return ERR_VMerror;
	usr_dict = InitDict(ctxt, USR_DICT_SIZE);	// userdict
		if(!usr_dict) return ERR_VMerror;
	err_dict = InitDict(ctxt, NUM_ERRORS+10);	// errordict
		if(!err_dict) return ERR_VMerror;

// Put handles to them in the context structure.
	ctxt->space.sysdict = dict_obj;				// skeleton dict objects
	ctxt->space.shrdict = dict_obj;
	ctxt->space.usrdict = dict_obj;
	ctxt->space.errdict = dict_obj;
												// fill in dictionary pointers
	ctxt->space.sysdict.obj.dictval = (g_obj*)sys_dict;
	ctxt->space.shrdict.obj.dictval = (g_obj*)shr_dict;
	ctxt->space.usrdict.obj.dictval = (g_obj*)usr_dict;
	ctxt->space.errdict.obj.dictval = (g_obj*)err_dict;

// Add all the operators that we've implemented so far to systemdict.
// And create system name table.

	MakeSysDict(ctxt);

// Now add some special entries to systemdict...
	AddDictEntry (ctxt, &ctxt->space.sysdict, TYPE_NULL,"null",0);
	AddDictEntry (ctxt, &ctxt->space.sysdict, TYPE_BOOL,"true",TRUE);
	AddDictEntry (ctxt, &ctxt->space.sysdict, TYPE_BOOL,"false",FALSE);
	AddDictEntry (ctxt, &ctxt->space.sysdict, TYPE_DICT,"systemdict",(int)sys_dict);
	AddDictEntry (ctxt, &ctxt->space.sysdict, TYPE_DICT,"shareddict",(int)shr_dict);
	AddDictEntry (ctxt, &ctxt->space.sysdict, TYPE_DICT,"userdict",(int)usr_dict);
	AddDictEntry (ctxt, &ctxt->space.sysdict, TYPE_DICT,"errordict",(int)err_dict);

// And finally create the dict stack by pushing sys,shared & usr dicts...

//	SAVE_LEVEL(&d);								// stuff save level in too.

	PUSHDICT(ctxt,&dict_obj);					// Push systemdict
	ctxt->space.stacks.d.sp->obj.dictval = (g_obj*) sys_dict;	// fill in dict pointer
//---------------------------------------------------
	PUSHDICT(ctxt,&dict_obj);					// Push shareddict
	ctxt->space.stacks.d.sp->obj.dictval = (g_obj*) shr_dict;
//---------------------------------------------------
	PUSHDICT(ctxt,&dict_obj);					// Push userdict
	ctxt->space.stacks.d.sp->obj.dictval = (g_obj*) usr_dict;

	return ERR_OK;
}

//** MakeSysDict *******************************************************
// Using compressed (operator)name table and operator function address
// array, construct system name table and add all operators to system-
// dict.
//**********************************************************************

void MakeSysDict(DPSContext ctxt) {

	register unsigned char *data,*ptr;
	register unsigned char code;
	register int *func;
	char stringbuf[50];
	int index;
	char **systab;

	systab = ctxt->space.SysNameTab;		// -> system name table
	data = encdat;							// -> start of compressed data
	func = opvectors;						// -> list of operator addresses

	while (*data) {							// for all compressed entries
		code = *data++;						// get code byte

		if (code & INDEX)					// if index follows,
			index = (int) *data++;			// pick it up

// howmany chars in common with previous name ?

		ptr = stringbuf + (code&COMMON);	// -> difference branch

		while (*data > 0 && *data < 0x80) {	// copy chars that differ from
			*ptr++ = *data++;				// previous name
		}
		*ptr = '\0';						// zero-terminate new name

		if (code & OPERATOR) {				// is this an operator name ?
			ptr = AddDictEntry(ctxt, &ctxt->space.sysdict,
						TYPE_OPERATOR | ATTR_EXECUTE, stringbuf, *func++);	

		} else {							// otherwise, just add to name
			ptr = AddName(ctxt, stringbuf);		// space
		}

// Fill in system name table as we go..

		if (code & INDEX) {
			if(index<=255)	systab[index] = ptr;
		}

	}
}

//** AddDictEntry *********************************************************
// This routine will add a dictionary entry to ANY dictionary.
// You pass it a dictionary OBJECT (i.e. NOT a dictionary itself!) and
// whatever you want to enter in that dictionary.
// No access checks are done (this routine to be used ONLY at init-time).
//**********************************************************************

char *AddDictEntry(DPSContext ctxt, pso *dict,int type,char * name,int val) {

	pso ob2;
	pso nameob={TYPE_NAME,0,0,0};

	nameob.obj.nameval = AddName(ctxt, name);		// ptr to VM'ed string

	ob2.type = type;
	ob2.len = ob2.tag = 0;
	ob2.obj.intval = val;							// anything !

// WITHOUT Overflow CHECKS **!! (shouldn't need it at init time !)
	Define(ctxt, dict, &nameob, &ob2);				// add "it"

	return nameob.obj.nameval;
}

//** InitDict **********************************************************
// Initialise a DICTIONARY.
// Sets all KEY entries to TYPE_NULL and inits child & parent links to 0.
// dict_entry[0] is a special entry that is NOT part of the real dictionary.
// It is used to store private dictionary stuff (length & maxlength).
// The resquested dict size (# of entries) is allowed to be zero !
//**********************************************************************

pso * InitDict(DPSContext ctxt, int entries) {

	dict_entry  *de,*dict;
	int i;

	// Get memory for a dictionary body (incl. special zero entry !)

	dict = AllocVM(VM, (entries+1)*sizeof(dict_entry) );

	if (dict) {
	    dict->key.len = 0;				// init 'length'
		dict->key.type = 0;				// clear type (NULL) and attributes

		dict->val.len = entries;		// init 'maxlength'

	    de = dict +1;					// skip entry zero (special !)
	    for (i=0; i<entries; i++,de++) {
			de->key.type = TYPE_NULL;	// empty dict contains NULLs

			de->child = 0; 				// init hash coll chain links
			de->parent = 0;
	    }

	} else {
	    if (verbose) PDEBUG ("Dictionary creation failed (REQUESTED SIZE:%d).\n",entries);
	}

	return (pso *) dict;	/* Maybe NULL ! */
}
//**********************************************************************
int hash (unsigned char * string, int strlen, int modulo);
int hash (unsigned char * string, int strlen, int modulo) {

	unsigned int hashval=0;
	register int i;

	for (i=0; i<strlen ;i++)
	    hashval += hashval<<3 ^ (*string++);

 	return (int) ((hashval % modulo)+1);
}

//*** Define ***********************************************************
// Enter (or modify) a key-value pair in a dictionary.
// Since Names are the most frequent type of keys, names are not stored/searched
// sequentially but entered and retrieved by hashing to find a hash-chain
// on which they are located.
//**********************************************************************

BOOL Define (DPSContext ctxt, pso *dict, pso *keyob, pso *valob) {

	int i,lasti,index;		// various indices in the dictionary array
	int	maxlen;				// # of real entries.
	char *str;				// name string ptr
	dict_entry *dbase;		// dictionary base address (fixed throughout)
	dict_entry *de,*dest;	// dictionary entry pointers
	dict_entry *parent,*child;	// when moving a node out of the way...

	maxlen = ((dict_entry*)dict->obj.dictval)->val.len;	// CAN BE 0 !!

//fprintf(OP,"Define() length,maxlength:%d,%d\n",
//			((dict_entry*)dict->obj.dictval)->key.len,
//			maxlen);

	if (! maxlen) return FALSE;							// ERR_dictfull

				 // Point to array of K-V pair entries (incl. entry 0 !!)
	dbase = (dict_entry*)dict->obj.dictval;

    if (OBJ_TYPE(keyob) == TYPE_NAME) {		// special case names ...

		str = keyob->obj.nameval;			// point to name string

		i = hash((unsigned char *)str, strlen(str), maxlen); // MODULO sizeof dict

		de = dbase+i;						// point to coll chain root.

		if (OBJ_TYPE(&de->key) == TYPE_NULL) {

			de->key = *keyob;		// now copy our argument into its
			de->val = *valob;		// rightful slot.
			de->child = 0;			// Since it's a brand new entry that
			de->parent = 0;			// didn't collide, chain ptrs are all NULL.

			dbase->key.len ++;				// incr # of used slots
			return TRUE;

		}

		if ( de->parent == 0) {		// if entry belongs here, follow hash chain...

			do {				// DO-WHILE !
				lasti = i;					// remember parent...
				i = de->child;

				if ( str == de->key.obj.nameval ) {

// if argument is already in dictionary, just modify entry

					de->val	= *valob;		// copy new value for entry
					return TRUE;
				}
				de = dbase+i;			// goto next chain link
			} while ( i ) ;		// DO-WHILE !

// we need to insert a new entry.
// check if dict has room for one more entry before doing so.

			if (dbase->key.len == maxlen) return FALSE;	// ERR_dictfull

			de = FindEmpty(dict,&index);			// find any empty slot.

			de->key = *keyob;			// now copy our argument into its
			de->val = *valob;			// rightful slot.
			de->child = 0;				// this entry is tail of chain
			de->parent = lasti;

			de = dbase + lasti;			// goto parent again
			de->child = index;			// link new entry to parent

			dbase->key.len ++;			// incr # of used slots

		} else {

// entry we want to add hashes to a slot already occupied by an entry in a
// hashchain belonging to another hashvalue chain !
// We need to move this "squatting" entry out of the way and update the links
// accordingly.

	// we need to insert a new entry.
	// check if dict has room for one more entry first...

			if (dbase->key.len == maxlen) return FALSE;	// ERR_dictfull
	
			dest = FindEmpty(dict,&index);			// find any empty slot
	
			*dest = *de;	// move 'squatting' entry out of the way.
	

	// now modify parent and (possible) child links to reflect
	// new entrie's position.

			parent = dbase + de->parent;		// point to parent & child
			child  = dbase + de->child;
	
		// only if moved node had a parent, modify that parent's child link!
	
			if (de->parent) {
				parent->child = index;
			}
	
		// only if moved node had a child, modify that child's parent link!
	
			if (de->child) {
				child->parent = index;
			}
	
			de->key = *keyob;		// now copy our argument into its
			de->val = *valob;		// rightful slot.
			de->child = 0;			// Since it's a brand new entry that
			de->parent = 0;			// didn't collide, chain ptrs are all NULL.

			dbase->key.len ++;				// incr # of used slots
		}	// ENDIF for if TYPE_NAME
//--------------------------------------------------------------------
	} else {	// "DEF" is asked to bind something to a non-name key...
				// Just find the first empty spot and insert it there.

		if (dbase->key.len == maxlen) return FALSE;	// ERR_dictfull

		de = FindEmpty(dict,&index);			// find any empty slot

		de->key = *keyob;	// now copy our argument into its
		de->val = *valob;	// rightful slot.
		de->child = 0;		// Non-name entries don't have hashchains, but
		de->parent = 0;		// the pointers are still "hot", so CLEAR them @!!

		dbase->key.len ++;				// incr # of used slots
	}
	return TRUE;
}

//** FindEmpty *********************************************************
// Find an empty slot in a guaranteed non-full dictionary. 
//**********************************************************************

static dict_entry *FindEmpty(pso *dict, int *index) {

	int maxlen;
	int i;

	register dict_entry *de;

	de = (dict_entry*)dict->obj.dictval;	// point to first real slot

	maxlen = (((pso*)de)+1) ->len;		// value obj in DE #0 !!!
	de++;								// skip DE #0

	for (i=0; i< maxlen; i++) {			// Do a linear search on dict

            if ( OBJ_TYPE(&de->key) == TYPE_NULL) {
				*index = i+1;			// return index of empty slot
				return de;				// and its absolute address.

			} else de++;				//check next slot
	}
}

//** NameLookUp ********************************************************
// This is the main routine for automatic name look-up (called mainly by
// the interpreter (but also by functions like where,load,etc..)).
// It scans each dictionary stacked on the dict stack for the key object.
// This routine has a side-effect of setting the global 'last_dict' to point
// to the last dictionary searched. This is used by operator 'where' **!!
//**********************************************************************

pso *NameLookUp(DPSContext ctxt, pso *keyobj) {

	int d;
	pso *entry;
	pso *dict;

	dict = ctxt->space.last_dict = DICTSP(ctxt);	// Point to TOS of Dict stack

	for (d=0; d <NUMDICT(ctxt); d++) {
	    entry = FindDictEntry(ctxt, dict, keyobj);	// scan a single dict

	    if (entry) return entry;

		ctxt->space.last_dict++;
		dict++;

	}
	return (pso *) NULL;
}

//** FindDictEntry *****************************************************
// Look-up a key in a specific (passed) dictionary.
// The 'dict' argument is a ptr to a standard DICTIONARY object.
// Return a pointer to the value of the key or NULL.
//**********************************************************************

pso * FindDictEntry(DPSContext ctxt, pso *dict, pso *any) {

	unsigned int hv;
	int i,j;
	char *s, *d;
	unsigned char *str,*tstr;
	register dict_entry *dbase, *de;

// Point to first dict entry K-V pair (incl. special 0 entry)
	dbase = ((dict_entry*)dict->obj.dictval);

// Guard against zero length dictionairies (which are legal !)
	
	if (! dbase->val.len) {				// test maxlength
		return NULL;
	}

    if (OBJ_TYPE(any) == TYPE_NAME) {	// in the case of looking up a name..

		str = any->obj.nameval;			// point to name string

		hv=0; tstr=str;					// hash it up to find dictionary slot
		while(*tstr) hv+=hv<<3^(*tstr++);	// to look at.
		hv=(hv % (dict->obj.dictval + 1)->len)+1;
		i=hv;

   	   	de = dbase+i;					// goto first chain entry

		if (OBJ_TYPE(&de->key) != OBJ_TYPE(any))	// if first is NULL or
			return NULL; // anything else than a name, this isn't a name hash chain.

		do {						// DO-WHILE !!

	    	if (str == de->key.obj.nameval)	// check if names are EQ
					return &de->val;		// return VALUE of K-V pair

			i = de->child;					// get link to next slot
	   	   	de = dbase+i;					// goto next chain entry

	    } while ( i );				// DO-WHILE !!

	// loop hit end of chain without finding the name : name doesn't exist.
		return NULL;				// (name) key object not in dictionary

// -----------------------------------------------------------------------
	} else {	// The object key is not a name, can't do a hash on them, so

		de = dbase;	

	    for (i=0; i< (dict->obj.dictval +1)->len; i++) {	// Do linear search..

		    if ( OBJ_TYPE(&de->key) == OBJ_TYPE(any) ) {

				if (OBJ_TYPE(any) == TYPE_STRING) {
					j = any->len;
					s = any->obj.stringval;
					d = de->key.obj.stringval;
					while (j--) {
						if (*s++ != *d++) break;
					}
					if (j== -1) return &de->val;

				} else {						

					if ( de->key.obj.intval == any->obj.intval) {
						return &de->val;
				    }
				}
			}
		de++;
	    }
	}
	return NULL;
}

//**********************************************************************
// FindOperator exists simply for 'cvs' only.

pso * FindOperator	(DPSContext ctxt, pso *operator) {

	register dict_entry *de;
	int i;

										// goto system dictionary
	de = (dict_entry*)ctxt->space.sysdict.obj.dictval;

	i = de++->val.len;					// maxlength number of entries to scan

	while (i--) {

		if (de->val.obj.operval == operator->obj.operval) {
			if (OBJ_TYPE(&de->key) != TYPE_NULL)
				return &de->key;		// return name object corresponding
		}								// to operator.
		de++;							// goto next dictionary slot
	}

	return NULL;
}
//**********************************************************************
