/*
** Add elem after pred
*/

#include "proto.h"

struct qelem {
	struct qelem *q_forw;
	struct qelem *q_back;
};

void insque(elem, pred)
	register struct qelem *elem, *pred;
{
	elem->q_forw = pred->q_forw;
	pred->q_forw = elem;

	elem->q_forw->q_back = elem;
	elem->q_back = pred;
}

/*
** Remove from list
*/

void remque(elem)
	register struct qelem *elem;
{
	elem->q_back->q_forw = elem->q_forw;
	elem->q_forw->q_back = elem->q_back;
}

