#ifndef		RESOURCES_TUPLES_H
#define		RESOURCES_TUPLES_H

/*
* ======================================================================
* ======================================================================
* ======================================================================
* Tuple TPL_CODEs
* ======================================================================
* ======================================================================
* ======================================================================
*/

#include	<exec/types.h>

#define		CISTPL_NULL	0x00
#define		CISTPL_DEVICE	0x01

/*
** 2-7 reserved for future, upwards compatable device information tuples
*/

/*
** 8-F reserved for future, incompatable device information tuples
*/

#define		CISTPL_CHECKSUM		0x10
#define		CISTPL_LONGLINK_A	0x11
#define		CISTPL_LONGLINK_C	0x12
#define		CISTPL_LINKTARGET	0x13
#define		CISTPL_NO_LINK		0x14
#define		CISTPL_VERS_1		0x15
#define		CISTPL_ALTSTR		0x16
#define		CISTPL_DEVICE_A		0x17
#define		CISTPL_JEDEC_C		0x18
#define		CISTPL_JEDEC_A		0x19

#define		CISTPL_VERS_2		0x40
#define		CISTPL_FORMAT		0x41
#define		CISTPL_GEOMETRY		0x42
#define		CISTPL_BYTEORDER	0x43
#define		CISTPL_DATE		0x44
#define		CISTPL_BATTERY		0x45
#define		CISTPL_ORG		0x46

#define		CISTPL_END		0xFF

/*
* ======================================================================
* ======================================================================
* Tuple structure definitions (Level 1)
* ======================================================================
* ======================================================================
*/

/* CISTPL_NULL tuple (ignore) */

struct	TP_Null {
	UBYTE	TPL_CODE;
};

/* CISTPL_LONGLINK_A or CISTPL_LONGLINK_C (link to next tuple chain) */

struct	TP_LongLink {
	UBYTE	TPL_CODE;
	UBYTE	TPL_LINK;
	UBYTE	TPLL_ADDR[4];
};


/* CISTPL_LINKTARGET (validate link) */

struct	TP_LinkTarget {
	UBYTE	TPL_CODE;
	UBYTE	TPL_LINK;
	UBYTE	TPLTG_TAG[3];			/* 'C','I','S' */
};

/* CISTPL_NOLINK (no implied CISTPL_LONGLINK_C tuple) */

struct	TP_NoLink {
	UBYTE	TPL_CODE;
	UBYTE	TPL_LINK;
};


/* CISTPL_END (end of list tuple) */

struct  TP_EndOfList {
	UBYTE	TPL_CODE;
};

/* CISTPL_CHECKSUM */

struct	TP_CheckSum {
	UBYTE	TPL_CODE;
	UBYTE	TPL_LINK;
	UBYTE	TPLCKS_ADDR[2];
	UBYTE	TPLCKS_LEN[2];
	UBYTE	TPLCKS_CS[1];
};


/*
* ======================================================================
* ======================================================================
* CISTPL_DEVICE, CISTPL_DEVICE_A
* ======================================================================
* ======================================================================
*
* CISTPL_DEVICE or CISTPL_DEVICE_A (variable length tuple)
*/

struct	TP_Device {
	UBYTE	TPL_CODE;
	UBYTE	TPL_LINK;
	UBYTE	TP_DEVICE_INFO[4];
};

#define	DTYPE_NULL	0
#define	DTYPE_ROM	1
#define	DTYPE_OTPROM	2
#define	DTYPE_EPROM	3
#define	DTYPE_EEPROM	4
#define	DTYPE_FLASH	5
#define	DTYPE_SRAM	6
#define	DTYPE_DRAM	7
#define	DTYPE_IO	0x0d

/*
* ======================================================================
* ======================================================================
* CISTPL_VERS_1 tuple
* ======================================================================
* ======================================================================
*/

struct	TP_Vers_1 {
	UBYTE	TPL_CODE;
	UBYTE	TPL_LINK;
	UBYTE	TPLLV1_MAJOR[1];
	UBYTE	TPLLV1_MINOR[1];
	UBYTE	TPLLV1_INFO[1];
};

/*
* ======================================================================
* ======================================================================
* CISTPL_VERS_2 tuple
* ======================================================================
* ======================================================================
*/

