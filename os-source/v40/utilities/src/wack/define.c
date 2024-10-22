
/***********************************************************************
*
*			   G R A N D W A C K
*
************************************************************************
*
*	Copyright (C) 1985, Commodore-Amiga. All rights reserved.
*
************************************************************************
*
*   Source Control:
*
*	$Id: define.c,v 1.5 91/04/24 20:51:58 peter Exp $
*
*	$Locker:  $
*
*	$Log:	define.c,v $
 * Revision 1.5  91/04/24  20:51:58  peter
 * Changed $Header to $Id.
 * 
 * Revision 1.4  90/01/15  13:04:21  jimm
 * disable control-D quit
 * 
 * Revision 1.3  89/11/22  16:23:02  kodiak
 * 1.5: Forbid()/Permit() using BGACK hold; "faster" symbol loading option.
 * 
 * Revision 1.2  88/01/21  13:36:26  root
 * kodiak's copy of jimm's version, snapshot jan 21
 * 
*
***********************************************************************/
/***********************************************************************
*
*	Wack -- Key and Symbol Support
*
***********************************************************************/

typedef void (*voidfunc)();

#include "symbols.h"
#include "watch.h"

extern short watch;

extern Conclude();

/* turn off control-D exit */
#define	CTRL_D	4
#define	NULLKEY	(0) /* control-@ */

struct KeyMap TopMap;
struct SymbolMap TopSymMap;

struct KeyMap *ThisMap;
char ThisKey;
extern long CurrAddr;

int Hash();

char *TypeNames[] = {
    "constant",
    "primitive",
    "offset",
    "base",
    "keymap"
};

PutError(s,a)
    char *s;
    int a;
{
    puts (" ");
    printf (s, a);
    puts (" ");
}

char *
UnCtrl(c,s)
    char c;
    char *s;
{
    char *t = s;

    if (c < ' ') {
	*t++ = '^';
	*t++ = 'A' - 1 + c;
    }
    else
	if (c != 0x7f) {
	    *t++ = c;
	}
    *t = 0;

    return s;
}

ShowKeyMap(keyMap)
    struct KeyMap *keyMap;
{
    struct KeyAction   *kp;
    char    s1[6];
    char    s2[6];

    puts (" ");
    for (kp = keyMap -> head; kp; kp = kp -> next) {
	if (kp -> lastKey == 0) {
	    printf ("key '%s'", UnCtrl (kp -> firstKey, s1));
	}
	else {
	    UnCtrl (kp -> firstKey, s1);
	    UnCtrl (kp -> lastKey, s2);
	    printf ("keys '%s'-'%s'", s1, s2);
	}
	printf (" bound to '%s'\n", kp -> symbol -> name);
    }
}

ShowTopKeyMap()
{
    ShowKeyMap (&TopMap);
}

struct KeyAction *
FindKey(key, keyMap)
    char key;
    struct KeyMap *keyMap;
{
    struct KeyAction   *kp;

    for (kp = keyMap -> head; kp; kp = kp -> next) {
	if (key == kp -> firstKey) {
	    return kp;
	}
	if ((key >= kp -> firstKey) && (key <= kp -> lastKey)) {
	    return kp;
	}
    }
    return 0;
}


DoKey(key)
    char key;
{
    struct KeyAction   *kp;

    kp = FindKey (key, ThisMap);
    if (kp == 0) {
     /* PutError ("no function bound to key %c in this context", key); */
        putchar (7);
	return 0;
    }
    Evaluate (kp -> symbol, "");	/* jimm: added second argument */
}


BindMacro (name, type, body)
    char *name;
    short *type;
    char *body;
{

}

struct Symbol *
NewSymbol(name, symMap)
    char *name;
    struct SymbolMap *symMap;
{
    char   *newName = (char *) malloc (strlen (name) + 1);
    struct Symbol  *sp = (struct Symbol *) malloc (sizeof (struct Symbol));

    strcpy (newName, name);
    sp -> name = newName;
    if (symMap -> tail[Hash (name)] == 0) {
	symMap -> head[Hash (name)] = sp;
    }
    else {
	symMap -> tail[Hash (name)] -> next = sp;
    }
    symMap -> tail[Hash (name)] = sp;
    sp -> next = (struct Symbol *) 0;
    sp -> stype = sp -> value1 = sp -> value2 = 0;
    return sp;
}


struct Symbol *
FindSymbol(name, symMap)
    char *name;
    struct SymbolMap *symMap;
{
    struct Symbol   *sp;

  /* printf ("hash is %d\n for %s", Hash(name),name); */
    for (sp = symMap -> head[Hash(name)]; sp; sp = sp -> next) {
	if (strcmp (name, sp -> name) == 0) {
	    return sp;
	}
    }
    return 0;
}


ShowSymbol(sp)
    struct Symbol *sp;
{
    if (watch)
	printf ("{%06lx} ", sp);
    printf ("%10s  %08lx  %08lx  %s\n", TypeNames[sp -> stype],
	    sp -> value1, sp -> value2, sp -> name);
}


