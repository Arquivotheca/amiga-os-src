/*
helloworld.c - Special version showing catalog use with ot without locale

This version of HelloWorld.c shows how to use a couple of link modules
(catalog.o and getcatalogstr.asm) to load your localized string catalog
on systems running V37 (i.e. without V38 locale.library).
The example will run under 1.3, but will use its internal strings
only.  If you felt like modifying catalog.c by providing replacements
for all of the 2.0 functions it uses, and adding a scheme for finding
the PROGDIR where the catalogs can be found under 1.3, you could even
make the catalog loading work under 1.3 + iffparse.library.  But you'll
have to do that yourself.

When using these modules, use XOpenCatalog and XCloseCatalog
when locale.library is not available, and ALWAYS use XGetString
to get a string.  XGetString() will uses XGetCatalogStr() in
getcatalogstr.asm which will use the locale.library GetCatalogStr()
if locale has been opened, else it will use its own string-getting code.
If you prefer to interface directly with XGetCatalogStr(), you
can do that also - it has a stack-based interface matching that
of the locale.library GetCatalogStr().

Note - the private variables of the XCatalog structure built by the
link modules is purposely different from the real private Catalog
structure variables in locale-allocated Catalogs.


The one catch when loading your own catalogs:

On a <V38 system, there are no Locale Prefs, and therefore no system
information about what language the user prefers.

When locale.library is available, you can use locale's OpenCatalog()
with no specified language preference tag since locale.library will
search for your catalog in the user's preferred order of languages.

When locale.library is not available, then you must use XOpenCatalog()
with an explicit OC_Language tag since that is the only way the module
can know what language to load.  This means your application will
need to know what language the user wants.  You could do this having
an icon tooltype specifying the language, or a command line arg, or
some special file you set up when the user initially uses or installs
your application.  In this simple example, we have just defined a string
UBYTE *language which is the language we will open on pre-V38 systems.
(The example only comes with that particular sample language catalog)

Note - do not ever attempt to locate/parse Locale.prefs yourself.
The prefs file name and contents are subject to change.
Use locale.library when available as this code does.


Since a CatComp object module contains code which directly calls
locale.library, we will instead use the CATCOMP_ARRAY in the .h file
generated by CatComp instead of the object file option.  Any other
application source modules that access strings should insetad
#define CATCOMP_NUMBERS when including <appname>_strings.h. The
catalog.c module will externally reference the catcomp string array
which is included here.


Sample use of CatComp for built-in strings:
CatComp helloworld.cd cfile helloworld_strings.h
LC -b1 -cfistq -v helloworld.c
Blink LIB:c.o helloworld.o catalog.o getcatalogstr.o TO HelloWorld LIB LIB:lc.lib LIB:amiga.lib SC SD
Quit

To create catalog files (after creating properly named directories for
translations and catalogs (with correct accented characters in directory
names, etc):

catcomp helloworld.cd CTFILE=translations/deutsch/helloworld.ct
(edit the .ct file to add the translated strings)
catcomp helloworld.cd translation=translations/deutsch/helloworld.ct (continued)
		catalog=catalogs/deutsch/helloworld.catalog
*/

#include <exec/types.h>
#include <libraries/locale.h>
#include <utility/tagitem.h>
#include <stdio.h>
#include <dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/locale_protos.h>

extern struct Library *SysBase;
extern struct Library *DOSBase;

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/locale_pragmas.h>

#include "catalog.h"

/* Note - if you have other applications source files that access
 * strings, in those modules you would include this CatComp-generated
 * header defining CATCOMP_NUMBERS instead since the actual strings
 * (i.e. the ARRAY) are only needed in one module, and they will
 * be externally referenced by catalog.c
 */
#define CATCOMP_ARRAY
#include "helloworld_strings.h"


/* referenced by catalog.c */
struct Library *LocaleBase  = NULL;
struct Library *UtilityBase = NULL;
struct Catalog *catalog = NULL;
struct CatCompArrayType *StringArray = &CatCompArray[0];
ULONG  StringCount = sizeof(CatCompArray) / sizeof(CatCompArray[0]);

UBYTE  *builtin  = "english";
UBYTE  *language = "deutsch";


VOID main(int argc, char **argv)
    {

    /* This is required by catalog.c, as supplied, to load a catalog */
    UtilityBase = OpenLibrary("utility.library",36);

    /* If available, we'll use locale.library */
    LocaleBase  = OpenLibrary("locale.library",38);

    if(LocaleBase)
        {
	/* Using user's preferred language under 2.1 and higher */
        catalog    = OpenCatalog(NULL,	"helloworld.catalog",
					OC_BuiltInLanguage, builtin,
					TAG_DONE);
	}
    else
	{
        catalog    = XOpenCatalog(NULL,"helloworld.catalog",
					OC_Language, language,
					OC_BuiltInLanguage, builtin,
					TAG_DONE);

	}


    printf("%s\n",XGetString(MSG_HELLO));
    printf("%s\n",XGetString(MSG_BYE));


    if(LocaleBase)
	{
	if(catalog)	CloseCatalog(catalog);
	CloseLibrary(LocaleBase);
	}
    else
	{
	if(catalog)	XCloseCatalog(catalog);
	}

    if(UtilityBase)	CloseLibrary(UtilityBase);
    }
