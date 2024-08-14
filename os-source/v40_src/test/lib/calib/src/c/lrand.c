/****** CALib.library/LRand/--background-- ************************************

    NAME
        LRand -- Long Random number system

    VERSION
        1.00    16 Oct 1991 - Inception

    AUTHOR
        Fred Mitchell

    DESCRIPTION
        The LRand system employs a shift-register metaphor to
        create the random sequence. based on the number of
        bits in the shift-register, it is possible to acheive
        a period of (2 ** bits) if done properly. Since this is
        implemented as unsigned longwords, you can get a steady
        train of random values.

        The system is designed to be customizable by the user,
        but does have useful defaults.

******************************************************************************/

#include <exec/types.h>
#include <exec/memory.h>
#include <string.h>
#include <math.h>
#include <proto/exec.h>

struct LREnv {
    long NumRegs;  // Number of registers in shift-registers (should be a prime number)
    long Tap;       // Where to place tap
    ULONG Seed;     // Seed used to initialize LRand
    long Index;     // Current location in array
    ULONG SR[1];    // First entry in shift register
    };

/****** CALib.library/LRand/LROpen ********************************************

    NAME
        LROpen -- Set up the LR System

    SYNOPSIS
        struct LREnv *LROpen(ULONG Seed, ULONG NumRegs, ULONG Tap)

    FUNCTION
        Creates a LREnv handle for use of random number generation.
        If NumRegs is zero, uses internal defaults for NumRegs and
        Tap. Else both must be specified. NumRegs should be a prime
        number, and Tap should probably be 2 or 3 less than
        NumReg. You will have to experiment for best settings of
        NumRegs and Tap.

    INPUTS
        Seed    - Seed to use for LRand

        NumRegs - Either ZERO (0) for internal defaults, or a number > 1.
                  Note that NumRegs longwords will be allocated for
                  the shift-register.

        Tap     - Place to tap register for feedback. Good taps would
                  probably be around NumRegs - 3. Experimentation is
                  Necessary. Note that this is ignored when NumRegs == 0.

    RESULTS
        Returns a pointer to a private handle, or NULL.

    SEE ALSO
        LRClose().

******************************************************************************/

struct LREnv *LROpen(ULONG Seed, ULONG NumRegs, ULONG Tap)
{
    struct LREnv *lr;

    if (!NumRegs)
    {
        NumRegs = 11;
        Tap = 3;
    }

    if (lr = AllocVec(sizeof(*lr) + NumRegs * sizeof(lr->SR), MEMF_CLEAR|MEMF_ANY))
    {
        long i;

        lr->NumRegs = NumRegs;
        lr->Tap = Tap;
        srand(lr->Seed = Seed);
        lr->Index = 0;

        for (i = 0; i < NumRegs; ++i)
            lr->SR[i] = rand() + 0xA2796591; // WARN: Relies on lattice's rand() to initialize!!!
    }

    return lr;
}

/****** CALib.library/LRand/LRClose *******************************************

    NAME
        LRClose -- Deallocate resources used for LRand

    SYNOPSIS
        void LRClose(struct LREnv *lr)

    FUNCTION
        Frees up and deallocates any resources associated with LR.

    INPUTS
        lr  - The handle returned by LROpen(). NULL handles are allowed.

    RESULTS
        Returns nothing

    SEE ALSO
        LROpen().

******************************************************************************/

void LRClose(struct LREnv *lr)
{
    if (lr)
        FreeVec(lr);
}

/****** CALib.library/LRand/LRand *********************************************

    NAME
        LRand -- Generate unsigned longword random number

    SYNOPSIS
        ULONG LRand(struct LREnv *lr)

    FUNCTION
        Generates a unsigned longword random number based on the state of
        the hande.

    INPUTS
        lr  - handle created by LROpen().

    RESULTS
        Returns an unsigned longword random number, to the full range
        of the longword (0 to 2**lwbits - 1).

    SEE ALSO
        LROpen().

******************************************************************************/

ULONG LRand(struct LREnv *lr)
{
    long Tap = (lr->Index + lr->Tap + lr->NumRegs) % lr->NumRegs;
    long Index = lr->Index;

    lr->SR[Index] += lr->SR[Tap];
    if (++lr->Index >= lr->NumRegs)
        lr->Index = 0;

    return lr->SR[Index];
}
