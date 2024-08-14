/* $Id: symbols.h,v 1.4 91/04/24 20:54:37 peter Exp $	*/

#define GetBlock getBlock
#define Print printf

#define HASHSIZE	27

#define ACT_CONSTANT	0
#define ACT_PRIM	1
#define ACT_OFFSET	2
#define ACT_BASE	3
#define ACT_KEYMAP	4

struct Symbol {
    struct Symbol *next;
    char *name;
    short stype;
    long value1;
    long value2;
};


struct KeyAction {
    struct KeyAction *next;
    char firstKey;
    char lastKey;
    struct Symbol *symbol;
};

struct KeyMap {
    struct KeyAction *head;
    struct KeyAction *tail;
};

struct SymbolMap {
    struct Symbol *head[HASHSIZE];
    struct Symbol *tail[HASHSIZE];
};

