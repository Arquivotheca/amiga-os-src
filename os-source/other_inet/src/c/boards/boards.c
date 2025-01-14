/* -----------------------------------------------------------------------
 * BOARDS.C  (manx 36)
 *
 * $Locker:  $
 *
 * $Id: boards.c,v 1.1 90/11/12 15:11:16 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: Hog:Other/inet/src/c/boards/RCS/boards.c,v 1.1 90/11/12 15:11:16 bj Exp $
 *
 *------------------------------------------------------------------------
 */

/*
** Dump statistics about boards hanging around in the system.
*/

#include <exec/types.h>
#include <libraries/configvars.h>
#include <ameristar.h>
#include <cbm.h>

#include "boards_rev.h"
static char *compiler = "MANX36 MANX36" ;
static char *vers = VERSTAG ;

struct {
	int	mfg;
	int	prod;
	char	*name;
} prods[] = {
	{AMERISTAR, AMER_ENET1,	"Ameristar AE-2000-100 Ethernet board"},
	{AMERISTAR, AMER_AEM500,"Ameristar AEM-500 Ethernet board"},
	{COMMODORE, CBM_ENET1,  "Commodore Ethernet controller board"},
	{COMMODORE, CBM_ARCNET,	"Commodore Arcnet board"},
	{AMERISTAR, AMER_ARCNET1,"Ameristar Arcnet board"},
	0, 0
};

struct {
	char	*name;
	int	mfg;
} mfgrs[] = {
	{"Ameristar",	AMERISTAR},
	{"Commdore",	COMMODORE}
};

long ExpansionBase;

main()
{
	static char exlib[] = "expansion.library";
	struct ConfigDev *cd;
	char *mfg, buf[64];
	int i, mfgnum;

	if(!(ExpansionBase = OpenLibrary(exlib, 0))){
		printf("Could not open %s;  are you running 1.2??\n", exlib);
		exit(1);
	}

	for(cd = 0; (cd = FindConfigDev(cd, -1, -1));){
		mfgnum = cd->cd_Rom.er_Manufacturer;
		for(i = 0; prods[i].name != 0; i++){
			if(prods[i].mfg == mfgnum &&
				prods[i].prod == cd->cd_Rom.er_Product){
				mfg = prods[i].name;
				break;
			}
		}
		if(!prods[i].name){
			for(i = 0; mfgrs[i].name; i++){
				if(mfgrs[i].mfg == mfgnum){
					sprintf(buf, "%s prod %d", mfgrs[i].name, 
						cd->cd_Rom.er_Product);
					break;
				}
			}
			if(!mfgrs[i].name){
				sprintf(buf, "Mfg %d prod %d", mfgnum, cd->cd_Rom.er_Product);
			}
			mfg = buf;
		}
		printf("%s at addr %06x; serial number %08x\n",
			mfg, cd->cd_BoardAddr, cd->cd_Rom.er_SerialNumber);
	}
	CloseLibrary(ExpansionBase); 
}

