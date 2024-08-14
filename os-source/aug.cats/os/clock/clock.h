#ifndef CLOCK_H
#define CLOCK_H


/*****************************************************************************/


/* clock types */
#define ANALOG		0
#define DIGITAL		1


/*****************************************************************************/


enum ClockCommand
{
    cc_NOP,
    cc_Analog,
    cc_Digital,
    cc_Quit,
    cc_Seconds,
    cc_Date,
    cc_DateFmt1,
    cc_DateFmt2,
    cc_DateFmt3,
    cc_DateFmt4,
    cc_DateFmt5,
    cc_DateFmt6,
    cc_SetAlarm,
    cc_Alarm,
    cc_SaveSettings
};


/*****************************************************************************/


#endif /* CLOCK_H */
