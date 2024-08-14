#include <exec/types.h>
#include <exec/lists.h>
#include <graphics/gfxbase.h>
#include <graphics/displayinfo.h>
#include <internal/displayinfo_internal.h>
#include <internal/vp_internal.h>
#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <dos/dos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/utility_protos.h>
#include <stdio.h>
#include "db.h"
#include "db_tm.h"

#define NOADDR 0x8000
extern struct GfxBase *GfxBase;

void PrintList(FILE * file, struct List *l, UWORD mode)
{
	if (!(mode & NOADDR)) 
	{
		fprintf(file, "\t\tList\n\t\t{\n");
		fprintf(file, "\t\tHead = 0x%lx\n", l->lh_Head);
		fprintf(file, "\t\tTail = 0x%lx\n", l->lh_Tail);
		fprintf(file, "\t\tPred = 0x%lx\n", l->lh_TailPred);
		fprintf(file, "\t\tType = 0x%lx\n", l->lh_Type);
		fprintf(file, "\t\t}\n");
	}
}

void PrintQuery(FILE * file, struct QueryHeader *q)
{
	fprintf(file, "\nQuery Header -\n");
	fprintf(file, "\tStructID  = %lx\n", q->StructID);
	fprintf(file, "\tDisplayID = %lx\n", q->DisplayID);
	fprintf(file, "\tSkipID    = %lx\n", q->SkipID);
	fprintf(file, "\tLength    = %lx\n", q->StructID);
}

void PrintSpecMon(FILE * file, struct SpecialMonitor *m, struct MonitorSpec *mon, UWORD mode)
{
	fprintf(file, "\tSpecialMonitor\n\t{\n");
	fprintf(file, "\t\tFlags = 0x%lx\n", m->spm_Flags);
	fprintf(file, "\t\tdo_monitor = 0x%lx\n", (mode & NOADDR ? 0 : m->do_monitor));
	fprintf(file, "\t\treserved1 = 0x%lx\n", m->reserved1);
	fprintf(file, "\t\treserved2 = 0x%lx\n", m->reserved2);
	fprintf(file, "\t\treserved3 = 0x%lx\n", m->reserved3);
	fprintf(file, "\t\thblank = (0x%lx, 0x%lx)\n", m->hblank.asi_Start, m->hblank.asi_Stop);
	fprintf(file, "\t\tvblank = (0x%lx, 0x%lx) = lines (0x%lx, 0x%lx)\n", m->vblank.asi_Start, m->vblank.asi_Stop, (m->vblank.asi_Start / mon->total_colorclocks), (m->vblank.asi_Stop / mon->total_colorclocks));
	fprintf(file, "\t\thsync =  (0x%lx, 0x%lx)\n", m->hsync.asi_Start, m->hsync.asi_Stop);
	fprintf(file, "\t\tvsync =  (0x%lx, 0x%lx) = lines (0x%lx, 0x%lx)\n", m->vsync.asi_Start, m->vsync.asi_Stop, (m->vsync.asi_Start / mon->total_colorclocks), (m->vsync.asi_Stop / mon->total_colorclocks));
	fprintf(file, "\t}\n");
}

