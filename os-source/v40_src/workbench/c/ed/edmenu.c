/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                             BBS: (919) 481-6436    */
/* | o  | ||   John Toebes     John Mainwaring    Jim Cooper                 */
/* |  . |//    Bruce Drake     Gordon Keener      Dave Baker                 */
/* ======      Doug Walker                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**     (C) Copyright 1989 Commodore-Amiga, Inc.
 **         All Rights Reserved
**/

/*----------------------------------------------------------------------*/
/* Command: edmenu.c                                                    */
/* Author:  John A. Toebes, VIII                                        */
/* Change History:                                                      */
/*  Date    Person        Action                                        */
/* -------  ------------- -----------------                             */
/* 28MAR90  John Toebes   Initial Creation                              */
/* 06APR90  John Toebes   Changed menu initialization                   */
/* 06APR90  John Toebes   Added BAR support                             */
/* 30MAY90  John Toebes   Updated prompt strings for menu items         */
/*                        Eliminated CreateMenusA warning (Finally!)    */
/* 26OCT90  John Toebes   Added (ugh) menu validation                   */
/* 30OCT90  John Toebes   Improved out of memory handling               */
/* 01NOV90  John Toebes   Eliminated Unnecessary local SysBases         */
/* 06NOV90  John Toebes   Corrected problem with freeing displayed menu */
/* 13NOV90  John Toebes   Corrected limit testing for menus             */
/* Notes:                                                               */
/*----------------------------------------------------------------------*/
#include "ed.h"
#include "utility/tagitem.h"
#include "pragmas/intuition_pragmas.h"

/***
*
* Initialize all menus
*
***/
void init_menus()
{
   execute_series("SI0 1'Project'"    "\0"
                  "SI1 2'New            ESCnw''NW'"   "\0"
                  "SI2 2'Open...        ESCop''OP ?/Open File:/'"      "\0"
                  "SI3 2'Insert File... ESCif''IF ?/Insert File:/'" "\0"
                  "SI4 4"      "\0"
                  "SI5 2'Write Block... ESCwb''WB ?/Write File:/'" "\0"
                  "SI6 2'Save           ESCsa''SA'"  "\0"
                  "SI7 2'Save As...     ESCsa''SA ?/Save As File:/'" "\0"
                  "SI8 2'Save & Exit    ESCx''X'"    "\0"
                  "SI9 4"      "\0"
                  "SI10 2'About          ESCsh''SH'"        "\0"
                  "SI11 4"     "\0"
                  "SI12 2'Quit           ESCq''Q'"   "\0"

                  "SI13 1'Edit'"      "\0"
                  "SI14 2'Undo Line      ESCu''U'"   "\0"
                  "SI15 4"     "\0"
                  "SI16 2'Start Block    ESCbs''BS'" "\0"
                  "SI17 2'End Block      ESCbe''BE'" "\0"
                  "SI18 2'Show Block     ESCsb''SB'" "\0"
                  "SI19 2'Insert Block   ESCib''IB'" "\0"
                  "SI20 4"     "\0"
                  "SI21 2'Delete Block   ESCdb''DB'" "\0"
                  "SI22 2'Delete Line      ^B''D'"   "\0"

                  "SI23 1'Movement'"  "\0"
                  "SI24 2'Top            ESCt''T'"   "\0"
                  "SI25 2'Bottom         ESCb''B'"   "\0"
                  "SI26 4"     "\0"
                  "SI27 2'Go to Line..   ESCm''M ?/Goto Line:/'"     "\0"
                  "SI28 4"     "\0"
                  "SI29 2'Next Page        ^D''PD'"  "\0"
                  "SI30 2'Previous Page    ^U''PU'"  "\0"

                  "SI31 1'Search'"    "\0"
                  "SI32 2'Find...            ESCs''F ?/String:/'"     "\0"
                  "SI33 2'Find Next''F'"     "\0"
                  "SI34 4"     "\0"
                  "SI35 2'Reverse Find...    ESCbf''BF ?/String:/'"   "\0"
                  "SI36 2'Reverse Find Next    ''BF'" "\0"
                  "SI37 4"     "\0"
                  "SI38 2'Replace...         ESCe''E ?/Search:/ ?/Replace:/'" "\0"
                  "SI39 2'Global Replace...  ESCrp e''E ?/Search:/ ?/Replace:/;RP E'" "\0"
                  "SI40 4"     "\0"
                  "SI41 2'Query Replace...   ESCeq''EQ ?/Search:/ ?/Replace:/'"       "\0"
                  "SI42 2'Global Q-Replace...ESCrp eq''EQ ?/Search:/ ?/Replace:/;RP EQ'"      "\0"

                  "SI43 1'Settings'"  "\0"
                  "SI44 2'Set FN Key...  ESCsf''SF ?/Key:/ ?/Command:/'"    "\0"
                  "SI45 2'Show FN Key... ESCdf''DF ?/Key:/'"       "\0"
                  "SI46 4"     "\0"
                  "SI47 2'Reset Keys     ESCrk''RK'"   "\0"
                  "SI48 4"     "\0"
                  "SI49 2'Right Margin...ESCsr''SR ?/Right Margin:/'"     "\0"
                  "SI50 2'Left Margin... ESCsl''SL ?/Left Margin:/'"       "\0"
                  "SI51 4"     "\0"
                  "SI52 2'Ignore Case    ESCuc''UC'" "\0"
                  "SI53 2'Case Sensitive ESClc''LC'" "\0"

                  "SI54 1'Commands'"  "\0"
                  "SI55 2'Extended Command...ESCcm''CM'"  "\0"
                  "SI56 2'Repeat Last        ESCre''RE'"  "\0"
                  "SI57 4"     "\0"
                  "SI58 2'Run File...        ESCrf''RF ?/Command File:/'" "\0"
                  "SI59 4"     "\0"
                  "SI60 2'AREXX Command...   ESCrx''RX ?/Rexx CMD:/'"        "\0"
                  "SI61 4"     "\0"
                  "SI62 2'Redisplay          ESCvw''VW'"    "\0"
                  "SI63 0;EM"  "\0"
                 );
}

