��GRAPHICS_COPPER_H�GRAPHICS_COPPER_H�COPPER_MOVE 0�COPPER_WAIT 1�CPRNXTBUF 2�CPR_NT_LOF 0x8000�CPR_NT_SHT 0x4000
�CopIns
{
�OpCode;
�
{
�CopList*nxtlist;
�
{
�
{
�VWaitPos;
�DestAddr;
}u1;
�
{
�HWaitPos;
�DestData;
}u2;
}u4;
}u3;
};�NXTLIST u3.nxtlist�VWAITPOS u3.u4.u1.VWaitPos�DESTADDR u3.u4.u1.DestAddr�HWAITPOS u3.u4.u2.HWaitPos�DESTDATA u3.u4.u2.DestData
�cprlist
{
�cprlist*Next;
�*start;
�MaxCount;
};
�CopList
{
�CopList*Next;
�CopList*_CopList;
�ViewPort*_ViewPort;
�CopIns*CopIns;
�CopIns*CopPtr;
�*CopLStart;
�*CopSStart;
�Count;
�MaxCount;
�DyOffset;
};
�UCopList
{
�UCopList*Next;
�CopList*FirstCopList;
�CopList*CopList;
};
�copinit
{
�diagstrt[4];
�sprstrtup[(2*8*2)+2+(2*2)+2];
�sprstop[2];
};