#include "configopts.h"

void resetscroll(global, gadget)
struct GLOBAL *global;
struct Gadget *gadget;
{
   struct PropInfo *pi;
   struct chkdata *chkd;
   USHORT first, total;
   USHORT vbody, vpot;

   pi   = (struct PropInfo *)gadget->SpecialInfo;
   chkd = (struct chkdata  *)gadget->UserData;

   /* We can find out the current value by just looking at the slot */
   first = global->strtab[chkd->slot].value;
   for (total = 0; total < LISTMAX; total++)
   {
      if (chkd->slot == 0x30)
      {
         if (global->strtab[chkd->slot+total+1].value == NOVAL)
            break;
      }
      else
      {
         if (global->strtab[chkd->slot+total+1].text == NULL)
            break;
      }
   }
   if (total < 4)
   {
      first = 0;
   }
   else if (first > (total-3))
   {
      first = total-3;
   }
   global->strtab[chkd->slot].value = first;
   FindPropValues(total, 3, first, &vbody, &vpot);
   ModifyProp(gadget, global->window, NULL, pi->Flags,
               pi->HorizPot, vpot, pi->HorizBody, vbody); 
}

void readscroll(global, gadget)
struct GLOBAL *global;
struct Gadget *gadget;
{
   struct PropInfo *pi;
   struct chkdata *chkd;
   USHORT total;

   pi   = (struct PropInfo *)gadget->SpecialInfo;
   chkd = (struct chkdata  *)gadget->UserData;

   /* We can find out the current value by just looking at the slot */
   for (total = 0; total < LISTMAX; total++)
   {
      if (chkd->slot == 0x30)
      {
         if (global->strtab[chkd->slot+total+1].value == NOVAL)
            break;
      }
      else
      {
         if (global->strtab[chkd->slot+total+1].text == NULL)
            break;
      }
   }
   global->strtab[chkd->slot].value = FindPropFirst(total, 3, pi->VertPot);
}
