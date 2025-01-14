#include "configopts.h"

void handlehit(global, gadget, back)
struct GLOBAL *global;
struct Gadget *gadget;
int back;
{
   struct cycdata *cycd;
   struct chkdata *chkd;
   int i,j;

   chkd = (struct chkdata *)gadget->UserData;
   switch(gadget->GadgetID)
   {
      case MULTI_V:
         cycd = (struct cycdata *)chkd;
         i = global->states[cycd->slot];
         if (back) i--; else i++;
         if (i >= cycd->count) i = 0;
         if (i < 0) i = cycd->count-1;
         global->states[cycd->slot] = i;
         rendercyc(global, gadget);
         break;
      case UP_V:
         if (global->strtab[chkd->slot].value)
         {
            global->strtab[chkd->slot].value--;
            setstate(global, 1, SCROLL_V, chkd->slot);
            setstate(global, 1, LIST_V, chkd->slot);
            setstate(global, 1, NLIST_V, chkd->slot);
         }
         break;
      case DOWN_V:
         i = chkd->slot + global->strtab[chkd->slot].value;
         if ((i < (chkd->slot+LISTMAX-3)) && 
                ((chkd->slot != 0x30) && (global->strtab[i+3].text  != NULL )))
         {
            global->strtab[chkd->slot].value++;
            setstate(global, 1, SCROLL_V, chkd->slot);
            setstate(global, 1, LIST_V, chkd->slot);
            setstate(global, 1, NLIST_V, chkd->slot);
         }
         break;
      case SCROLL_V:
         readscroll(global, gadget);
         setstate(global, 1, SCROLL_V, chkd->slot);
         setstate(global, 1, LIST_V, chkd->slot);      
         setstate(global, 1, NLIST_V, chkd->slot);
         break;
      case CHECK_V:
         global->states[chkd->slot] = !global->states[chkd->slot];
         renderchk(global, gadget);
         break;
      case NEW_V:
         /* We need to allocate storage for the string here */
         /* First insert the entry into the slot */
         /* figure out what the current slot is for the string */
         i = strslot(global, chkd->slot);
         if (i == 0) i = chkd->slot;
         i++;
         /* We need to go through and shuffle down the other slots */
         for(j = chkd->slot+LISTMAX; j > i; j--)
         {
            global->strtab[j] = global->strtab[j-1];
         }
         global->strtab[i].text = strmem(global, MAXBUF);
         setstate(global, -i, STR_V, chkd->slot);
         if (global->strtab[chkd->slot+LISTMAX].text != NULL)
            setstate(global, 0, NEW_V, chkd->slot);
         setstate(global, 1, LIST_V, chkd->slot);
         setstate(global, 1, SCROLL_V, chkd->slot);
         setstate(global, 1, DEL_V, chkd->slot);
         break;
      case DEL_V:
         i = strslot(global, chkd->slot);
         setstate(global, 0, STR_V, chkd->slot);
         setstate(global, 0, NUM_V, chkd->slot);
         while(i < chkd->slot+LISTMAX)
         {
            global->strtab[i] = global->strtab[i+1];
            i++;
         }
         global->strtab[i].text = NULL;
      case NUM_V:
      case STR_V:
         setstate(global, 1, SCROLL_V, chkd->slot);
         setstate(global, 0, STR_V, chkd->slot);
         setstate(global, 0, NUM_V, chkd->slot);
         setstate(global, 0, MULTI_V, NCYCLE_V);
         setstate(global, 0, DEL_V, chkd->slot);
         setstate(global, 1, NEW_V, chkd->slot);
         setstate(global, 1, LIST_V, chkd->slot);
         setstate(global, 1, NLIST_V, chkd->slot);
         break;
      case NLIST_V:
         i = 1;
         goto donitem;
      case NITEM2_V:
         i = 2;
         goto donitem;
      case NITEM3_V:
         i = 3;
donitem: i += global->strtab[chkd->slot].value + chkd->slot;
         if (global->strtab[i].value != NOVAL)
         {
            setstate(global, -i,  NUM_V, chkd->slot);
            setstate(global, 1,  MULTI_V, NCYCLE_V);
            setstate(global, 1,  DEL_V, chkd->slot);
         }
         setstate(global, 1, NLIST_V, chkd->slot);
         break;

      case LIST_V:
         i = 1;
         goto doitem;
      case ITEM2_V:
         i = 2;
         goto doitem;
      case ITEM3_V:
         i = 3;
doitem:  i += global->strtab[chkd->slot].value + chkd->slot;
         if (global->strtab[i].text != NULL)
         {
            setstate(global, -i, STR_V, chkd->slot);
            setstate(global, 1,  DEL_V, chkd->slot);
         }
         setstate(global, 1, LIST_V, chkd->slot);
         break;
      case STRING_V:
         if (chkd->slot == 0x55) goto dook;
         break;
      case BUTTON_V:
         switch(chkd->slot)
         {
            case BTN_ADV_V:
               global->scrnum = 2;
               break;
            case BTN_OBJ_V:
               global->scrnum = 3;
               break;
            case BTN_SAVE_V:
               freegads(global);
               global->scrnum = 0;
               break;
            case BTN_DEFAULT_V:
               freegads(global);
               global->scrnum = 0;
               break;
            case BTN_CANCEL_V:
               if (global->scrnum == 4)
                  global->scrnum = 1;
               else
                  global->scrnum = 0;
               break;
            case BTN_OK_V:
            case BTN_USE_V:
dook:
               /* Force the string to be saved */
               if (global->scrnum == 6)
               {
                  global->scrnum = 7;
                  setup(global);
                  global->scrnum = 1;
               }
               else if (global->scrnum == 4)
               {
                  global->scrnum = 7;
                  setup(global);
                  global->scrnum = 1;
               }
               else
               {
                  global->scrnum = 1;
               }
               break;
            case BTN_ALTCAN_V:
               global->scrnum = 1;
               break;
         }
         if (global->scrnum) setup(global);
         break;
   }
}