void PrintMSpec(FILE * file, struct MonitorSpec *m, UWORD mode)
{

	fprintf(file, "\nMonitorSpec - (at %lx)\n", (mode & NOADDR ? 0 : m));
	fprintf(file, "{\n");
	fprintf(file, "\tName: %s\n", m->ms_Node.xln_Name);
	fprintf(file, "\tFlags     = %lx\n\t\t", m->ms_Flags);
	if (m->ms_Flags & REQUEST_NTSC)
		fprintf(file, "REQUEST_NTSC ");
	if (m->ms_Flags & REQUEST_PAL)
		fprintf(file, "REQUEST_PAL ");
	if (m->ms_Flags & REQUEST_SPECIAL)
		fprintf(file, "REQUEST_SPECIAL ");
	if (m->ms_Flags & REQUEST_A2024)
		fprintf(file, "REQUEST_A2024 ");
	if (m->ms_Flags & MSF_DOUBLE_SPRITES)
		fprintf(file, "DOUBLE_SPRITES ");
	fprintf(file, "\n");

	fprintf(file, "\tratioh      = %lx = %ld\n", m->ratioh, m->ratioh);
	fprintf(file, "\tratiov      = %lx = %ld\n", m->ratiov, m->ratiov);
	fprintf(file, "\ttotal_rows  = %lx = %ld\n", m->total_rows, m->total_rows);
	fprintf(file, "\ttotal_colorclocks\n");
	fprintf(file, "\t            = %lx = %ld\n", m->total_colorclocks, m->total_colorclocks);
	fprintf(file, "\tDeniseMaxDisplayColumn\n");
	fprintf(file, "\t            = %lx = %ld\n", m->DeniseMaxDisplayColumn, m->DeniseMaxDisplayColumn);
	fprintf(file, "\tBeamCon0    = %lx = %ld\n", m->BeamCon0, m->BeamCon0);
	fprintf(file, "\tmin_row     = %lx = %ld\n", m->min_row, m->min_row);
	if (m->ms_Special)
	{
		PrintSpecMon(file, m->ms_Special, m, mode);
	}
	fprintf(file, "\tms_OpenCount= %ld\n", (mode & NOADDR ? 0 : m->ms_OpenCount));
	fprintf(file, "\tms_transform= %lx\n", (mode & NOADDR ? 0 : m->ms_transform));
	fprintf(file, "\tms_translate= %lx\n", (mode & NOADDR ? 0 : m->ms_translate));
	fprintf(file, "\tms_scale    = %lx\n", (mode & NOADDR ? 0 : m->ms_scale));
	fprintf(file, "\tms_xoffset  = %lx = %ld\n", m->ms_xoffset, m->ms_xoffset);
	fprintf(file, "\tms_yoffset  = %lx = %ld\n", m->ms_yoffset, m->ms_yoffset);
	fprintf(file, "\tms_LegalView Rectangle =\n");
	fprintf(file, "\t\t(0x%lx, 0x%lx) - (0x%lx, 0x%lx) =\n", m->ms_LegalView.MinX, m->ms_LegalView.MinY, m->ms_LegalView.MaxX, m->ms_LegalView.MaxY);
	fprintf(file, "\t\t(%ld, %ld) - (%ld, %ld)\n", m->ms_LegalView.MinX, m->ms_LegalView.MinY, m->ms_LegalView.MaxX, m->ms_LegalView.MaxY);
	fprintf(file, "\tms_maxoscan= %lx\n", (mode & NOADDR ? 0 : m->ms_maxoscan));
	fprintf(file, "\tms_videoscan= %lx\n", (mode & NOADDR ? 0 : m->ms_videoscan));
	fprintf(file, "\tDeniseMinDisplayColumn\n");
	fprintf(file, "\t            = %lx = %ld\n", m->DeniseMinDisplayColumn, m->DeniseMinDisplayColumn);
	fprintf(file, "\tDisplayCompatible\n");
	fprintf(file, "\t            = %lx\n", m->DisplayCompatible);
	fprintf(file, "\tDataBase at   %lx\n", (mode & NOADDR ? 0 : &m->DisplayInfoDataBase));
	PrintList(file, &m->DisplayInfoDataBase, mode);
	fprintf(file, "\tMrgCop      = %lx\n", (mode & NOADDR ? 0 : m->ms_MrgCop));
	fprintf(file, "\tLoadView    = %lx\n", (mode & NOADDR ? 0 : m->ms_LoadView));
	fprintf(file, "\tKillView    = %lx\n", (mode & NOADDR ? 0 : m->ms_KillView));

	fprintf(file, "}\n");
}

