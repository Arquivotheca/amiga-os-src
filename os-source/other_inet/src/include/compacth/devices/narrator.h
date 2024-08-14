€ˆDEVICES_NARRATOR_H€DEVICES_NARRATOR_HˆºŒ"exec/io.h"‡€ND_NoMem -2€ND_NoAudLib -3€ND_MakeBad -4€ND_UnitErr -5€ND_CantAlloc -6€ND_Unimpl -7€ND_NoWrite -8€ND_Expunged -9€ND_PhonErr -20€ND_RateErr -21€ND_PitchErr -22€ND_SexErr -23€ND_ModeErr -24€ND_FreqErr -25€ND_VolErr -26€DEFPITCH 110€DEFRATE 150€DEFVOL 64€DEFFREQ 22200€MALE 0€FEMALE 1€NATURALF0 0€ROBOTICF0 1€DEFSEX MALE€DEFMODE NATURALF0€MINRATE 40€MAXRATE 400€MINPITCH 65€MAXPITCH 320€MINFREQ 5000€MAXFREQ 28000€MINVOL 0€MAXVOL 64
ƒnarrator_rb{
ƒIOStdReq message;
‰rate;
‰pitch;
‰mode;
‰sex;
Š*ch_masks;
‰nm_masks;
‰volume;
‰sampfreq;
Šmouths;
Šchanmask;
Šnumchan;
Špad;
};
ƒmouth_rb{
ƒnarrator_rb voice;
Šwidth;
Šheight;
Šshape;
Špad;
};‡