ShowSymbolMap(symMap)
    struct SymbolMap *symMap;
{
    struct Symbol  *sp;
    int h;

    puts (" ");
    for (h = 0; h < HASHSIZE; h++) {
	for (sp = symMap -> head[h]; sp; sp = sp -> next) {
	    ShowSymbol (sp);
	}
    }
}


Fixup(symMap)
    struct SymbolMap *symMap;
{
    struct Symbol  *sp;
    int     h;
    char    s[40];

    puts ("\n  generating symbol addresses ...");
    for (h = 0; h < HASHSIZE; h++) {
	for (sp = symMap -> head[h]; sp; sp = sp -> next) {
/*	    printf ("%s\n", sp->name); */
	    if ((sp -> stype) == ACT_OFFSET) {
		sprintf (s, "`hunk_%lx", sp->value2);
		sp->value2 = GetValue(s);
	    }
	}
    }
}

FixupTopMap()
{
    Fixup (&TopSymMap);
}

ShowTopSymMap()
{
    ShowSymbolMap (&TopSymMap);
}

ApproxSym()
{
    struct Symbol  *sp,
                   *cp;
    long    v,
            lv;
    int	    h;
    char    s[20];
    cp = 0;
    lv = 0x1000000;

    for (h = 0; h < HASHSIZE; h++) {
	for (sp = TopSymMap.head[h]; sp; sp = sp -> next) {
	    if (sp -> stype == ACT_OFFSET) {
		v = (sp->value2) + (sp -> value1);
		if ((CurrAddr - v >= 0) && (CurrAddr - v < lv)) {
		    cp = sp;
		    lv = CurrAddr - v;
		}
	    }
	}
    }
    if (cp != 0)
	printf ("\n%s + %lx   (%lx + %lx = %lx)", cp -> name, lv,
		cp -> value2, cp -> value1, v);
    else
	printf ("\nYou're lost.");
    puts (" ");
}

struct KeyAction *
BindKey (key, mapName, name)
    char key;
    char *mapName;
    char *name;
{
    struct KeyAction   *kp;
    struct KeyMap  *keyMap;
    struct Symbol  *sp,
                   *spkm;

    sp = FindSymbol (name, &TopSymMap);
    if (sp == 0) {
	PutError ("cannot bind key to unknown symbol");
	return 0;
    }

    spkm = FindSymbol (mapName, &TopSymMap);
    if (spkm == 0) {
	PutError ("cannot bind key to unknown keymap");
	return 0;
    }

    if (spkm -> stype != ACT_KEYMAP) {
	PutError ("cannot bind key to non-keymap type symbol");
	return 0;
    }    

    keyMap = (struct KeyMap *) spkm -> value1;

    kp = FindKey (key, keyMap);
    if (kp == 0) {
	kp = (struct KeyAction *) malloc (sizeof (struct KeyAction));
    }

    keyMap -> tail -> next = kp;
    keyMap -> tail = kp;
    kp -> next = 0;
    kp -> firstKey = key;
    kp -> lastKey = 0;
    kp -> symbol = sp;

    return kp;
}

struct KeyAction *
BindSet (firstKey, lastKey, mapName, name)
    char firstKey, lastKey;
    char *mapName;
    char *name;
{
    struct KeyAction   *kp;

    kp = BindKey (firstKey, mapName, name);
    if (kp != 0)
	kp -> lastKey = lastKey;
}

struct Symbol *
BindValue1(name, vtype, value)
    char *name;
    short vtype;
    long value;
{
    struct Symbol  *symbol;

    symbol = FindSymbol (name, &TopSymMap);
    if (symbol == 0) {
	symbol = NewSymbol (name, &TopSymMap);
    }

    symbol -> stype = vtype;
    symbol -> value1 = value;
}

struct Symbol *
BindValue2(name, vtype, value)
    char *name;
    short vtype;
    long value;
{
    struct Symbol  *symbol;

    symbol = NewSymbol (name, &TopSymMap);

    symbol -> stype = vtype;
    symbol -> value1 = value;
}


BindPrim(name, function)
    char *name;
    voidfunc function;
{
    BindValue1 (name, ACT_PRIM, (long) function);
}


struct Symbol exit_sym = {
    (struct Symbol *) 0, "`exit", ACT_PRIM, (long) Conclude, 0
};

struct KeyAction ctrl_d = {
#if 1
    (struct KeyAction *) 0, NULLKEY, 0, &exit_sym
#else
    (struct KeyAction *) 0, CTRL_D, 0, &exit_sym
#endif
};


Interp()
{
    while (1) {
	DoKey (ThisKey = getchar ());
    }
}


TryIt()
{
    int     h;

    ThisMap = &TopMap;
    TopMap.head = &ctrl_d;
    TopMap.tail = &ctrl_d;

    for (h = 0; h < HASHSIZE; h++) {
	TopSymMap.head[h] = 0;
	TopSymMap.tail[h] = 0;
    }


    BindValue1 ("TopMap", ACT_KEYMAP, &TopMap);
    InitTopMap ();

    InitGather ();

}
