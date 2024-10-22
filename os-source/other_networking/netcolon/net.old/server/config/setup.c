#include "configopts.h"

void *gmem(global, size)
struct GLOBAL *global;
int size;
{
   void *p;

   p = AllocRemember(&(global->gadmem), size, MEMF_CLEAR | MEMF_PUBLIC);
   if (p == NULL) fatal(global, "Not enough memory");
   memset(p, 0, size);
   return(p);
}

void *strmem(global, size)
struct GLOBAL *global;
int size;
{
   void *p;

   p = AllocRemember(&(global->strmem), size, MEMF_CLEAR | MEMF_PUBLIC);
   if (p == NULL) fatal(global, "Not enough memory");
   memset(p, 0, size);
   return(p);
}

void newgad(global, x, y, gadtype, data, width, height)
struct GLOBAL *global;
int x, y;
int gadtype;
char *data;
int width, height;
{
   struct Gadget *gadget;
   struct StringInfo *si;
   struct PropInfo *pi;
   struct Image *gi;
   int bufsize;

   gadget = (struct Gadget *)gmem(global, sizeof(struct Gadget));
   global->gadcount++;
   gadget->NextGadget = global->gadlist;
   global->gadlist    = gadget;

   gadget->LeftEdge   = x;
   gadget->TopEdge    = y;
   gadget->Width      = width;
   gadget->Height     = height;
   gadget->Activation = RELVERIFY;
   gadget->GadgetType = BOOLGADGET;
   gadget->GadgetID   = gadtype;
   gadget->UserData   = (APTR)data;
   gadget->Flags      = GADGHCOMP;

   switch(gadtype)
   {
      case MULTI_V:
         if (*data != NCYCLE_V) break;
      case DEL_V:
         gadget->Flags      |= GADGDISABLED;
         break;
      case STR_V:
         gadget->Flags      |= GADGDISABLED;
      case STRING_V:
         bufsize = MAXBUF;
         gadget->Activation = RELVERIFY /* | STRINGCENTER */;
         /* Kludge - disable the user startup when the startup mode is */
         /* not cycled to the user startup                             */
         if (*data == 0x51 && (global->states[0x24] != 5))
            gadget->Flags      |= GADGDISABLED;
         goto dostrnum;
      case NUM_V:
         gadget->Flags      |= GADGDISABLED;
      case NUMERIC_V:
         bufsize = 6;
         gadget->Activation = RELVERIFY | STRINGRIGHT | LONGINT;
dostrnum:
         gadget->TopEdge += 1;
         gadget->GadgetType = STRGADGET;
         gadget->Flags = GADGHCOMP | (gadget->Flags & GADGDISABLED);
         si = (struct StringInfo *)gmem(global, sizeof(struct StringInfo)+bufsize);
         gadget->SpecialInfo = (APTR)si;
         si->Buffer = (UBYTE *)(si+1);
         si->MaxChars = bufsize;
         si->Buffer[0] = 0;
         /* Now we need to fill in the buffer */
         if (gadtype == NUMERIC_V)
         {
            si->LongInt = global->strtab[*data].value;
            if (si->LongInt != NOVAL)
               sprintf(si->Buffer, "%ld", si->LongInt);
         }
         if ((gadtype == STRING_V) && (global->strtab[*data].text != NULL))
            strcpy(si->Buffer, global->strtab[*data].text);
         break;
      case SCROLL_V:
         pi = (struct PropInfo *)gmem(global, sizeof(struct PropInfo));
         gi = (struct Image *)gmem(global, sizeof(struct Image));
         gi->Width = width;
         gi->Height = 4;
         gadget->SpecialInfo = (APTR)pi;
         gadget->GadgetType = PROPGADGET;
         gadget->Flags = GADGHCOMP;
         gadget->GadgetRender = (APTR)gi;
         pi->Flags = AUTOKNOB | FREEVERT | PROPBORDERLESS;
/*         FindPropValues(0, 3, 0, &pi->VertBody, &pi->VertPot); */
         pi->HorizPot = -1;
         pi->VertPot = 11915;
         pi->HorizBody = -1;
         pi->VertBody = 6553;
         break;
   }
}

void freegads(global)
struct GLOBAL *global;
{
   struct Gadget *gadget;
   struct chkdata *chkd;
   struct StringInfo *si;

