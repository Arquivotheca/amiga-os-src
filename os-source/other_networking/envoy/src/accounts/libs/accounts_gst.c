/*
** $Id: accounts_gst.c,v 1.1 92/10/13 11:20:10 kcd Exp $
**
** Module that contains routines to verify a user's username and
** password and to verify a user's group membership.
**
*/

/*--------------------------------------------------------------------------*/

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef ENVOY_NIPC_H
#include <envoy/nipc.h>
#endif

#ifndef ENVOY_ERRORS_H
#include <envoy/errors.h>
#endif

#include "accountsbase.h"
#include "/accounts.h"
#include "/transactions.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/nipc_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/nipc_pragmas.h>

