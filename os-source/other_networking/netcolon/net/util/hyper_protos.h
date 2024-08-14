/* hyper_protos.h --- function prototypes for the hypertext library
 * Copyright (C) 1990 Commodore-Amiga, Inc.
 * Written by David N. Junod
 */

LONG LockHyperBase (HYPERCONTEXT);
VOID UnlockHyperBase (LONG);
VOID ExpungeDataBases (BOOL);
HYPERCONTEXT OpenHyper (struct NewHyper *, ULONG);
HYPERCONTEXT OpenHyperAsync (struct NewHyper *, ULONG);
VOID CloseHyper (HYPERCONTEXT);
ULONG HyperSignal (HYPERCONTEXT);
struct HyperMsg *GetHyperMsg (HYPERCONTEXT);
VOID ReplyHyperMsg (struct HyperMsg *);
LONG SetHyperContext (HYPERCONTEXT, ULONG, ULONG);
LONG SendHyperContext (HYPERCONTEXT, ULONG);
LONG SendHyperCmd (HYPERCONTEXT, STRPTR, ULONG);
