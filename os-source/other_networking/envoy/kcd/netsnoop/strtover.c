#include <exec/types.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>

#include <stdlib.h>
#include <string.h>

#define STV_FAIL 0xFFFFFFFF

/* proto */
ULONG strToVer(STRPTR);

/* strToVer =======================================================
   Input: null-terminated string of form "39 106"
   Output: ULONG with version in upper word, revision in lower word.
           (version is first number, revision is second.)
   Special cases:
        - Failure: returns 0xffffffff
        - Only one number: assumes it's a version and revision == 0.
*/
ULONG strToVer(STRPTR verStr) {
    STRPTR v, v1=NULL, v2, newStr;
    int vlen;
    ULONG result=STV_FAIL;

    if (!verStr)
        return(result);

    vlen = strlen(verStr);
    if (!vlen)
        return(result);

    if (newStr = AllocVec(vlen+1UL, MEMF_CLEAR)) {
        strcpy(newStr, verStr);
        v = newStr;
        while (*v != '\0' && *v != ' ')
            v++;
        if (*v == '\0') {
            result = strtoul(newStr,&v1,0);
            if (result)
                result <<= 16;
        }
        else {
            *v++ = '\0';
            v1 = newStr;
            v2 = v;
            result = (strtoul(v1, &v, 0) << 16);
            result |= strtoul(v2, &v, 0);
        }
        FreeVec(newStr);
    }

    return(result);
}


