
#ifndef LHEXTRACT

# define	OPT_ARCHIVE	0 	/* ALL operations, but HELP */
# define	OPT_FILES	1 	/* ALL operations, but HELP */
# define	OPT_ADD		2       /* ADD files to archive */
# define	OPT_EXTRACT	3       /* EXTRACT files from archive */
# define	OPT_LIST	4       /* LIST files in archive */
# define	OPT_UPDATE	5       /* UPDATE (replace) file in archive */
# define	OPT_REMOVE	6       /* REMOVE (delete) file from archive */
# define	OPT_CREATE	7       /* remove archive and create anew */
# define	OPT_PRINT	8       /* PRINT files to stdout */
# define	OPT_TEST	9       /* TEST archive integrity */
# define	OPT_HELP	10      /* Explanatory HELP text */
# define	OPT_QUIET	11      /* All operations, but HELP */
# define	OPT_VERBOSE	12	/* All operations, but HELP */
# define	OPT_NOEXEC	13	/* All operations, but HELP */
# define	OPT_FORCE	14	/* EXTRACT */
# define	OPT_TEXT	15	/* ADD, UPDATE, CREATE */
# define	OPT_OLD		16	/* ADD, UPDATE, CREATE */
# define	OPT_DELETE	17	/* ADD, UPDATE, CREATE */
# define	OPT_IGNOREPATH	18	/* EXTRACT */
# define	OPT_NOCOMPRESS	19	/* ADD, UPDATE, CREATE */
# define	OPT_DIRECTORY	20	/* EXTRACT */
# define	OPT_HEADER	21	/* ADD, UPDATE, CREATE */
#if	defined(IDEBUG)
# define	OPT_DEBUG	22
# define	OPT_COUNT	23
#else
# define	OPT_COUNT	22
#endif

#else

# define	OPT_ARCHIVE	0 	/* ALL operations, but HELP */
# define	OPT_FILES	1 	/* ALL operations, but HELP */
# define	OPT_EXTRACT	2       /* EXTRACT files from archive */
# define	OPT_LIST	3       /* LIST files in archive */
# define	OPT_PRINT	4       /* PRINT files to stdout */
# define	OPT_TEST	5       /* TEST archive integrity */
# define	OPT_HELP	6       /* Explanatory HELP text */
# define	OPT_QUIET	7       /* All operations, but HELP */
# define	OPT_VERBOSE	8	/* All operations, but HELP */
# define	OPT_NOEXEC	9	/* All operations, but HELP */
# define	OPT_FORCE	10	/* EXTRACT */
# define	OPT_IGNOREPATH	11	/* EXTRACT */
# define	OPT_DIRECTORY	12	/* EXTRACT */
#if	defined(IDEBUG)
# define	OPT_DEBUG	13
# define	OPT_COUNT	14
#else
# define	OPT_COUNT	13
#endif

#endif
