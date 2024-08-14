

#include <exec/types.h>
#include "texttable.h"


STRPTR AppStrings[] =
{
    "",
    "ERROR",
    "ERROR: string line for token '%s' not found\n",
    "ERROR: token not found\n       file '%s' line %lu\n",
    "ERROR: '(' expected\n       file '%s' line %lu column %lu\n",
    "ERROR: ')' expected\n       file '%s' line %lu column %lu\n",
    "ERROR: '/' expected\n       file '%s' line %lu column %lu\n",
    "ERROR: garbage characters after token '%s'\n       file '%s' line %lu column %lu\n",
    "ERROR: '%s' is not a valid token\n       file '%s' line %lu\n",
    "ERROR: token '%s' not found\n       file '%s'\n",
    "ERROR: string too short for token '%s'\n       file '%s' line %lu\n",
    "ERROR: string too long for token '%s'\n       file '%s' line %lu\n",
    "ERROR: negative value for minimum length\n       file '%s' line %lu column %lu\n",
    "ERROR: negative value for maximum length\n       file '%s' line %lu column %lu\n",
    "ERROR: non-positive value for %% ordering\n       file '%s' line %lu column %lu\n",
    "ERROR: missing numerical value for %% ordering\n       file '%s' line %lu column %lu\n",
    "ERROR: %% ordering value too large\n       file '%s' line %lu column %lu\n",
    "ERROR: %% size incorrect\n       file '%s' line %lu column %lu\n",
    "ERROR: %% command does not match\n       file '%s' line %lu column %lu\n",
    "ERROR: token '%s' defined multiple times\n       file '%s' line %lu\n",
    "ERROR: id '%ld' already used for token '%s'\n       file '%s' line %lu\n",
    "ERROR: no command found after '#'\n       file '%s' line %lu\n",
    "ERROR: '%s' is not a valid command after '#'\n       file '%s' line %lu\n",
    "ERROR: '%s' is not a valid codeset value\n       file '%s' line %lu\n",
    "ERROR: '%s' is not a valid rcsid value\n       file '%s' line %lu\n",
    "ERROR: '%s' is not a valid lengthbytes value\n       file '%s' line %lu\n",
    "ERROR: couldn't write catalog '%s'\n",
    "ERROR: couldn't write translation file '%s'\n",
    "ERROR: couldn't write C source file '%s'\n",
    "ERROR: couldn't write ASM source file '%s'\n",
    "ERROR: couldn't write M2 source file '%s'.def or '%s'.mod\n",
    "ERROR: couldn't write binary object file '%s'\n",

    "WARNING: '%lc' is an unknown formatting command\n         file '%s' line %lu column %lu\n",
    "WARNING: string for token '%s' matches string in descriptor\n         file '%s' line %lu\n",
    "WARNING: original string for token '%s' had a trailing ellipsis (...)\n         file '%s' line %lu\n",

    "## version $VER: XX.catalog XX.XX (XX.XX.XX)\n## codeset X\n## language X\n;\n",
    "\n/* This file was created automatically by CatComp.\n * Do NOT edit by hand!\n */\n\n\n#ifndef EXEC_TYPES_H\n#include <exec/types.h>\n#endif\n\n#ifdef CATCOMP_ARRAY\n#undef CATCOMP_NUMBERS\n#undef CATCOMP_STRINGS\n#define CATCOMP_NUMBERS\n#define CATCOMP_STRINGS\n#endif\n\n#ifdef CATCOMP_BLOCK\n#undef CATCOMP_STRINGS\n#define CATCOMP_STRINGS\n#endif\n\n\n/****************************************************************************/\n\n\n",

    "\n* This file was created automatically by CatComp.\n* Do NOT edit by hand!\n*\n\n\n\tIFND EXEC_TYPES_I\n\tINCLUDE \'exec/types.i\'\n\tENDC\n\n\tIFD CATCOMP_ARRAY\nCATCOMP_NUMBERS SET 1\nCATCOMP_STRINGS SET 1\n\tENDC\n\n\tIFD CATCOMP_CODE\nCATCOMP_BLOCK SET 1\n\tENDC\n\n\n;-----------------------------------------------------------------------------\n\n\n",

    "\n(* This file was created automatically by CatComp.\n * Do NOT edit by hand!\n *)\n\n",

    "'%s' is a valid descriptor file\n",
    "'%s' is a valid translation file\n"
};


STRPTR GetString(enum AppStringsID id)
{
    return(AppStrings[id]);
}
