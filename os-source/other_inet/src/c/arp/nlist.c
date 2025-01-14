/*
** Simple nlist of inet.library
*/

#include <exec/types.h>
#include <exec/execbase.h>
#include <pragmas/exec_pragmas.h>
#include <clib/exec_protos.h>
#include <netinet/inet.h>
#include <nlist.h>

extern struct ExecBase *SysBase;
static char *inet = INETNAME;

int
nlist(kernel, nlp)
	char	*kernel;
	register struct	nlist *nlp;
{
	register struct InetNode *ip;
	register struct nlist *kl;
	register i;

	Forbid();
	ip = (struct InetNode *)FindName(&SysBase->LibList, inet);
	if(!ip){
		Permit();
		return -1;
	}

	for(; nlp->n_name && nlp->n_name[0]; nlp++){
		nlp->n_value = 0l;
		kl = ip->names;
		for(i = 0; i < ip->nlsize; i++, kl++){
			if(kl->n_name && (strcmp(nlp->n_name, kl->n_name) == 0)){
				nlp->n_value = kl->n_value;
				nlp->n_type = 1;
				break;
			}
		}
	}
	Permit();

	return 0;
}
