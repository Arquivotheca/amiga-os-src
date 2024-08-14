#ifndef CATALOG_H
#define CATALOG_H


/*****************************************************************************/


#include <exec/types.h>
#include <libraries/locale.h>


/*****************************************************************************/


struct XCatalog
{
    struct Node    ec_Link;       /* for internal linkage         	  */
    UWORD          ec_Pad;        /* to longword align                    */
    STRPTR         ec_Language;   /* language of the catalog, may be NULL */
    ULONG          ec_CodeSet;    /* currently always 0                   */
    UWORD          ec_Version;    /* version of the catalog               */
    UWORD          ec_Revision;   /* revision of the catalog              */
    char           ec_LanguageStr[32];
    APTR           ec_Strings;
};

/*****************************************************************************/

/* tags on stack interface */
struct Catalog *XOpenCatalog(struct Locale *locale,
			      STRPTR name,
			      ULONG tags, ...);

/* tag list pointer interface */
struct Catalog *XOpenCatalogA(struct Locale *locale,
			       STRPTR name,
			       struct TagItem *tags);


VOID   XCloseCatalog(struct Catalog *catalog);

STRPTR XGetString( LONG id);

STRPTR XGetCatalogStr(struct Catalog *catalog,
		       LONG stringNum,
		       STRPTR defaultString);




/*****************************************************************************/


#endif /* CATALOG_H */

