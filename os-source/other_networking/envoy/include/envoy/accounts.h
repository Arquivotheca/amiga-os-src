#ifndef	ENVOY_ACCOUNTS_H
#define ENVOY_ACCOUNTS_H

struct UserInfo
{
	UBYTE	ui_UserName[32];
	UWORD	ui_UserID;
	UWORD	ui_PrimaryGroupID;
	ULONG   ui_Flags;
};

struct GroupInfo
{
	UBYTE	gi_GroupName[32];
	UWORD	gi_GroupID;
};

/* Error codes for the security server */

#define	ACCERROR_NORESOURCES	100
#define ACCERROR_NOPRIVS	101
#define ACCERROR_NOAUTHORITY	102
#define ACCERROR_UNKNOWNUSER	103
#define ACCERROR_UNKNOWNGROUP	104
#define ACCERROR_LASTUSER	105
#define ACCERROR_LASTGROUP	106
#define ACCERROR_LASTMEMBER	107
#define ACCERROR_GROUPEXISTS	108
#define ACCERROR_NOFREEGROUPS	109
#define ACCERROR_UNKNOWNMEMBER	110

/* Currently Defined Flags for the User Flags data */

#define UFLAGB_AdminName	0	/* Whether or not this User may change his/her own name */
#define UFLAGB_AdminPassword	1	/* Whether or not this User may change his/her own password */
#define UFLAGB_NeedsPassword	2	/* Whether or not a password is required for this user */
#define	UFLAGB_AdminGroups	3	/* Whether or not this User may create/delete groups */
#define UFLAGB_AdminAll		4	/* User may do anything he wants. */

#define UFLAGF_AdminName	(1 << UFLAGB_AdminName)
#define UFLAGF_AdminPassword	(1 << UFLAGB_AdminPassword)
#define UFLAGF_NeedsPassword	(1 << UFLAGB_NeedsPassword)
#define UFLAGF_AdminGroups	(1 << UFLAGB_AdminGroups)
#define UFLAGF_AdminAll		(1 << UFLAGB_AdminAll)

#endif	/* ENVOY_ACCOUNTS_H */
