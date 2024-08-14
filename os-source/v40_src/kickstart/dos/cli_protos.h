/* cli_protos.h */

LONG __regargs cli_init(struct DosPacket *pkt);
struct DosList * __regargs createnode(LONG unit,
			   char *devname);
struct MsgPort * __regargs startup(struct DosList *node,
				   LONG *syssegl,
				   LONG boottime,
				   struct ExpansionBase *eb);
LONG __regargs make_task_assignment(char *name,
				    char *dir);

LONG __regargs make_assign_entry(char *name);
void __regargs make_node_entry(char *name,
			       LONG startup,
			       BPTR seglist,
			       char *handler,
			       LONG stksize,
			       LONG pri,
			       LONG gv);
void __regargs makes(char *name,
		     LONG startup,
		     LONG stacksize);
void __regargs make_node_entries(LONG *syssegl);
LONG ASM makeseg(REG(d1) char *name,REG(d2) BPTR seg,REG(d3) LONG system);
void __regargs readprefs(void);
ULONG __regargs MaxCli(void);
struct Process * ASM FindCli(REG(d1) ULONG num);
LONG __regargs cli_init_run (struct DosPacket *pkt);
void __regargs run_init_clip (struct CommandLineInterface *clip, BPTR outs,
			      struct CommandLineInterface *oclip,
			      struct DosPacket *pkt);
LONG ASM myNameFromLock (REG(d1) BPTR lock, REG(d2) char *buffer,
			 REG(d3) LONG buflen);
BPTR __regargs copylist (BPTR p);
LONG __regargs cli_init_newcli (struct DosPacket *pkt);
BPTR __regargs finddefault (char *file);

/* cli.h */
#ifdef OLD_CLI
void ASM start (REG(d1) struct DosPacket *parm_pkt);
BPTR __regargs load (char *file,BPTR dir,LONG *codep,LONG flag);
void __regargs getr2 (LONG *codep);
LONG __regargs load_and_run (BPTR module,struct CommandLineInterface *clip);
BPTR __regargs openf (LONG f);
LONG __regargs err (char *m,BPTR si,BPTR so);
#endif

void __regargs kill_cli_num (LONG tnum);

struct Segment * ASM searchsegs (REG(d1) UBYTE *name,
				 REG(d2) struct Segment *start,
				 REG(d3) LONG system);
LONG ASM RemSegment (REG(d1) struct Segment *segment);
LONG ASM RunCommand (REG(d1) BPTR seg, REG(d2) LONG stack, REG(d3) UBYTE *args,
		     REG(d4) LONG length);

/* external */
#ifndef USE_FPTR
LONG ASM call_bcpl_fptr (REG(d1) struct DosPacket *parm_packet,
			 REG(d2) LONG fptr);
#endif

LONG ASM FindArg  (REG(d1) UBYTE *keyword, REG(d2) UBYTE *template);
LONG ASM ReadItem (REG(d1) UBYTE *name, REG(d2) LONG maxchars,
		   REG(d3) struct CHSource *CHSource);
struct RDArgs * ASM ReadArgs (REG(d1) UBYTE *keys, REG(d2) LONG *argv,
			      REG(d3) struct RDArgs *control);
void ASM FreeArgs (REG(d1) struct RDArgs *control);

/* rdargshack.c */

struct RDArgs * ASM RealReadArgs (REG(d1) UBYTE *keys, REG(d2) LONG *argv,
			          REG(d3) struct RDArgs *control);
/* execute.c */

LONG ASM execute (REG(d1) char *comm,				/* GLOBAL */
		  REG(d2) BPTR instr,
		  REG(d3) BPTR outstr);
LONG ASM system (REG(d1) char *comm,				/* NEW */
		 REG(d2) struct TagItem *tags);
LONG __regargs internal_run (char *command, char *shell, BPTR instr,
			     BPTR outstr, LONG type, struct TagItem *tags);
LONG __regargs find_tnum (struct CliProcList **clip);
