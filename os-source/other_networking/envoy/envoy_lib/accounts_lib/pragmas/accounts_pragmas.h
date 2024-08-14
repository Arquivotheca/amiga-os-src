/* "accounts.library" */
/*--- functions in V37 or higher (Release 2.04) ---*/
/* User and Group structure Allocation/Deallocation */
#pragma libcall AccountsBase AllocUserInfo 1e 00
#pragma libcall AccountsBase AllocGroupInfo 24 00
#pragma libcall AccountsBase FreeUserInfo 2a 801
#pragma libcall AccountsBase FreeGroupInfo 30 801
/* User Verification functions * */
#pragma libcall AccountsBase VerifyUser 36 A9803
#pragma libcall AccountsBase MemberOf 3c 9802
/* Functions to find users/groups or build lists of users/groups/members * */
#pragma libcall AccountsBase NameToUser 42 9802
#pragma libcall AccountsBase NameToGroup 48 9802
#pragma libcall AccountsBase IDToUser 4e 8002
#pragma libcall AccountsBase IDToGroup 54 8002
#pragma libcall AccountsBase NextUser 5a 801
#pragma libcall AccountsBase NextGroup 60 801
#pragma libcall AccountsBase NextMember 66 9802