   /* We need to clear out the window */
   if (!global->reset)
   {
      SetAPen(global->rp, 0);

      RectFill(global->rp, global->left, global->top, 
                           (WORD)(global->right-1), (WORD)(global->bottom-1));
   }

   /* First eliminate any gadgets that we may already have created.       */
   if (global->gadlist != NULL)
   {
      RemoveGList(global->window, global->gadlist, global->gadcount);
      /* First walk the gadget list and handle any outstanding gadgets */
      for (gadget = global->gadlist; gadget != NULL; gadget = gadget->NextGadget)
      {
         chkd = (struct chkdata *)gadget->UserData;
         si   = (struct StringInfo *)gadget->SpecialInfo;
         switch(gadget->GadgetID)
         {
            case NUMERIC_V:
               global->strtab[chkd->slot].value = si->LongInt;
               break;
            case NUM_V:
               /* chkd->key is the string slot number */
               if (chkd->key != 0)
               {
                  global->strtab[chkd->key].value = si->LongInt + 
                                             (global->states[NCYCLE_V] << 16);
               }
               break;
            case STRING_V:
               if (strlen(si->Buffer))
               {
                  if (global->strtab[chkd->slot].text == NULL)
                     global->strtab[chkd->slot].text = strmem(global, MAXBUF);
                  strcpy(global->strtab[chkd->slot].text, si->Buffer);
               }
               else
               {
                  /* So we waste a little memory here... */
                  /* It comes back when we exit          */
                  global->strtab[chkd->slot].text = NULL;
               }
               break;
            case STR_V:
               /* Now we can fiddle with the string information */
               /* chkd->key is the string slot number */
               if ((chkd->key != 0) && 
                   (global->strtab[chkd->key].text != NULL))
               {
                  strcpy(global->strtab[chkd->key].text, si->Buffer);
               }
               break;
         }
      }
      FreeRemember(&(global->gadmem), TRUE);
      global->gadlist = NULL;
      global->gadcount = 0;
   }
}


void setup(global)
struct GLOBAL *global;
{
   char *p, *tp;
   int xpos, ypos;
   int i;

#if 0
   switch(global->scrnum)
   {
      case 1: p =   MULTI    "\x00" "\x06" "b" "No Debug"            "\0"
                                               "Line Numbers"        "\0"
                                               "Local Symbols"       "\0"
                                               "Local Symbol/Flush"  "\0"
                                               "Full Symbols"        "\0"
                                               "Full Symbol/Flush"   "\0"
                    MULTI    "\x01" "\x03" "d" "Small Data Model"    "\0"
                                               "Large Data Model"    "\0"
                                               "Auto Far Data Model" "\0"
                    MULTI    "\x02" "\x02" "c" "Small Code Model"    "\0"
                                               "Large Code Model"    "\0"
                    MULTI    "\x03" "\x05" "f" "No Floating Point"   "\0"
                                               "IEEE FP"             "\0"
                                               "Lattice FP"          "\0"
                                               "FFP (floats)"        "\0"
                                               "M68881 FP"           "\0"
                    MULTI    "\x04" "\x06" "e" "68000 Code"          "\0"
                                               "68010 Code"          "\0"
                                               "68020 Code"          "\0"
                                               "68030 Code"          "\0"
                                               "68040 Code"          "\0"
                                               "Any Processor"       "\0"
                    MULTI    "\x05" "\x03" "p" "Stack Parameters"    "\0"
                                               "Register Parms"      "\0"
                                               "Dual Parameters"     "\0"
                    BUTTON   BTN_ADV       "a" "Advanced Options..." "\0"
                    BUTTON   BTN_OBJ       "o" "Object Options..."   "\0"
                    NEWCOLUMN
                    CHECK    "\x06"        "r" "Strict ANSI"         "\0"
                    CHECK    "\x07"        "g" "Global Optimizer"    "\0"
                    CHECK    "\x08"        "i" "Short Integers"      "\0"
                    CHECK    "\x09"        "k" "No Stack Checking"   "\0"
                    CHECK    "\x0a"        "t" "Typical Link Opts"   "\0"
                    CHECK    "\x0b"        "y" "Require Prototypes"  "\0"
                    CHECK    "\x0c"        "s" "Single String Copy"  "\0"
                    CHECK    "\x0d"        "u" "Unsigned Characters" "\0"
                    CHECK    "\x0e"        "q" "Quiet int return"    "\0"
                    CHECK    "\x38"        "n" "Invoke LSE on error" "\0"
                    CHECK    "\x39"        "m" "No stdio (TINYMAIN)" "\0"
                    NEWCOLUMN
                    LIST     "\x00"        "~" "Include Directories" "\0"
                    LIST     "\x10"        "~" "#define Symbols"     "\0"
                    ADJUST   "\xFC"
                    STRING   "\x56"        "~" "Project Name"        "\0"
                    BUTTON   BTN_SAVE      "~" "Save"                "\0"
                    BUTTON   BTN_DEFAULT   "~" "Save Default"        "\0"
                    BUTTON   BTN_CANCEL    "~" "Cancel"              "\0";
              tp = "SAS/C� Compiler Options";
              break;
      case 2: p = 
                    CHECK    "\x0f"        "c" "Nested Comments"     "\0"
                    CHECK    "\x10"        "$" "$ in Identifiers"    "\0"
                    CHECK    "\x11"        "r" "No Error Line"       "\0"
                    CHECK    "\x12"        "t" "No Multi-includes"   "\0"
                    CHECK    "\x13"        "a" "Allow chip/near/far" "\0"
                    CHECK    "\x14"        "-" "Multi-Char Constant" "\0"
                    CHECK    "\x15"        "d" "Disallow __asm"      "\0"
                    CHECK    "\x16"        "w" "Warn Undefined Tags" "\0"
                    CHECK    "\x17"        "8" "8 Char Identifiers"  "\0"
                    CHECK    "\x18"        "p" "Structure Passing"   "\0"
                    CHECK    "\x37"        "q" "Struct Equivalence"  "\0"
                    NEWCOLUMN
                    CHECK    "\x19"        "l" "List Source"         "\0"
                    CHECK    "\x1a"        "m" "List Macros"         "\0"
                    CHECK    "\x1b"        "e" "List Excluded Lines" "\0"
                    CHECK    "\x1c"        "i" "List Included Files" "\0"
                    CHECK    "\x1d"        "h" "List Header Files"   "\0"
                    CHECK    "\x1e"        "x" "XREF"                "\0"
                    CHECK    "\x1f"        "s" "XREF define Symbols" "\0"
                    CHECK    "\x20"        "f" "XREF Compiler Files" "\0"
                    CHECK    "\x21"        "u" "Undefine Symbols"    "\0"
                    ADJUST   "\xFF"
                    NUMERIC  "\x57"        "?" "Error Limit"         "\0"
                    NEWCOLUMN
                    LIST     "\x20"        "p" "Precompiled headers" "\0"
                    NLIST    "\x30"        "g" "Error/Warning Control" "\0"
                    ADJUST   "\xF8"
                    NUMERIC  "\x58"        "/" "Warn Limit"          "\0"
#if 0
                    BUTTON   BTN_USE       "u" "Use"                 "\0"
                    BUTTON   BTN_ALTCAN    "n" "Cancel"              "\0"
#endif
                    BUTTON   BTN_OK        "o" "OK"                  "\0"
                    ;
              tp = "SAS/C� Advanced Compiler Options";
              break;
      case 3: p =   MULTI    "\x24" "\x06" "s" "Normal Startup"      "\0"
                                               "Resident Startup"    "\0"
                                               "Background Startup"  "\0"
                                               "Catch Startup"       "\0"
                                               "CatchRes Startup"    "\0"
                                               "Special Startup"     "\0"
                    MULTI    "\x25" "\x03" "c" "Normal Code"         "\0"
                                               "Fast Ram Code"       "\0"
                                               "Chip Ram Code"       "\0"
                    MULTI    "\x26" "\x03" "d" "Normal Data"         "\0"
                                               "Fast Ram Data"       "\0"
                                               "Chip Ram Data"       "\0"
                    MULTI    "\x27" "\x03" "b" "Normal BSS"          "\0"
                                               "Fast Ram BSS"        "\0"
                                               "Chip Ram BSS"        "\0"
                    MULTI    "\x28" "\x02" "z" "Optimize Space"      "\0"
                                               "Optimize Time"       "\0"
                    ADJUST   "\x04"
                    CHECK    "\x29"        "r" "No Auto-Register"    "\0"
                    CHECK    "\x2a"        "f" "Default Segments"    "\0"
                    CHECK    "\x2b"        "4" "Load A4 at start"    "\0"
                    CHECK    "\x2c"        "m" "Disable Call Merge"  "\0"
                    CHECK    "\x39"        "i" "No Linking in LSE"   "\0"
                    NEWCOLUMN
                    CHECK    "\x2d"        "a" "ADDSYM"              "\0"
                    CHECK    "\x2e"        "e" "SMALLCODE"           "\0"
                    CHECK    "\x2f"        "t" "SMALLDATA"           "\0"
                    CHECK    "\x30"        "v" "VERBOSE"             "\0"
                    CHECK    "\x31"        "g" "NODEBUG"             "\0"
                    CHECK    "\x32"        "h" "Map Hunk"            "\0"
                    CHECK    "\x33"        "p" "Map Symbols"         "\0"
                    CHECK    "\x34"        "l" "Map Libraries"       "\0"
                    CHECK    "\x35"        "x" "Map Xref"            "\0"
                    CHECK    "\x36"        "y" "Map Overlay"         "\0"
                    NEWCOLUMN
                    LIST     "\x40"        "j" "Linker Objects"      "\0"
                    ADJUST   "\xf4"
                    STRING   "\x50"        "k" "Make Library"        "\0"
                    STRING   "\x51"        "u" "User Startup"        "\0"
                    ADJUST   "\x04"
                    STRING   "\x52"        "e" "Code Name"           "\0"
                    STRING   "\x53"        "1" "Data Name"           "\0"
                    STRING   "\x54"        "2" "BSS Name"            "\0"
#if 0
                    BUTTON   BTN_USE       "u" "Use"                 "\0"
                    BUTTON   BTN_ALTCAN    "n" "Cancel"              "\0"
#endif
                    BUTTON   BTN_OK        "o" "OK"                  "\0"
                    ;
              tp = "SAS/C� Advanced Object Options";
              break;
      case 6:
      case 4: p =   NEWCOLUMN
                    ADJUST   "\x40"
                    STRING   "\x55"        "f" "File Name"           "\0"
                    BUTTON   BTN_USE       "u" "Use"                 "\0"
                    BUTTON   BTN_ALTCAN    "n" "Cancel"              "\0"
                    ;
              tp = "Select File";
              break;
      case 5: p =   BUTTON   BTN_OK        "o" "OK"                  "\0"
                    ;
              tp = "Unable to perform operation";
              break;
      case 7: p =   BUTTON   BTN_OK        "o" "OK"                  "\0"
                    ;
              tp = "Processing File";
              break;
      default: p =  BUTTON   BTN_OK        "o" "OK"                  "\0"
                    ;
               tp = "Unknown State - Contact SAS/C� Technical Support";
              break;
   }

