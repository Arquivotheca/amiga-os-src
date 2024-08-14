struct RigidDiskBlock DummyRDB =
{
	IDNAME_RIGIDDISK,
	sizeof(struct RigidDiskBlock) >> 2,0,7,
	512,
	RDBFF_LAST | RDBFF_LASTLUN | RDBFF_LASTTID |
	RDBFF_NORESELECT | RDBFF_DISKID,
	NULL,NULL,NULL,NULL,
	{0,0,0,0,0,0},
	612,17,4,	/***************************/
	1,612,		/* park ??? */
	{0L,0L,0L},
	612,612,3,	/*steprate ? */
	{0,0,0,0,0},
	0,(4*17*2)-1,2,611,
	4*17,0,		/* seconds? */
	0,0,
	"Seagate",	/* FIX! */
	"ST-238",	/* FIX! */
	"1.1",		/* FIX! */
	"",
	"",
	"",
	{0,0,0,0,0,0,0,0,0,0},
};

struct Drive DummyDrive =
{
	NULL,
	&DummyRDB,NULL,NULL,
	ST506|WRITTEN,
	0,0,0,
	"xt.device",
	0,
};
