/* dbwin.h */

#ifdef RAM_DEBUG
extern void dbwrite(char *);
extern void dbwrite1(char *,long);
extern void dbwrite2(char *,long,long);

#define DBUG(x)		dbwrite(x)
#define DBUG1(x,y)	dbwrite1((x),(y))
#define DBUG2(x,y,z)	dbwrite2((x),(y),(z))
#else
/*
#define DBUG(x)		kprintf(x "\n")
#define DBUG1(x,y)	kprintf(x "\n",(y))
#define DBUG2(x,y,z)	kprintf(x "\n",(y),(z))
*/
#define DBUG(x)
#define DBUG1(x,y)
#define DBUG2(x,y,z)
#endif