struct	TP_Vers_2 {
	UBYTE	TPL_CODE;
	UBYTE	TPL_LINK;
	UBYTE	TPLLV2_VERS[1];
	UBYTE	TPLLV2_COMPLY[1];
	UBYTE	TPLLV2_DINDEX[2];
	UBYTE	TPLLV2_RSV6[1];
	UBYTE	TPLLV2_RSV7[1];
	UBYTE	TPLLV2_VSPEC8[1];
	UBYTE	TPLLV2_VSPEC9[1];
	UBYTE	TPLLV2_NHDR[1];
	UBYTE	TPLLV2_OEM[1];

};

/* TPLLV2_OEM, and TPLLV2_INFO follows (variable size ) */

/*
* ======================================================================
* ======================================================================
* CISTPL_DATE and CISTPL_BATTERY tuple
* ======================================================================
* ======================================================================
*
*/

struct	TP_Date {
	UBYTE	TPL_CODE;
	UBYTE	TPL_LINK;
	UBYTE	TPLDATE_TIME[2];
	UBYTE	TPLDATE_DAY[2];
};

/*
* CISTPL_BATTERY (date when battery was replaced, and date which it is
* expected that it will need to be replaced)
*/

struct	TP_Battery {
	UBYTE	TPL_CODE;
	UBYTE	TPL_LINK;
	UBYTE	TPLBATT_RDAY[2];
	UBYTE	TPLBATT_XDAY[2];
};

/*
*
* Byte	7	6	5	4	3	2	1	0
* ---------------------------------------------------------------------
* TIME	MMM (lo)		SSS
* 	HHH					MMM (hi)
*
* DAY	MON (lo)		DAY
*	YYY							MON (hi)
*
*
* if both fields are 0, date, and time were unknown when card was inited
*
*/

#define	DATE_TIME_MMM_LO_MASK	0xE0
#define	DATE_TIME_MMM_LO_SHIFT	5

#define	DATE_TIME_SSS_MASK	0x1F

#define	DATE_TIME_HHH_MASK	0xF8
#define	DATE_TIME_HHH_SHIFT	3

#define	DATE_DAY_MON_LO_MASK	0xE0
#define	DATE_DAY_MON_LO_SHIFT	5

#define	DATE_DAY_DAY_MASK	0x1F

#define	DATE_DAY_YYY_MASK	0xFE
#define	DATE_DAY_YYY_SHIFT	1

#define	DATE_DAY_MON_HI_MASK	0x01


/*
* ======================================================================
* ======================================================================
* CISTPL_FORMAT tuple
* ======================================================================
* ======================================================================
*
*/

struct	TP_Format {
	UBYTE	TPL_CODE;
	UBYTE	TPL_LINK;
	UBYTE	TPLFMT_TYPE[1];
	UBYTE	TPLFMT_EDC[1];
	UBYTE	TPLFMT_OFFSET[4];
	UBYTE	TPLFMT_NBYTE[4];
	UBYTE	TPLFMT_BKSZ[2];
	UBYTE	TPLFMT_NBLOCKS[4];
	UBYTE	TPLFMT_EDCLOC[4];
};

#define	TPLFMTTYPE_DISK	0
#define	TPLFMTTYPE_MEM	1


#define	TPLFMT_EDC_MASK	 0x78
#define	TPLFMT_EDC_SHIFT 3

#define	TPLFMTEDC_NONE	0
#define	TPLFMTEDC_CHSUM	1
#define	TPLFMTEDC_CRC	2
#define	TPLFMTEDC_PCC	3

#define	TPLFMT_LENGTH_MASK	0x07

/*
* ======================================================================
* ======================================================================
* CISTPL_GEOMETRY tuple
* ======================================================================
* ======================================================================
*
* CISTPL_GEOMETRY (shall appear in partition tuples for disk-like
* partitions).  It provides instructions to OS's that require all
* mass-storage devices be divided into cylinders, tracks, and sectors.
*
* The product:
*
* TPLGEO_NCYL * TPLGEO_TPC * TPLGEO_SPT must be <= TPLFMT_NBLOCKS
*
*/

struct	TP_Geometry {
	UBYTE	TPL_CODE;
	UBYTE	TPL_LINK;
	UBYTE	TPLGEO_SPT[1];
	UBYTE	TPLGEO_TPC[1];
	UBYTE	TPLGEO_NCLI[2];
};


#endif	/* RESOURCES_TUPLES_H */
