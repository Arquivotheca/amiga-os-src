/*
**  $Id: test.c,v 1.3 92/04/09 11:58:30 dlarson Exp Locker: dlarson $
**
**  Authentication.library test program.
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#include <exec/exec.h>
#include <envoy/authentication.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/authentication_protos.h>
#include <pragmas/authentication_pragmas.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern struct Library *SysBase;
struct Library *AuthenticationBase;

void printuser(struct UserProfile *user);
void report(char *string, ULONG should, ULONG code);
void main(void);

void main(void)
{
struct UserProfile *tp, *ap, *bp;
struct Group *g;
ULONG err;

	AuthenticationBase = OpenLibrary("authentication.library", 0L);
	if(!AuthenticationBase)
	{
		printf("Can't open authentication.library!!!\n");
		exit(-1);
	}
	err = ChooseAuthServer(NULL);
	report("Choose server", 0, err);
	if(err)
	{
		goto END;
	}
	tp = AuthenticateUser("admin", "hosehead");
	if(tp)
		printf("AuthenticateUser() with wrong password = %lx (should be 0).\n",tp);
	tp = AuthenticateUser("admin", "changeme");
	if(!tp)
	{
		goto END;
	}
/*  Now we are going to re-use tp for something else... */
	tp->up_UID = 1;
	err = FindUserName(tp);
//	printuser(tp);
	report("FindUserName(1)", 0, err);
	tp->up_UID = 65535;
	err = FindUserName(tp);
//	printuser(tp);
	report("FindUserName(65535)", 203, err);
/*  Now we are going to re-use tp for something else... */
	strcpy(tp->up_UserName, "joe");
	strcpy(tp->up_PassWord, "none");
	strcpy(tp->up_FullName, "Joe Studd");
	if( !(ap = AllocUserProfile()) )
	{
		printf("Can't AllocUserProfile(ap).\n");
		FreeUserProfile(tp);
		goto ERROR1;
	}
	strcpy(ap->up_UserName, "admin");
	strcpy(ap->up_PassWord, "changeme");
/* Try various combinations of AddUser(), only the last should succeed */
	err = AddUser(ap, ap);
	report("AddUser(admin, admin)", 204, err);
	err = AddUser(ap, tp);
	report("AddUser(admin, joe)", 202, err);
	err = AddUser(tp, tp);
	report("AddUser(joe, joe)", 202, err);
	err = AddUser(tp, ap);
	report("AddUser(joe, admin)", 0, err);
/*  We've added joe to the user database, let's authenticate him... */
	bp = AuthenticateUser("joe", "none");
//	printuser(bp);
	if(!bp)
	{
		printf("failed to authenticate joe (1).\n");
		goto JUMP;
	}
/*  Let's see which groups he is in... */
	if( !(g = AllocGroup()) )
	{
		printf("Can't AllocGroup().\n");
		FreeUserProfile(tp);
		goto END;
	}
	strcpy(g->g_GroupName, "Administrators");
	if(MemberOf(g, bp))
	{
		printf("MemberOf(g, bp) succeeded, but should've failed.\n");
	}
	if(!MemberOf(g, ap))
	{
		printf("MemberOf(g, ap) failed like it shouldn't have.\n");
	}
/*  Let's change joe's password... */
	strcpy(bp->up_PassWord, "some");
	err = ChangePassWord(tp, bp);
	report("\nChangePassWord(old,new)", 202,err);
	err = ChangePassWord(bp, tp);
	report("ChangePassWord(new,old)", 0, err);
/*  We've changed joe's password, let's authenticate him again... */
	bp = AuthenticateUser("joe", "none");
//	printuser(bp);
	if(bp)
	{
		printf("attempt to authenticate joe failed (2).\n");
		FreeUserProfile(bp);
	}
	bp = AuthenticateUser("joe", "some");
//	printuser(bp);
	if(bp)
	{
		FreeUserProfile(bp);
	}else
	{
		printf("Authenticated joe inappropriately (3).\n");
	}
/*  Now let's change it back */
	err = ChangePassWord(tp, ap);
	report("ChangePassWord(old,admin)", 0, err);