void handlekey(global, key, back)
struct GLOBAL *global;
int key;
int back;
{
#if 0
   struct Gadget *gadget;
   struct cycdata *cycd;
   struct chkdata *chkd;
   int    testkey;

   if (key == '~')
   {
      DisplayBeep(NULL);
      return;
   }

   if (key >= 'A' && key <= 'Z')
      key += 'a'-'A';

   for(gadget = global->gadlist; gadget != NULL; gadget = gadget->NextGadget)
   {
      chkd = (struct chkdata *)gadget->UserData;
      switch(gadget->GadgetID)
      {
         case MULTI_V:
            cycd = (struct cycdata *)chkd;
            testkey = cycd->key;
            break;
         case CHECK_V:
         case BUTTON_V:
            testkey = chkd->key;
            break;
         default:
            testkey = -1;
            break;
      }
      if (key == testkey)
      {
         handlehit(global, gadget, back);
         return;
      }
   }
#endif
   DisplayBeep(NULL);
}

void handlemenu(global, menunum, itemnum)
struct GLOBAL *global;
int menunum;
int itemnum;
{
#if 0
   int reshow = 0;
   int i, len;
   char buffer[256];

   switch(menunum*7+itemnum)
   {
      case 0:    /* 0, 0 -  Open ...         */
         if (AslBase != NULL)
         {
            global->filereq->rf_Hail = "Open Options:";
            if (RequestFile(global->filereq))
            {
               strncpy(buffer, global->filereq->rf_Dir, 254);
               i = strlen(buffer);
               if (i && buffer[i-1] != ':' && buffer[i-1] != '/')
                  buffer[i++] = '/';
               len = 255-i;
               strncpy(buffer+i, global->filereq->rf_File, len);
               if (loadopts(global, buffer, 1))
               {
                  global->scrnum = 1;
               }
            }
            reshow = 1;
            break;
         }
         global->scrnum = 4;
         reshow = 1;
         break;
      case 1:    /* 0, 1 -                   */
         break;
      case 2:    /* 0, 2 -  Save             */
         freegads(global);
         saveopts(global, "SASCOPTS");
         break;
      case 3:    /* 0, 3 -  Save As          */
         freegads(global);
         if (AslBase != NULL)
         {
            /* .... */
            global->filereq->rf_Hail = "Save As:";
            if (RequestFile(global->filereq))
            {
               strncpy(buffer, global->filereq->rf_Dir, 254);
               i = strlen(buffer);
               if (i && buffer[i-1] != ':' && buffer[i-1] != '/')
                  buffer[i++] = '/';
               len = 254-i;
               strncpy(buffer+i, global->filereq->rf_File, len);
               if (saveopts(global, buffer))
               {
                  global->scrnum = 1;
               }
            }
            reshow = 1;
            break;            
         }
         global->scrnum = 6;
         reshow = 1;
         break;
      case 4:    /* 0, 4 -  Save As Default  */
         freegads(global);
         saveopts(global, "ENV:SASCOPTS");
         break;
      case 5:    /* 0, 5                     */
         break;
      case 6:    /* 0, 6 -  Quit             */
         global->scrnum = 0;
         break;
      case 7:    /* 1, 0 -  Reset to default */
         resetall(global);
         reshow = 2;
         break;
      case 8:    /* 1, 1 -  Last Saved       */
         resetall(global);
         if (!loadopts(global, "ENV:SASCOPTS", 0))
            resetall(global);
         reshow = 2;
         break;
      case 9:    /* 1, 2 -  Restore          */
         resetall(global);
         if (loadopts(global, "SASCOPTS", 0) <= 0)
            if (!loadopts(global, "ENV:SASCOPTS", 0))
               resetall(global);
         reshow = 2;
         break;
   }
   if (reshow == 2)
   {
      global->reset = 2;
      setup(global);
      global->reset = 0;
   }
   else if (reshow)
   {
      setup(global);
   }
#endif
}

void resetall(global)
struct GLOBAL *global;
{
#if 0
   int i;

   freegads(global);
   memset(global->states, 0, sizeof(global->states));
   memset(global->strtab, 0, sizeof(global->strtab));
 
   global->strtab[0x57].value = NOVAL;  /* Error limit */
   global->strtab[0x58].value = NOVAL;  /* Warn Limit */
   for (i = 0x31; i < 0x40; i++)
      global->strtab[i].value = NOVAL;  /* Warn Limit */
#endif
}
