* === AllocJRemember() ========================================================
* 
* NAME
*     AllocJRemember  --  Allocate Janus memory and link into a Remember list
* 
* SYNOPSIS
*     MemPtr = AllocJRemember(RememberKey, Size,   Types)
*     D0,A0                   A0           D0:0-15 D1:0-15
* 
*         APTR    MemPtr;
*         struct  JanusRemember **RememberKey;
*         USHORT  Size;
*         USHORT  Types;
*     From assembly, expects JanusBase in A6
* 
* 
* FUNCTION
*     This routine calls the Janus library's AllocJanusMem() call for you, 
* 
* 
* EXAMPLE
* INPUTS
*     Type = the type of the desired memory.  Please refer to the 
*         AllocJanusMem() documentation for details about this field
*     
*     
* RESULTS
* SEE ALSO
