
//
// Here's a list of our function prototypes, such that we
// can use automatic registerization on them.  This should
// both help stack usage, and also speed the program up a bit.
//
ULONG rcp(void);
int GetResponse(struct GlobalData *g);
void SendError(struct GlobalData *g, STRPTR errmsg);
BOOL GetUnixData(struct GlobalData *g, STRPTR filename, ULONG *mode, ULONG *size);
int SendFile(struct GlobalData *g, STRPTR filename);
int ChangeDirectory(struct GlobalData *g, STRPTR path);
int ExitDirectory(struct GlobalData *g);
int FromUsToThem(struct GlobalData *g, STRPTR path);
int ToUsFromThem(struct GlobalData *g, STRPTR f);
BOOL IsWild(STRPTR x, struct GlobalData *g);
void strdncpy(char *to, char *from, int length, struct GlobalData *g);
struct RDArgs *MyReadArgs(UBYTE *template, ULONG *table, struct RDArgs *ra, struct GlobalData *g);
extern __asm int myatol(register __a0 char *m);

