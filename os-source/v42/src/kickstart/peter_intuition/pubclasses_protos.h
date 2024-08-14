/* Prototypes for functions defined in
pubclasses.c
 */

int InitClassList(void);

struct List * lockClassList(void);

int unlockClassList(void);

Class * FindClass(ClassID classid);

Class * MakeClass(ClassID classid,
                  int superid,
                  Class * superclass,
                  UWORD instsize,
                  int flags);

Class * makePublicClass(ClassID classid,
                        Class * superid,
                        UWORD instsize,
                        ULONG (* dispatch)());

int AddClass(Class * cl);

int RemoveClass(Class * cl);

int FreeClass(Class * cl);