/***
*
* Enable any menu that is currently pending
*
***/
void enable_menu()
{
   struct TagItem taglist[2];
   int i, level, newlev, count[4];
   
   taglist[0].ti_Tag = GTMN_FrontPen;
   taglist[0].ti_Data = 0;
   taglist[1].ti_Tag = TAG_DONE;

   if (window != NULL)
   {
      if (vi == NULL)
         if (!(vi = GetVisualInfoA(window->WScreen, taglist+1)))
            return;

      if (menuattached)
      {
         if(window)ClearMenuStrip(window);
         menuattached = 0;
      }

      if (menu != NULL)
      {
         FreeMenus(menu);
         menu = NULL;
      }
      /* Let us go through and validate the menu they have constructed */
      level = 0;
      for (i = 0; i < MAXMENU; i++)
      {
         newlev = newmenu[i].nm_Type;
         if (newlev > level)
         {
            level++;
            /* Make sure we are going down only one level */
            if (newmenu[i].nm_Type != level || level > NM_SUB) return;
            count[level] = 0; /* Initialize our count */
         }
         else if (newlev == level)
         {
            /* Make sure we don't have two TITLES in a row */
            if (level == NM_TITLE) return;
            count[level]++;
         }
         else
         {
            /* We are going up a level.  Validate the counts on the way up */
            if (newlev < NM_END) return;
            while (level > newlev)
            {
               if (count[level] >= "!\x20\x40\x20"[level])
                  return;
               else
                  level--;
            }
            /* When we hit the end of the menus, break */
            if (level == 0) break;
            count[level]++;
         }
      }

      /* Lastly ensure that they had an END marker */
      if (level) return;

      if (!(menu = CreateMenusA(newmenu, taglist)))
         return;
      
      if (!LayoutMenusA(menu, vi, taglist+1))
         return;

      menuattached = SetMenuStrip(window, menu);
   }
}

/***
*
* Set up a menu item to a given string and command
*
***/
void set_menu_item(num, type, str, cmd)
int num;
int type;
char *str;
char *cmd;
{
   if (num < 0 || num >= MAXMENU)
      return;

   if (menuattached)
   {
      ClearMenuStrip(window);
      menuattached = 0;
   }
   /* First we free up any menu item that might be there */
   if (newmenu[num].nm_UserData != NULL)
      FreeVec(newmenu[num].nm_UserData);
   if ((newmenu[num].nm_Label != NULL) &&
       (newmenu[num].nm_Label != NM_BARLABEL))
      FreeVec(newmenu[num].nm_Label);
   newmenu[num].nm_UserData = AllocVec(cmd[0]+8, MEMF_PUBLIC);
   newmenu[num].nm_Label    = (type == 4) ? NM_BARLABEL
                                          : AllocVec(str[0]+8, MEMF_PUBLIC);
   if ((newmenu[num].nm_UserData == NULL) ||
       (newmenu[num].nm_Label    == NULL))
   {
      newmenu[num].nm_Type = 0;
      msg(ERROR_NO_MEMORY);
      return;
   }
   memcpy(newmenu[num].nm_UserData, cmd, cmd[0]+1);
   if (type == 4)
   {
      type = 2;
   }
   else
   {
      str[str[0]+1]=0; /* null terminate the string */
      memcpy(newmenu[num].nm_Label,str+1,str[0]+1);
   }
   newmenu[num].nm_Type = type;
   newmenu[num].nm_Flags = 0;
   newmenu[num].nm_CommKey = 0;
}

/***
*
* Free up all menu storage
*
***/
void free_menus()
{
   int i;

   if (menuattached)
   {
      if(window)ClearMenuStrip(window);
      menuattached = 0;
   }

   if (menu != NULL)
   {
      FreeMenus(menu);
      menu = NULL;
   }
   FreeVisualInfo(vi);
   for (i = 0; i < MAXMENU; i++)
   {
      if (newmenu[i].nm_UserData != NULL)
         FreeVec(newmenu[i].nm_UserData);
      if ((newmenu[i].nm_Label != NULL) &&
          (newmenu[i].nm_Label != NM_BARLABEL))
         FreeVec(newmenu[i].nm_Label);
      newmenu[i].nm_UserData = NULL;
      newmenu[i].nm_Label    = NULL;
   }
}
