// This file contains Dictionary information.
// March 1991 L.V. © CBM-Amiga

// Dictionaries are currently arrays of 'maxlength+1' dict_entry structures.
// Array element 0 is not a valid entry and is used to hold the dict's
// 'length' and 'maxlength' (in key->len and val->len resp.).
// Dictionary objects (TYPE_DICTs) contain a ptr (dictval) to element 0, NOT
// element 1.
// Unlike other composite objects, dictionary attributes are NOT stored in
// the dictionary objects pointing to a dictionary but instead are stored
// IN the dictionary itself. The key of entry 0 holds these attributes.

// Entries have forw and back links to manage hash collision chains.

#ifndef	DICT_H
#define	DICT_H

// **!! systemdict is too big here. This is for development only. Tune once done.
#define	SYS_DICT_SIZE	500	// # of entries in ...
#define	SHR_DICT_SIZE	100	// # of entries in ...
#define	USR_DICT_SIZE	200	// # of entries in ...

typedef struct tdef_dict_entry { // This is a Dictionary entry, i.e. a
    ps_obj key;			// KEY-VALUE pair (Key holds length and attributes !)
    ps_obj val;
    uword  child;		// hash collision-chain next node index
    uword  parent;		// hash collision-chain prev node index (BELONGS if 0)
} dict_entry;

#endif	DICT_H