void PrintDisp(FILE * file, struct DisplayInfo *d, UWORD mode)
{
	UWORD t1;
	ULONG t2;

	fprintf(file, "\nDisplayInfo\n{\n");
	PrintQuery(file, &d->Header);
	fprintf(file, "NotAvailable  = 0x%lx\n\t", (t1 = d->NotAvailable));
	if (t1 & DI_AVAIL_NOCHIPS)
		fprintf(file, "NOCHIPS ");
	if (t1 & DI_AVAIL_NOMONITOR)
		fprintf(file, "NOMONITOR" );
	if (t1 & DI_AVAIL_NOTWITHGENLOCK)
		fprintf(file, "NOTWITHGENLOCK");
	fprintf(file, "\n");

	fprintf(file, "PropertyFlags = 0x%lx\n\t",(t2 = d->PropertyFlags));
	if (t2 & DIPF_IS_LACE)
		fprintf(file, "LACE ");
	if (t2 & DIPF_IS_DUALPF)
		fprintf(file, "DUALPF ");
	if (t2 & DIPF_IS_PF2PRI)
		fprintf(file, "PF2PRI ");
	if (t2 & DIPF_IS_HAM)
		fprintf(file, "HAM ");
	if (t2 & DIPF_IS_ECS)
		fprintf(file, "ECS ");
	if (t2 & DIPF_IS_AA)
		fprintf(file, "AA ");
	if (t2 & DIPF_IS_PAL)
		fprintf(file, "PAL ");
	if (t2 & DIPF_IS_SPRITES)
		fprintf(file, "SPRITES ");
	if (t2 & DIPF_IS_GENLOCK)
		fprintf(file, "GENLOCK ");
	if (t2 & DIPF_IS_WB)
		fprintf(file, "WB ");
	if (t2 & DIPF_IS_DRAGGABLE)
		fprintf(file, "DRAGGABLE ");
	if (t2 & DIPF_IS_PANELLED)
		fprintf(file, "PANELLED ");
	if (t2 & DIPF_IS_BEAMSYNC)
		fprintf(file, "BEAMSYNC ");
	if (t2 & DIPF_IS_EXTRAHALFBRITE)
		fprintf(file, "EXTRAHALFBRITE ");
	if (t2 & DIPF_IS_SPRITES_ATT)
		fprintf(file, "SPRITES_ATTACHED ");
	if (t2 & DIPF_IS_SPRITES_CHNG_RES)
		fprintf(file, "SPRITES_CHNG_RES ");
	if (t2 & DIPF_IS_SPRITES_BORDER)
		fprintf(file, "SPRITES_BORDER ");
	if (t2 & DIPF_IS_SCANDBL)
		fprintf(file, "SCANDBL ");
	if (t2 & DIPF_IS_SPRITES_CHNG_BASE)
		fprintf(file, "SPRITES_CHNG_BASE ");
	if (t2 & DIPF_IS_SPRITES_CHNG_PRI)
		fprintf(file, "SPRITES_CHNG_PRI ");
	if (t2 & DIPF_IS_FOREIGN)
		fprintf(file, "FOREIGN ");
	if (t2 & DIPF_IS_DBUFFER)
		fprintf(file, "DBUFFER ");
	if (t2 & DIPF_IS_PROGBEAM)
		fprintf(file, "PROGBEAM ");
	fprintf(file, "\n");

	fprintf(file, "Resolution    = (0x%lx, 0x%lx) = (%ld, %ld) (ticks per pixel)\n", d->Resolution.x, d->Resolution.y, d->Resolution.x, d->Resolution.y);
	fprintf(file, "PixelSpeed    = 0x%lxns = %ldns\n", d->PixelSpeed, d->PixelSpeed);
	fprintf(file, "NumStdSprites = %lx\n", d->NumStdSprites);
	fprintf(file, "PaletteRange  = 0x%lx = %ld\n", d->PaletteRange, d->PaletteRange);
	fprintf(file, "SpriteRestion = (0x%lx, 0x%lx) = (%ld, %ld) (sprite ticks per pixel)\n", d->SpriteResolution.x, d->SpriteResolution.y, d->SpriteResolution.x, d->SpriteResolution.y);
	if (GfxBase->LibNode.lib_Version >= 38)
		fprintf(file, "RGB Bits      = (0x%lx, 0x%lx, 0x%lx)\n", d->RedBits, d->GreenBits, d->BlueBits);
	fprintf(file, "}\n");
}

