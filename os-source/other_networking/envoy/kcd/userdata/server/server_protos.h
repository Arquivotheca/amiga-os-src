/* Prototypes for functions defined in server.c */
void __saveds server(void);
void ReadUserData(ServerDataPtr sd);
void WriteUserData(ServerDataPtr sd);
void ReadGroupData(ServerDataPtr sd);
void WriteGroupData(ServerDataPtr sd);
void FlushUserData(ServerDataPtr sd,
                   BOOL flush);
struct Entity *CreateEnt(ServerDataPtr sd,
                         Tag firsttag, ...);
void ProcessRequests(ServerDataPtr sd);
void AddUser(ServerDataPtr sd,
             struct Transaction *trans);
BOOL InternalAddUser(ServerDataPtr sd,
                     UserData *user);
void AddUserSorted(ServerDataPtr sd,
                   struct MinList *list,
                   UserNode *user);
void RemUser(ServerDataPtr sd,
             struct Transaction *trans);
BOOL InternalRemUser(ServerDataPtr sd,
                     UserData *user);
void InternalRemUserFromGroup(ServerDataPtr sd,
                              GroupNode *group,
                              UserNode *user);
UserNode *FindUserByName(ServerDataPtr sd,
                         STRPTR name);
UserNode *FindUserByUID(ServerDataPtr sd,
                        UWORD uid);
GroupNode *FindGroupByName(ServerDataPtr sd,
                           STRPTR name);
GroupNode *FindGroupByGID(ServerDataPtr sd,
                          UWORD gid);
void NextUser(ServerDataPtr sd,
              struct Transaction *trans);
void IDToUser(ServerDataPtr sd,
              struct Transaction *trans);
void NameToUser(ServerDataPtr sd,
                struct Transaction *trans);
void IDToGroup(ServerDataPtr sd,
               struct Transaction *trans);
void NameToGroup(ServerDataPtr sd,
                 struct Transaction *trans);
UserNode *VerifyAuthority(ServerDataPtr sd,
                          AuthorityData *ad);
UserNode *InternalVerifyUser(ServerDataPtr sd,
                             UserData *userdata);
void VerifyUser(ServerDataPtr sd,
                struct Transaction *trans);
void AdminUser(ServerDataPtr sd,
               struct Transaction *trans);
void ModifyGroup(ServerDataPtr sd,
                 struct Transaction *trans);
void AddGroup(ServerDataPtr sd,
              struct Transaction *trans);
void AddGroupSorted(ServerDataPtr sd,
                    struct MinList *list,
                    GroupNode *group);
void RemGroup(ServerDataPtr sd,
              struct Transaction *trans);
void NextGroup(ServerDataPtr sd,
               struct Transaction *trans);
void NextMember(ServerDataPtr sd,
                struct Transaction *trans);
MinUserNode *FindMember(ServerDataPtr sd,
                        GroupNode *group,
                        UserNode *user);
void MemberOf(ServerDataPtr sd,
              struct Transaction *trans);
void NewList(struct List *list);
void AddMember(ServerDataPtr sd,
               struct Transaction *trans);
void RemoveMember(ServerDataPtr sd,
                  struct Transaction *trans);
