��GRAPHICS_GELS_H�GRAPHICS_GELS_H�SUSERFLAGS 255�VSPRITE 1�SAVEBACK 2�OVERLAY 4�MUSTDRAW 8�BACKSAVED 256�BOBUPDATE 512�GELGONE 0x400�VSOVERFLOW 0x800�BUSERFLAGS 255�SAVEBOB 1�BOBISCOMP 2�BWAITING 256�BDRAWN 512�BOBSAWAY 0x400�BOBNIX 0x800�SAVEPRESERVE 0x1000�OUTSTEP 0x2000�ANFRACSIZE 6�ANIMHALF 32�RINGTRIGGER 1�VUserStuff�VUserStuff���BUserStuff�BUserStuff���AUserStuff�AUserStuff��
�VSprite
{
�VSprite*NextVSprite;
�VSprite*PrevVSprite;
�VSprite*DrawPath;
�VSprite*ClearPath;
�OldY,OldX;
��;
�Y,X;
�Height;
�Width;
�Depth;
�MeMask;
�HitMask;
�*ImageData;
�*BorderLine;
�*CollMask;
�*SprColors;
�Bob*VSBob;
�PlanePick;
�PlaneOnOff;
VUserStuff VUserExt;
};
�Bob
{
��;
�*SaveBuffer;
�*ImageShadow;
�Bob*Before;
�Bob*After;
�VSprite*BobVSprite;
�AnimComp*BobComp;
�DBufPacket*DBuffer;
BUserStuff BUserExt;
};
�AnimComp
{
��;
�Timer;
�TimeSet;
�AnimComp*NextComp;
�AnimComp*PrevComp;
�AnimComp*NextSeq;
�AnimComp*PrevSeq;
�(*AnimCRoutine)();
�YTrans;
�XTrans;
�AnimOb*HeadOb;
�Bob*AnimBob;
};
�AnimOb
{
�AnimOb*NextOb,*PrevOb;
�Clock;
�AnOldY,AnOldX;
�AnY,AnX;
�YVel,XVel;
�YAccel,XAccel;
�RingYTrans,RingXTrans;
�(*AnimORoutine)();
�AnimComp*HeadComp;
AUserStuff AUserExt;
};
�DBufPacket
{
�BufY,BufX;
�VSprite*BufPath;
�*BufBuffer;
};�InitAnimate(animKey) {*(animKey)=�;}�RemBob(b) {(b)->�|=BOBSAWAY;}�B2NORM 0�B2SWAP 1�B2BOBBER 2
�collTable
{
�(*collPtrs[16])();
};