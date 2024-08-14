#ifndef JANUS_JFTRANS_H
#define JANUS_JFTRANS_H

/****************************************************************************
 * (PC side file)
 *
 * JFtrans.h -- Flag definitions for using j_file_transfer in ljanus.lib
 *
 * Copyright (c) 1988 Commodore Amiga Inc. All rights reserved.
 *
 * 7-15-88 - Bill Koester - Created this file
 ***************************************************************************/

/* JFT transfer modes */
#define	JFT_CR_LF			0
#define	JFT_BINARY			1

/* JFT conversion modes */
#define	JFT_NO_CONVERT		0
#define	JFT_CONVERT			1

/* JFT transfer directions */
#define	JFT_PC_AMIGA		0
#define	JFT_AMIGA_PC		1

/*	JFT Error codes	*/
#define	JFT_NOERROR								0
#define 	JFT_ERR_INVALID_MODE					1
#define	JFT_ERR_INVALID_DIRECTION			2
#define	JFT_ERR_NO_SERVER						3
#define	JFT_ERR_PC_OPEN						4
#define	JFT_ERR_AMIGA_OPEN					5
#define	JFT_ERR_AMIGA_READ					6
#define	JFT_ERR_AMIGA_WRITE					7
#define 	JFT_ERR_INVALID_CONVERSION_MODE	8
#define  JFT_ERR_PC_DIRECTORY					9

#ifdef	LINT_ARGS
int j_file_transfer(char *, char *, int, int,unsigned char *,int);
#else
int j_file_transfer();
#endif

#endif	/* End of JANUS_JFTRANS_H conditional compilation                 */