   SetWindowTitles(global->window, tp, 
  "Copyright � 1990 by SAS Institute, Inc. Cary NC USA.  All Rights Reserved");

   /* Now p points to a description of the compiler options.  We could    */
   /* At some time in the future take this string from an external source */
   /* (Such as even the icon) but for this iteration we will not do so    */
   /* We need to step through the string interpreting the gadgets and     */
   /* rendering them on the screen.                                       */

   freegads(global);

   xpos = global->left + 14;
   ypos  = global->top  + 10;
   /* Now walk through the option list and set up the gadgets */
   while(*p)
   {
      switch(*p++)
      {
         case MULTI_V:
            newgad(global, xpos, ypos, MULTI_V, p, 180, 14);
            ypos += 14;
            p ++;
            i = *p++;
            p++;
            while(i--) p+= strlen(p)+1;
            break;
         case CHECK_V:
            newgad(global, xpos, ypos, CHECK_V, p, 26, 11);
            ypos += 13;
            p += 2;
            p += strlen(p)+1;
            break;
         case NEWCOLUMN_V:
            xpos += 190;
            ypos = global->top + 10;
            break;
         case ADJUST_V:
            ypos += *(signed char *)p;
            p++;
            break;
         case STRING_V:
            ypos += 14;
            newgad(global, xpos+105, ypos, STRING_V, p, 105, 8);
            p++;
            p += strlen(p)+1;
            break;
         case NUMERIC_V:
            ypos += 13;
                           /* 140 when in last col */
            newgad(global, xpos+102, ypos, NUMERIC_V, p, 42, 8);
            p++;
            p += strlen(p)+1;
            break;
         case NLIST_V:
            ypos += 10;
            tp = (char *)gmem(global, 23);
            tp[0] = *p;  tp[1] = 0; tp[2] = 0;
            memcpy(tp+3, NCYCLE "\x03" "~" "Ignore" "\0"
                                           "Warn"   "\0"
                                           "Error"  "\0", 21);
            newgad(global, xpos+2,   ypos,    NLIST_V,      p, 190, 11);
            newgad(global, xpos+2,   ypos+11, NITEM2_V,     p, 190, 11);
            newgad(global, xpos+2,   ypos+22, NITEM3_V,     p, 190, 11);
            newgad(global, xpos+197, ypos+1,  SCROLL_V,     p, 10,  13);
            newgad(global, xpos+194, ypos+16, UP_V,         p, 16,  9);
            newgad(global, xpos+194, ypos+25, DOWN_V,       p, 16,  9);
            newgad(global, xpos+4,   ypos+36, NUM_V,       tp, 52,  8);
            newgad(global, xpos+60,  ypos+34, MULTI_V,   tp+3, 86,  14);
            newgad(global, xpos+146, ypos+34, NEW_V,        p, 32,  14);
            newgad(global, xpos+178, ypos+34, DEL_V,        p, 32,  14);
            ypos += 52;
            p++;
            p += strlen(p)+1;
            break;
         case LIST_V:
            ypos += 10;
            tp = (char *)gmem(global, 3);
            tp[0] = *p;  tp[1] = 0; tp[2] = 0;
            newgad(global, xpos+2,   ypos,    LIST_V,   p, 190, 11);
            newgad(global, xpos+2,   ypos+11, ITEM2_V,  p, 190, 11);
            newgad(global, xpos+2,   ypos+22, ITEM3_V,  p, 190, 11);
            newgad(global, xpos+197, ypos+1,  SCROLL_V, p, 10,  13);
            newgad(global, xpos+194, ypos+16, UP_V,     p, 16,  9);
            newgad(global, xpos+194, ypos+25, DOWN_V,   p, 16,  9);
            newgad(global, xpos+4,   ypos+36, STR_V,   tp, 138, 8);
            newgad(global, xpos+146, ypos+34, NEW_V,    p, 32,  14);
            newgad(global, xpos+178, ypos+34, DEL_V,    p, 32,  14);
            ypos += 52;
            p++;
            p += strlen(p)+1;
            break;
         case BUTTON_V:
            i = *p;
            switch(i)
            {
               case BTN_ADV_V:  /* Advanced Options */
               case BTN_OBJ_V:  /* Object Options   */
                     ypos += 16;
                     newgad(global, xpos, ypos, BUTTON_V, p, 180, 14);
                     break;
               case BTN_SAVE_V:  /* Save */
               case BTN_USE_V:  /* Use */
                     newgad(global, global->left+10, global->bottom-18,
                                    BUTTON_V, p, 64, 14);
                     break;
               case BTN_OK_V:     /* OK */
               case BTN_DEFAULT_V:  /* Save Default */
                     newgad(global, global->left+(global->right-global->left)/2 - 60,
                                    global->bottom-18,
                                    BUTTON_V, p, 120, 14);
                     break;
               case BTN_CANCEL_V:  /* Cancel */
               case BTN_ALTCAN_V:  /* Cancel */
                     newgad(global, global->right-74, global->bottom-18,
                                    BUTTON_V, p, 64, 14);
                     break;
            }
            p += 2;
            p += strlen(p)+1;
            break;
      }
   }
   AddGList(global->window, global->gadlist, -1, global->gadcount, NULL);
   RefreshGList(global->gadlist, global->window, NULL, -1);
   refresh(global);
#endif
}

