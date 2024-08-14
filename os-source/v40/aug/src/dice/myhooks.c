/* myhooks.c -- set up layers vector stealing	*/

typedef	LONG	(*PFL)();

struct	hook	{
	LONG	hk_SysFunc;
	PFL		hk_MyFunc;
	PFL		hk_Entry;	/* unique entry point	*/
	WORD	hk_Test;
	LONG	hk_LVO;
};

void	drawShadowSignal();

/* unique entry points	*/
long	entry0();
long	entry1();
long	entry2();
long	entry3();
long	entry4();
long	entry5();
long	entry6();
long	entry7();

struct hook	myhooks[] = {
	{	0,	
		(PFL) drawShadowSignal,
		entry0,
		0,
		-48,		/* upfront			*/
	},
	{	0,	
		(PFL) drawShadowSignal,
		entry1,
		0,
		-42			/* create behind	*/
	},
	{	0,	
		(PFL) drawShadowSignal,
		entry2,
		0,
		-36			/* create upfront	*/
	},
	{	0,	
		(PFL) drawShadowSignal,
		entry3,
		0,
		-90			/* delete			*/
	},
	{	0,	
		(PFL) drawShadowSignal,
		entry4,
		0,
		-54			/* behind			*/
	},
	{	0,	
		(PFL) drawShadowSignal,
		entry5,
		0,
		-66			/* size 			*/
	},
	{	0,	
		(PFL) drawShadowSignal,
		entry6,
		0,
		-60			/* move 			*/
	},
	{	0,	
		(PFL) drawShadowSignal,
		entry7,
		0,
		-180			/* movesize 			*/
	}
};

#define NUMHOOKS	(sizeof (myhooks)/ sizeof (struct hook))

extern	struct Library	*LayersBase;

numHooks()
{
	if ( LayersBase->lib_Version < 36 ) return ( NUMHOOKS - 1 );
	return ( NUMHOOKS );
}

setup_hooks()
{
	int		i;
	int		numhooks;

	/* install the hooks	*/
	Forbid();

	numhooks = numHooks();

	for (i = 0; i < numhooks; ++i)
	{
		myhooks[i].hk_SysFunc =
			SetFunction(LayersBase, myhooks[i].hk_LVO, myhooks[i].hk_Entry);
	}
	Permit();
}

cleanup_hooks()
{
	int	i;
	int numhooks;

	/* remove the hooks		*/
	/*** ZZZZ need to see if vectors are still mine	***/
	numhooks = numHooks();

	for (i = 0; i < numhooks; ++i)
	{
		SetFunction(LayersBase, myhooks[i].hk_LVO, myhooks[i].hk_SysFunc);
	}

	return (1);		/* tell him it's ok to exit	*/
}
