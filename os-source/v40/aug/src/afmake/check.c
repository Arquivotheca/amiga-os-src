/*
 *	Check structures for make.
 */

#include <stdio.h>
#include "h.h"
#ifdef LATTICE
#include <string.h>
#endif

/* note: stderr in this file changed to stdout for amiga use */

/*
 *	Prints out the structures as defined in memory.  Good for check
 *	that you make file does what you want (and for debugging make).
 */
void
prt()
{
    register struct name *np;
    register struct depend *dp;
    register struct line *lp;
    register struct cmd *cp;
    register struct macro *mp;


    for (mp = macrohead; mp; mp = mp->m_next)
	fprintf(stdout, "%s = %s\n", mp->m_name, mp->m_val);

    fputc('\n', stdout);

    for (np = namehead.n_next; np; np = np->n_next) {
	if (np->n_flag & N_DOUBLE)
	    fprintf(stdout, "%s::\n", np->n_name);
	else fprintf(stdout, "%s:\n", np->n_name);
	if (np == firstname) fprintf(stdout, "(MAIN NAME)\n");

	for (lp = np->n_line; lp; lp = lp->l_next) {
	    fputc(':', stdout);
	    for (dp = lp->l_dep; dp; dp = dp->d_next)
		fprintf(stdout, " %s", dp->d_name->n_name);
	    fputc('\n', stdout);

	    for (cp = lp->l_cmd; cp; cp = cp->c_next)
#ifdef os9
		fprintf(stdout, "-   %s\n", cp->c_cmd);
#else
		fprintf(stdout, "-\t%s\n", cp->c_cmd);
#endif
	    fputc('\n', stdout);
	}
	fputc('\n', stdout);
    }
}


/*
 *	Recursive routine that does the actual checking.
 */
void
check(np)
    struct name    *np;
{
    register struct depend *dp;
    register struct line *lp;


    if (np->n_flag & N_MARK)
	fatal("Circular dependency from %s", np->n_name);

    np->n_flag |= N_MARK;

    for (lp = np->n_line; lp; lp = lp->l_next)
	for (dp = lp->l_dep; dp; dp = dp->d_next)
	    check(dp->d_name);

    np->n_flag &= ~N_MARK;
}


/*
 *	Look for circular dependancies.
 *	ie.
 *		a: b
 *		b: a
 *	is a circular dep
 */
void
circh()
{
    register struct name *np;


    for (np = namehead.n_next; np; np = np->n_next)
	check(np);
}


/*
 *	Check the target .PRECIOUS, and mark its dependentd as precious
 */
void
precious()
{
    register struct depend *dp;
    register struct line *lp;
    register struct name *np;


    if (!((np = newname(".PRECIOUS"))->n_flag & N_TARG))
	return;

    for (lp = np->n_line; lp; lp = lp->l_next)
	for (dp = lp->l_dep; dp; dp = dp->d_next)
	    dp->d_name->n_flag |= N_PREC;
}
