/* eof character */
#define ENDSTREAMCH		-1

/* buffered IO buffer size */
#define SCB_BUFFERSIZE		(49*4)

/* BCPL global offset local globals start at */
#define UG			150
#define FG			UG

/* # of globals before the global pointer */
#define NEGATIVE_GLOBAL_SIZE	50

/* error report types */
#define REPORT_STREAM		0	/* a stream */
#define REPORT_TASK		1	/* a process */
#define REPORT_LOCK		2	/* a lock */
#define REPORT_VOLUME		3	/* a volume node */

/* random from manhdr */
#define ABORT_DISK_ERROR	296
#define ABORT_BUSY		288

#define ACTION_STARTUP		0

/* resident segment structure - should be getvec()ed */
/* variable sized structure (name starts at name) */
/* should be getvec()ed */

struct Segment {
	BPTR seg_Next;
	LONG seg_UC;
	BPTR seg_Seg;
	char seg_Name[4];	/* actually the first 4 chars of BSTR name */
};

/* cli command path node */
/* should be getvec()ed */

struct Path {
	BPTR path_Next;
	BPTR path_Lock;
};