/*  We've changed joe's password, let's authenticate him again... */
	bp = AuthenticateUser("joe", "none");
//	printuser(bp);
	if(!bp)
	{
		printf("failed to authenticate joe (4).\n");
		goto JUMP;
	}
/*  Let's change a non-existant person's password  */
	strcpy(bp->up_UserName, "elvis");
	err = ChangePassWord(bp, ap);
	report("ChangePassWord(elvis,admin)", 203, err);
/*  Let's add a group:  */
	strcpy(g->g_GroupName, "CoolDudes");
	err = AddGroup(g, bp);
	report("AddGroup(CoolDudes,elvis)", 202, err);
	err = AddGroup(g, tp);
	report("AddGroup(CoolDudes,joe)", 0, err);
/*  Try to add elvis.  Try to add joe.  Check memberof joe, memberof admin, */
	if(!MemberOf(g, tp))
	{
		printf("MemberOf(CoolDudes, joe) failed like it shouldn't have.\n");
	}
	if(MemberOf(g, ap))
	{
		printf("MemberOf(CoolDudes, admin) succeeded, as it shouldn't have.\n");
	}
	err = AddToGroup(g, bp, tp);
	report("AddToGroup(CoolDudes, elvis, joe)", 203, err);
	err = AddToGroup(g, ap, tp);
	report("AddToGroup(CoolDudes, admin, joe)", 0, err);
	if(!MemberOf(g, ap))
	{
		printf("MemberOf(CoolDudes, admin) failed like it shouldn't have.\n");
	}
	if(!MemberOf(g, tp))
	{
		printf("MemberOf(CoolDudes, joe) failed like it shouldn't have.\n");
	}
	err = RemoveFromGroup(g, ap, ap);
	report("RemoveFromGroup(CoolDudes, admin, admin)", 0, err);
	if(!MemberOf(g, tp))
	{
		printf("MemberOf(CoolDudes, joe) failed like it shouldn't have.\n");
	}
	if(MemberOf(g, ap))
	{
		printf("MemberOf(CoolDudes, admin) succeeded, as it shouldn't have.\n");
	}
/*  Delete group: */
	err = DeleteGroup(g, bp);
	report("DeleteGroup(CoolDudes, elvis)", 202, err);
	err = DeleteGroup(g, tp);
	report("DeleteGroup(CoolDudes, joe)", 0, err);
	err = DeleteGroup(g, ap);
	report("DeleteGroup(CoolDudes, admin)", 203, err);
/*  Bye, Elvis, group:  */
	FreeUserProfile(bp);
	FreeGroup(g);
/*  Let's get rid of joe... */
JUMP:	err = DeleteUser(tp, tp);
	report("DeleteUser(joe, joe)", 202, err);
	err = DeleteUser(tp, ap);
	report("DeleteUser(joe, admin)", 0, err);
	err = DeleteUser(tp, ap);
	report("DeleteUser(joe, admin)", 203, err);
	bp = AuthenticateUser("joe", "none");
//	printuser(bp);
	if(bp)
	{
		printf("authenticated joe but shouldn't have been able to!\n\n");
		FreeUserProfile(bp);
	}
/*  Clean up and go home...*/
	FreeUserProfile(ap);
ERROR1:
	FreeUserProfile(tp);
END:
	CloseLibrary(AuthenticationBase);
	printf("All tests completed.\n");
}


void printuser(struct UserProfile *user)
{
	printf("---  UserProfile %lx ---\n", user);
	if(!user)
	{
		return;
	}
	printf("UserName: %s\n", user->up_UserName);
	printf("FullName: %s\n", user->up_FullName);
	printf("PassWord: %s\n", user->up_PassWord);
	printf("UID:      %d\n", user->up_UID);
	printf("GID:      %d\n", user->up_GID);
}


void report(char *string, ULONG should, ULONG code)
{
char buffer[80];

	if(should == code)
	{
		return;
	}
	printf("%s, err=%ld (should be %lu).\n",string, code, should);
	if(AuthErrorText(code, buffer, 80))
	{
		printf("\tError Text: '%s'\n", buffer);
	}else
	{
		printf("\tCan't get error text.\n");
	}
}