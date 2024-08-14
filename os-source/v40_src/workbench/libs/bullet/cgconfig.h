/* cgconfig.h */
/*  Copyright (C)  1989-90 by Agfa Compugraphic, Inc. All rights reserved. */

/*   26-Jun-90  bjg Added BITMAP, OUTLINE, LINEAR and QUADRA.              */
/*   21-Sep-90  jfd Changed BITMAP to CGBITMAP due to conflict in          */
/*                  "windows.h"                                            */
/*   22-Oct-90  jfd Disabled OUTLINE and QUADRA as default settings        */
/*   05-Dec-90  bg  Instead of #defining OUTLINE separately,               */
/*                  #define OUTLINE QUADRA                                 */

/*  These defines control whether various sections of code are compiled.   */

#define CACHE        0
#define SCREEN_FONTS 0
#define PCLEO        0
#define SEGACCESS    0
#define COMPRESS     0
#define CGBITMAP     1   /*  At least one of {CGBITMAP, OUTLINE} must be true */
#define QUADRA       0
#define OUTLINE      QUADRA
