#ifndef	ACCOUNTS_H
#define ACCOUNTS_H

struct UserInfo
{
	UBYTE	ui_UserName[32];
	UWORD	ui_UserID;
	UWORD	ui_PrimaryGroupIP;
};

struct GroupInfo
{
	UBYTE	gi_GroupName[32];
	UWORD	gi_GroupID;
};

#endif	/* ACCOUNTS_H */