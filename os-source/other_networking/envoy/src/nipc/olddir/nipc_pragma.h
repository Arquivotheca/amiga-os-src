#pragma libcall NIPCBase BeginTransaction 1E A9803
#pragma libcall NIPCBase DoTransaction 24 A9803
#pragma libcall NIPCBase AbortTransaction 2A 901
#pragma libcall NIPCBase WaitTransaction 30 901
#pragma libcall NIPCBase CheckTransaction 36 901
#pragma libcall NIPCBase AllocTransactionA 3C 801
#pragma libcall NIPCBase CreateEntityA 42 801
#pragma libcall NIPCBase DeleteEntity 48 801
#pragma libcall NIPCBase WaitEntity 4E 801
#pragma libcall NIPCBase FindEntity 54 BA9804
#pragma libcall NIPCBase LoseEntity 5A 801
#pragma libcall NIPCBase GetTransaction 60 801
#pragma libcall NIPCBase ReplyTransaction 66 901
#pragma libcall NIPCBase WaitTransaction 6C 901
#pragma libcall NIPCBase FreeTransaction 72 901
#pragma libcall NIPCBase GetEntityName 78 09803
#pragma libcall NIPCBase StartStats 7E 801
#pragma libcall NIPCBase EndStats 84 0
#pragma libcall NIPCBase AddRoute 8A 4321005
#pragma libcall NIPCBase DeleteRoute 90 001
#pragma libcall NIPCBase GetRouteInfo 96 0