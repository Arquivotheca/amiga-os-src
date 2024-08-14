/************************************************************************
 *									*
 *	Copyright (c) 1989 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */

/*
 * Packages or modules which are intended for use by many application
 * programs should return an error Class ( i.e. ERC_something ) but
 * not an error number.  The assumption is that the calling program
 * would OR an error number which means something to it with the error
 * class which was returned.
 */

/*
 * Application programs should always set an error Class and probably
 * an error code which can be used to pinpoint exactly where in the
 * program the error message is being generated.
 */

typedef int ERRORCODE;

/*	Error Classes	*/

#define ERC_BITS		0x3F00

#define ERC_NONE		0x0100	/* none of the errorcodes apply */
#define ERC_NO_MEMORY		0x0200	/* Could not get enough memory */
#define ERC_NO_LIBRARY		0x0300	/* Could not open a library */
#define ERC_NO_DEVICE		0x0400	/* Could not open a device */
#define ERC_NO_RESOURCE		0x0500
#define ERC_IO_FAILED		0x0600
#define ERC_NO_PORT		0x0700	/* Could not open a message port */
#define ERC_NO_MESSAGE		0x0800	/* Could not create message struct */
#define ERC_NO_FILE		0x0900	/* Could not open a file */
#define ERC_NO_WINDOW		0x0A00	/* Could not open a new window */
#define ERC_NO_SCREEN		0x0B00	/* Could not open a new screen */
#define ERC_NO_SIGNAL		0x0C00	/* Could not allocate a signal bit */
#define ERC_UNIMPLEMENTED	0x0D00	/* For an unimplemented feature */
#define ERC_PACKAGE		0x0E00	/* A packaged function which returns
					 * no error code failed */
#define ERC_USER		0x0F00	/* Caused by user cancel/abort */

/* Error Flags */

#define ERF_BITS		0xC000
#define ERF_RESERVED		0x4000	/* for future expansion */
#define ERF_PRIVATE		0x8000	/* Not totaly defined yet */


/* Error numbers */

#define ERR_BITS		0x00FF

#define ERR01	0x0001
#define ERR02	0x0002
#define ERR03	0x0003
#define ERR04	0x0004
#define ERR05	0x0005
#define ERR06	0x0006
#define ERR07	0x0007
#define ERR08	0x0008
#define ERR09	0x0009

#define ERR10	0x0010
#define ERR11	0x0011
#define ERR12	0x0012
#define ERR13	0x0013
#define ERR14	0x0014
#define ERR15	0x0015
#define ERR16	0x0016
#define ERR17	0x0017
#define ERR18	0x0018
#define ERR19	0x0019

#define ERR20	0x0020
#define ERR21	0x0021
#define ERR22	0x0022
#define ERR23	0x0023
#define ERR24	0x0024
#define ERR25	0x0025
#define ERR26	0x0026
#define ERR27	0x0027
#define ERR28	0x0028
#define ERR29	0x0029

#define ERR30	0x0030
#define ERR31	0x0031
#define ERR32	0x0032
#define ERR33	0x0033
#define ERR34	0x0034
#define ERR35	0x0035
#define ERR36	0x0036
#define ERR37	0x0037
#define ERR38	0x0038
#define ERR39	0x0039

#define ERR40	0x0040
#define ERR41	0x0041
#define ERR42	0x0042
#define ERR43	0x0043
#define ERR44	0x0044
#define ERR45	0x0045
#define ERR46	0x0046
#define ERR47	0x0047
#define ERR48	0x0048
#define ERR49	0x0049

#define ERR50	0x0050
#define ERR51	0x0051
#define ERR52	0x0052
#define ERR53	0x0053
#define ERR54	0x0054
#define ERR55	0x0055
#define ERR56	0x0056
#define ERR57	0x0057
#define ERR58	0x0058
#define ERR59	0x0059

#define ERR60	0x0060
#define ERR61	0x0061
#define ERR62	0x0062
#define ERR63	0x0063
#define ERR64	0x0064
#define ERR65	0x0065
#define ERR66	0x0066
#define ERR67	0x0067
#define ERR68	0x0068
#define ERR69	0x0069

#define ERR70	0x0070
#define ERR71	0x0071
#define ERR72	0x0072
#define ERR73	0x0073
#define ERR74	0x0074
#define ERR75	0x0075
#define ERR76	0x0076
#define ERR77	0x0077
#define ERR78	0x0078
#define ERR79	0x0079

#define ERR80	0x0080
#define ERR81	0x0081
#define ERR82	0x0082
#define ERR83	0x0083
#define ERR84	0x0084
#define ERR85	0x0085
#define ERR86	0x0086
#define ERR87	0x0087
#define ERR88	0x0088
#define ERR89	0x0089

#define ERR90	0x0090
#define ERR91	0x0091
#define ERR92	0x0092
#define ERR93	0x0093
#define ERR94	0x0094
#define ERR95	0x0095
#define ERR96	0x0096
#define ERR97	0x0097
#define ERR98	0x0098
#define ERR99	0x0099



/*
 *	Function prototypes for exported functions.
 */

extern void querybailout( char *, ERRORCODE );
extern void bailout( char *, ERRORCODE );
extern int mention_error( char *, ERRORCODE );
extern int retry_error( char *, ERRORCODE );
extern int ask_question( char * );
extern void tell_user( char *string );
