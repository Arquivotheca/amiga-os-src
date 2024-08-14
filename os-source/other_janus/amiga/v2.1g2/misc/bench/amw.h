/*
 *	amw.h - Header file for PC side Janus benchmark
 *
 *	Copyright (C) 1990 Active Circuits, Inc.
 *
 *	Permission granted to Commodore Business Machines to use
 *	this code solely for the purpose of internal testing of
 *	Amiga Janus software/hardware performance.
 */

/* Amiga <---> IBM Command Server, PC Side */

#define	BENCH_APPLICATION_ID	123457L
#define	BENCH_LOCAL_ID		5
#define	BUFSIZE			(16 * 1024)

/* Commands */
#define	TC_EXIT		0x04	/* Exit PC Server program */
#define	TC_RESET	0x07	/* PC to go back to wait for new service */

/* Additional commands for benchmarking PC/AT and PC/XT implementations */
#define	TC_ACK		0x08	/* Simply ACK, don't do anything */
#define	TC_MEM_READ	0x09	/* Read every byte of the Janus buffer */
#define TC_MEM_COPY	0x0A	/* Copy Janus buffer to non-Janus buffer */
#define	TC_MEM_DUPL	0x0B	/* Copy non-Janus memory around */
#define	TC_MEM_TARGA	0x0C	/* Copy Janus memory to Targa board */
#define	TC_CHECK_JANUS	0x0D	/* PC to check Janus memory has values written by Amiga */


struct BenchMessage {
	unsigned char	Command, Result, pad1, pad2;
	unsigned char	Buffer[BUFSIZE];
};