void PrintDims(FILE * file, struct DimensionInfo *d, UWORD mode)
{

	fprintf(file, "\nDimensionInfo\n{\n");
	PrintQuery(file, &d->Header);

	fprintf(file, "MaxDepth = 0x%lx\n",d->MaxDepth);
	fprintf(file, "MinRasterWidth = 0x%lx = %ld pixels\n", d->MinRasterWidth, d->MinRasterWidth);
	fprintf(file, "MinRasterHeight = 0x%lx = %ld pixels\n", d->MinRasterHeight, d->MinRasterHeight);
	fprintf(file, "MaxRasterWidth = 0x%lx = %ld pixels\n", d->MaxRasterWidth, d->MaxRasterWidth);
	fprintf(file, "MaxRasterHeight = 0x%lx = %ld pixels\n", d->MaxRasterHeight, d->MaxRasterHeight);

	fprintf(file, "Nominal Rectangle (Standard Dimensions) =\n");
	fprintf(file, "\t(0x%lx, 0x%lx) - (0x%lx, 0x%lx) =\n", d->Nominal.MinX, d->Nominal.MinY, d->Nominal.MaxX, d->Nominal.MaxY);
	fprintf(file, "\t(%ld, %ld) - (%ld, %ld)\n", d->Nominal.MinX, d->Nominal.MinY, d->Nominal.MaxX, d->Nominal.MaxY);
	fprintf(file, "MaxOScan Rectangle (fixed, hardware dependant) =\n");
	fprintf(file, "\t(0x%lx, 0x%lx) - (0x%lx, 0x%lx) =\n", d->MaxOScan.MinX, d->MaxOScan.MinY, d->MaxOScan.MaxX, d->MaxOScan.MaxY);
	fprintf(file, "\t(%ld, %ld) - (%ld, %ld)\n", d->MaxOScan.MinX, d->MaxOScan.MinY, d->MaxOScan.MaxX, d->MaxOScan.MaxY);
	fprintf(file, "VideoOScan Rectangle (fixed, hardware dependant) =\n");
	fprintf(file, "\t(0x%lx, 0x%lx) - (0x%lx, 0x%lx) =\n", d->VideoOScan.MinX, d->VideoOScan.MinY, d->VideoOScan.MaxX, d->VideoOScan.MaxY);
	fprintf(file, "\t(%ld, %ld) - (%ld, %ld)\n", d->VideoOScan.MinX, d->VideoOScan.MinY, d->VideoOScan.MaxX, d->VideoOScan.MaxY);
	fprintf(file, "TxtOScan Rectangle (Editable via Preferences) =\n");
	fprintf(file, "\t(0x%lx, 0x%lx) - (0x%lx, 0x%lx) =\n", d->TxtOScan.MinX, d->TxtOScan.MinY, d->TxtOScan.MaxX, d->TxtOScan.MaxY);
	fprintf(file, "\t(%ld, %ld) - (%ld, %ld)\n", d->TxtOScan.MinX, d->TxtOScan.MinY, d->TxtOScan.MaxX, d->TxtOScan.MaxY);
	fprintf(file, "StdOScan Rectangle (Editable via Preferences) =\n");
	fprintf(file, "\t(0x%lx, 0x%lx) - (0x%lx, 0x%lx) =\n", d->StdOScan.MinX, d->StdOScan.MinY, d->StdOScan.MaxX, d->StdOScan.MaxY);
	fprintf(file, "\t(%ld, %ld) - (%ld, %ld)\n", d->StdOScan.MinX, d->StdOScan.MinY, d->StdOScan.MaxX, d->StdOScan.MaxY);

	fprintf(file, "}\n");
}

void PrintMon(FILE * file, struct MonitorInfo *m, UWORD mode)
{

	fprintf(file, "\nMonitorInfo\n{\n");
	PrintQuery(file, &m->Header);
	if (m->Mspc)
		PrintMSpec(file, m->Mspc, mode);

	fprintf(file, "ViewPosition     = (0x%lx, 0x%lx) = (%ld, %ld)\n", m->ViewPosition.x, m->ViewPosition.y, m->ViewPosition.x, m->ViewPosition.y);
	fprintf(file, "ViewResolution   = (0x%lx, 0x%lx) = (%ld, %ld) (ticks per pixel)\n", m->ViewResolution.x, m->ViewResolution.y, m->ViewResolution.x, m->ViewResolution.y);
	fprintf(file, "ViewPositionRange Rectangle (fixed, hardware dependant) =\n");
	fprintf(file, "\t(0x%lx, 0x%lx) - (0x%lx, 0x%lx) =\n", m->ViewPositionRange.MinX, m->ViewPositionRange.MinY, m->ViewPositionRange.MaxX, m->ViewPositionRange.MaxY);
	fprintf(file, "\t(%ld, %ld) - (%ld, %ld)\n", m->ViewPositionRange.MinX, m->ViewPositionRange.MinY, m->ViewPositionRange.MaxX, m->ViewPositionRange.MaxY);
	fprintf(file, "TotalRows        = 0x%lx = %ld\n", m->TotalRows, m->TotalRows);
	fprintf(file, "TotalColorClocks = 0x%lx = %ld\n", m->TotalColorClocks, m->TotalColorClocks);
	fprintf(file, "MinRow           = 0x%lx = %ld\n", m->MinRow, m->MinRow);
	fprintf(file, "Comatibility     = 0x%lx = ", m->Compatibility);
	if (m->Compatibility == MCOMPAT_MIXED)
		fprintf(file, "MCOMPAT_MIXED\n");
	if (m->Compatibility == MCOMPAT_SELF)
		fprintf(file, "MCOMPAT_SELF\n");
	if (m->Compatibility == MCOMPAT_NOBODY)
		fprintf(file, "MCOMPAT_NOBODY\n");
	fprintf(file, "MouseTicks       = (0x%lx, 0x%lx) = (%ld, %ld)\n", m->MouseTicks.x, m->MouseTicks.y, m->MouseTicks.x, m->MouseTicks.y);
	fprintf(file, "DefaultViewPosn  = (0x%lx, 0x%lx) = (%ld, %ld)\n", m->DefaultViewPosition.x, m->DefaultViewPosition.y, m->DefaultViewPosition.x, m->DefaultViewPosition.y);
	fprintf(file, "PreferredModeID  = 0x%lx\n", m->PreferredModeID);

	fprintf(file, "}\n");
}

void PrintProg(FILE * file, struct ProgInfo *p, UWORD mode)
{

	fprintf(file, "\nProgInfo\n{\n");

	fprintf(file, "bplcon0     = 0x%lx\n", p->bplcon0);
	fprintf(file, "bplcon2     = 0x%lx\n", p->bplcon2);
	fprintf(file, "ToViewX     = 0x%lx\n", p->ToViewX);
	fprintf(file, "Flags       = 0x%lx\n", p->Flags);
	fprintf(file, "MakeItType  = 0x%lx\n", p->MakeItType);
	fprintf(file, "ScrollVPCount = 0x%lx\n", p->ScrollVPCount);
	fprintf(file, "DDFSTRTMask = 0x%lx\n", p->DDFSTRTMask);
	fprintf(file, "DDFSTOPMask = 0x%lx\n", p->DDFSTOPMask);
	fprintf(file, "ToDIWResn   = 0x%lx\n", p->ToDIWResn);
	fprintf(file, "Offset      = 0x%lx\n", p->Offset);

	fprintf(file, "}\n");
}

void PrintVec(FILE * file, struct VecInfo *v, UWORD mode)
{

	fprintf(file, "\nVecInfo\n{\n");
	PrintQuery(file, &v->Header);

	fprintf(file, "Vec   = 0x%lx\n", (mode & NOADDR ? 0 : v->Vec));
	fprintf(file, "Data  = 0x%lx\n", (mode & NOADDR ? 0 : v->Data));
	fprintf(file, "Type  = 0x%lx\n", v->Type);
	
	PrintProg(file, (struct ProgInfo *)v->Data, mode);
}


void PrintRec(FILE * file, struct DisplayInfoRecord *rec, UWORD mode)
{

	fprintf(file, "\nDisplayInfoRecord (private!!) at 0x%lx\n{\n", (mode & NOADDR ? 0 : rec));
	fprintf(file, "MajorKey = 0x%lx\n", rec->rec_MajorKey);
	fprintf(file, "MinorKey = 0x%lx\n", rec->rec_MinorKey);
	fprintf(file, "Control  = 0x%lx (= %ld)\n", rec->rec_Control, rec->rec_Control);
	fprintf(file, "get_data = 0x%lx\n", rec->rec_get_data);
	fprintf(file, "set_data = 0x%lx\n", rec->rec_set_data);
	fprintf(file, "ClipOScan Rectangle =\n");
	fprintf(file, "\t(0x%lx, 0x%lx) - (0x%lx, 0x%lx) =\n", rec->rec_ClipOScan.MinX, rec->rec_ClipOScan.MinY, rec->rec_ClipOScan.MaxX, rec->rec_ClipOScan.MaxY);
	fprintf(file, "\t(%ld, %ld) - (%ld, %ld)\n", rec->rec_ClipOScan.MinX, rec->rec_ClipOScan.MinY, rec->rec_ClipOScan.MaxX, rec->rec_ClipOScan.MaxY);

	fprintf(file, "}\n");
}