void setstate(global, state, id, button)
struct GLOBAL *global;
int state;
int id;
int button;
{
#if 0
   struct Gadget *gadget;
   struct chkdata *chkd;
   struct StringInfo *si;

   gadget = findgad(global, id, button);
   if (gadget == NULL) return;

   chkd = (struct chkdata *)gadget->UserData;
   si = (struct StringInfo *)gadget->SpecialInfo;

   /* Found the right gadget, now hit it */
   if (state < 0)
   {
      /* First we need to save away the old gadget information */
      RemoveGadget(global->window, gadget);
      if (id == STR_V)
      {
         /* Now we can fiddle with the string information */
         /* chkd->key is the string slot number */
         if ((chkd->key != 0) && (global->strtab[chkd->key].text != NULL))
         {
            strcpy(global->strtab[chkd->key].text, si->Buffer);
         }
         state = -state;
         chkd->key = state;
         si->Buffer[0] = 0;
         si->BufferPos = 0;
         si->LongInt = NOVAL;
         if (global->strtab[state].text != NULL)
         {
            strcpy(si->Buffer, global->strtab[state].text);
         }
      }
      else
      {
         /* Must be a numeric one */
         /* chkd->key is the string slot number */
         if (chkd->key != 0)
         {
            global->strtab[chkd->key].value = si->LongInt + 
                                              (global->states[NCYCLE_V] << 16);
         }
         state = -state;
         chkd->key = state;
         si->Buffer[0] = 0;
         si->BufferPos = 0;
         si->LongInt = NOVAL;
         if (global->strtab[state].value != NOVAL)
         {
            si->LongInt   = global->strtab[state].value & 0xffff;
            sprintf(si->Buffer, "%ld", si->LongInt);
            global->states[NCYCLE_V] = (global->strtab[chkd->key].value >> 16) & 0xff;
         }
      }
      /* Having munged that information, update it */
      gadget->Flags      &= ~GADGDISABLED;
      AddGadget(global->window, gadget, -1);
      RefreshGList(gadget, global->window, NULL, 1);
      ActivateGadget(gadget, global->window, NULL);
    }
   else
   {
      switch(state)
      {
         case 0:
            OffGadget(gadget, global->window, NULL);
            if (gadget->GadgetID == STR_V)
            {
               /* First we need to save away the old gadget information */
               RemoveGadget(global->window, gadget);
               /* Now we can fiddle with the string information */
               /* chkd->key is the string slot number */
               if ((chkd->key != 0) && (global->strtab[chkd->key].text != NULL))
               {
                  strcpy(global->strtab[chkd->key].text, si->Buffer);
               }
               chkd->key = 0;
               si->Buffer[0] = 0;
               si->BufferPos = 0;
               si->LongInt = NOVAL;
               /* Having munged that information, update it */
               gadget->Flags      |= GADGDISABLED;
               AddGadget(global->window, gadget, -1);
               RefreshGList(gadget, global->window, NULL, 1);
            }
            else if (gadget->GadgetID == NUM_V)
            {
               /* First we need to save away the old gadget information */
               RemoveGadget(global->window, gadget);
               /* Now we can fiddle with the string information */
               /* chkd->key is the string slot number */
               if (chkd->key != 0 && si->LongInt != NOVAL)
               {
                  global->strtab[chkd->key].value = si->LongInt + 
                                                     (global->states[NCYCLE_V] << 16);
               }
               chkd->key = 0;
               si->Buffer[0] = 0;
               si->BufferPos = 0;
               si->LongInt = NOVAL;
               /* Having munged that information, update it */
               gadget->Flags      |= GADGDISABLED;
               AddGadget(global->window, gadget, -1);
               RefreshGList(gadget, global->window, NULL, 1);
            }
            break;
         case 1:
            OnGadget(gadget, global->window, NULL);
            if (gadget->GadgetID == STR_V ||
                gadget->GadgetID == STRING_V)
            {
               ActivateGadget(gadget, global->window, NULL);
            }
            break;
         case 2:
           break;
      }
   }
   render(global, gadget);
#endif
}

struct Gadget *findgad(global, id, button)
struct GLOBAL *global;
int id;
int button;
{
   struct Gadget *gadget;
   struct chkdata *chkd;

   for (gadget = global->gadlist; gadget != NULL; gadget = gadget->NextGadget)
   {
      chkd = (struct chkdata *)gadget->UserData;
      if ((gadget->GadgetID == id) && (chkd->slot == button))
      {
         return(gadget);
      }
   }
   return(NULL);
}

int strslot(global, button)
struct GLOBAL *global;
int button;
{
   struct Gadget *gadget;
   struct chkdata *chkd;

   gadget = findgad(global, STR_V, button);
   if (gadget == NULL)
      gadget = findgad(global, NUM_V, button);
   if (gadget != NULL)
   {
      chkd = (struct chkdata *)gadget->UserData;
      return((int)chkd->key);
   }
   return(0);
}