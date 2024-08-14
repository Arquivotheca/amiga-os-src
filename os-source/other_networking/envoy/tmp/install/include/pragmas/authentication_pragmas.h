/* "authentication.library" */
/*--- functions in V37 or higher (distributed as Release 2.04) ---*/
#pragma libcall AuthenticationBase AddGroup 1e 9802
#pragma libcall AuthenticationBase AddToGroup 24 A9803
#pragma libcall AuthenticationBase AddUser 2a 9802
#pragma libcall AuthenticationBase AllocUserProfile 30 00
#pragma libcall AuthenticationBase AuthenticateUser 36 9802
#pragma libcall AuthenticationBase AllocGroup 3c 00
#pragma libcall AuthenticationBase FreeGroup 42 801
#pragma libcall AuthenticationBase ChangePassWord 48 9802
#pragma libcall AuthenticationBase ChooseAuthServer 4e 801
#pragma libcall AuthenticationBase DeleteGroup 54 9802
#pragma libcall AuthenticationBase DeleteUser 5a 9802
#pragma libcall AuthenticationBase AuthErrorText 60 18003
#pragma libcall AuthenticationBase FindGroupID 66 801
#pragma libcall AuthenticationBase FindGroupName 6c 801
#pragma libcall AuthenticationBase FindUserName 72 801
#pragma libcall AuthenticationBase FreeUserProfile 78 801
#pragma libcall AuthenticationBase ListGroups 7e 801
#pragma libcall AuthenticationBase ListMembers 84 9802
#pragma libcall AuthenticationBase ListUsers 8a 801
#pragma libcall AuthenticationBase RemoveFromGroup 90 A9803
#pragma libcall AuthenticationBase MemberOf 96 9802
#pragma libcall AuthenticationBase NoSecurity 9c 001