BOOL ConvToNum(STRPTR hexString, ULONG *value)
{
char  c;
SHORT i;


    i      = 2;
    *value = 0;
    for (;;)
    {
        c = ToUpper(hexString[i++]);
        if ((c >= '0') && (c <= '9'))
        {
            *value = *value*16 + c - '0';
        }
        else if ((c >= 'A') && (c <= 'F'))
        {
            *value = *value*16 + c - 'A' + 10;
        }
        else if (c != NULL)
        {
            return(FALSE);
        }
        else
        {
            return(TRUE);
        }
    }
}

void PrintDB(struct TMData *TMData)
{
    ULONG ModeID;
    struct DisplayInfoRecord *handle;
    FILE *con;
    STRPTR string;
    UBYTE conname[256];
    UWORD mode = 0;
    ULONG _name, _mon, _disp, _dims, _vec;
    struct DisplayInfo disp;
    struct DimensionInfo dims;
    struct MonitorInfo mon;
    struct NameInfo name;
    struct VecInfo vec;
    int i;

    /* What ModeID is selected? */
    GT_GetGadgetAttrs(gadget_MODEID, window_GFXDATAB, NULL,
                      GTST_String, (ULONG)&string,
                      TAG_DONE);
    ConvToNum(string, &ModeID);
    /* What do we need to print? */
    GT_GetGadgetAttrs(gadget_NAME, window_GFXDATAB, NULL,
                      GTCB_Checked, &_name,
                      TAG_DONE);
    GT_GetGadgetAttrs(gadget_MONITOR, window_GFXDATAB, NULL,
                      GTCB_Checked, &_mon,
                      TAG_DONE);
    GT_GetGadgetAttrs(gadget_DISPLAY, window_GFXDATAB, NULL,
                      GTCB_Checked, &_disp,
                      TAG_DONE);
    GT_GetGadgetAttrs(gadget_DIMENSIO, window_GFXDATAB, NULL,
                      GTCB_Checked, &_dims,
                      TAG_DONE);
    GT_GetGadgetAttrs(gadget_VECTOR, window_GFXDATAB, NULL,
                      GTCB_Checked, &_vec,
                      TAG_DONE);

    sprintf(conname, "con:0/10/640/200/DB OutPut %s %s%s%s%s%s/WAIT", string, (_name ? "N" : "n"), (_mon ? "O" : "o"), (_dims ? "D" : "d"), (_disp ? "I" : "i"), (_vec ? "V" : "v"));

    if (con = fopen(conname, "wr"))
    {
	handle = (struct DisplayInfoRecord *)FindDisplayInfo(ModeID);

	if (handle)
	{
		if (_name)
		{
			_name = GetDisplayInfoData(handle, (APTR)&name, sizeof(struct NameInfo), DTAG_NAME, ModeID);
		}
	
		if (_mon)
		{
			_mon = GetDisplayInfoData(handle, (APTR)&mon, sizeof(struct MonitorInfo), DTAG_MNTR, ModeID);
		}
	
		if (_disp)
		{
			_disp = GetDisplayInfoData(handle, (APTR)&disp, sizeof(struct DisplayInfo), DTAG_DISP, ModeID);
		}
	
		if (_dims)
		{
			_dims = GetDisplayInfoData(handle, (APTR)&dims, sizeof(struct DimensionInfo), DTAG_DIMS, ModeID);
		}
	
		if (_vec)
		{
			_vec = GetDisplayInfoData(handle, (APTR)&vec, sizeof(struct VecInfo), DTAG_VEC, ModeID);
		}
	
		fprintf(con, "\n*** Mode 0x%08lx ***\n", ModeID);
		fprintf(con, "Name : ");
		if (_name)
		{
			fprintf(con, "%s\n", name.Name);
		}
		else
		{
			fprintf(con, "\n");
		}
	
		PrintRec(con, handle, mode);
		if (_mon)
		{
			PrintMon(con, &mon, mode);
		}
		if (_dims)
		{
			PrintDims(con, &dims, mode);
		}
		if (_disp)
		{
			PrintDisp(con, &disp, mode);
		}
		if (_vec)
		{
			PrintVec(con, &vec, mode);
		}
	}
	else
	{
		fprintf(con, "Mode 0x%08lx does not exist\n", ModeID);
	}
	fflush(con);
	fclose(con);
    }
}
