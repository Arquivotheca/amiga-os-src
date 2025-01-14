/* readargs.c
 *
 */

#include "wdisplay.h"

struct Args
{
    STRPTR a_Key;
    UBYTE a_Type;
};

#define	ARG_STRING	0
#define	ARG_NUMERIC	1
#define	ARG_SWITCH	2

struct Args args[] =
{
    {"TITLE",		ARG_STRING},
    {"PUBSCREEN",	ARG_STRING},
    {"LEFT",		ARG_NUMERIC},
    {"TOP",		ARG_NUMERIC},
    {"WIDTH",		ARG_NUMERIC},
    {"HEIGHT",		ARG_NUMERIC},
#if DOBACKDROP
    {"BACKDROP",	ARG_SWITCH},
#endif
    {"SCALE",		ARG_SWITCH},
    {"NOREMAP",		ARG_SWITCH},
    {"MENUITEM",	ARG_STRING},
    {NULL,},
};

VOID readargs (struct AppInfo * ai, int argc, char **argv)
{
    register UBYTE *tmp;
    register WORD i;
    register WORD j;
    BOOL found;

    if (argc == 0)
    {
	for (j = 0; args[j].a_Key; j++)
	{
	    if (tmp = FindToolType (argv, args[j].a_Key))
	    {
		switch (args[j].a_Type)
		{
		    case ARG_STRING:
			ai->ai_Options[(j + 1)] = (LONG) tmp;
			break;

		    case ARG_NUMERIC:
			ai->ai_Options[(j + 1)] = (LONG) atoi (tmp);
			break;

		    case ARG_SWITCH:
			ai->ai_Options[(j + 1)] = TRUE;
			break;
		}
	    }
	}
    }
    else
    {
	/* Step through the arguments */
	for (i = 1; ((i < argc) && !ai->ai_Done); i++)
	{
	    /* Asking for help? */
	    if (stricmp (argv[i], "?") == 0)
	    {
		CLIMsg (TEMPLATE);
		ai->ai_Done = TRUE;
	    }
	    else
	    {
		/* Clear the found flag */
		found = FALSE;

		/* Step through the possible arguments */
		for (j = 0; args[j].a_Key; j++)
		{
		    /* Does it match? */
		    if (stricmp (argv[i], args[j].a_Key) == 0)
		    {
			/* Show that we found it */
			found = TRUE;

			/* Make sure an argument follows */
			if ((args[j].a_Type != ARG_SWITCH) && (++i >= argc))
			{
			    NotifyUser (ai, ERR_REQUIRED_ARG_MISSING);
			    ai->ai_Done = TRUE;
			}
			else
			{
			    switch (args[j].a_Type)
			    {
				case ARG_STRING:
				    ai->ai_Options[(j + 1)] = (LONG) argv[i];
				    break;

				case ARG_NUMERIC:
				    ai->ai_Options[(j + 1)] = (LONG) atoi (argv[i]);
				    break;

				case ARG_SWITCH:
				    ai->ai_Options[(j + 1)] = TRUE;
				    break;
			    }
			}
			break;
		    }
		}

		if (!found)
		{
		    ai->ai_Options[0] = (LONG) argv[i];
		}
	    }
	}
    }
}
