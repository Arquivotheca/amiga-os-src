/**
*
* This module defines the error messages corresponding to the codes that
* can appear in _OSERR.  
*
*/

int os_nerr = 232;      /* Highest valid error number */

struct {
        short err_no;
        char *msg;
        } os_errlist[] = {      

        103,"Insufficient free store",
        105,"Task table full",
        120,"argument line invalid or too long",
        121,"File is not an object module",
        122,"Invalid resident library during load",
        201,"No default directory",
        202,"Object in use",
        203,"Object already exists",
        204,"Directory not found",
        205,"Object not found",
        206,"Invalid window",
        207,"Object too large",
        209,"Action not known",
        210,"Invalid stream component name",
        211,"Invalid lock",
        212,"Object not of required type",
        213,"Disk not validated",
        214,"Disk write protected",
        215,"Rename across devices attempted",
        216,"Directory not empty",
        217,"Too many levels",
        218,"Device not mounted",
        219,"Seek error",
        220,"Comment too big",
        221,"Disk full",
        222,"File is protected from deletion",
        223,"File is protected from writing",
        224,"File is protected from reading",
        225,"Not a DOS disk",
        226,"No disk in drive",
        209,"Packet request type unknown",
        211,"Invalid object lock",
        219,"Seek error",
        232,"No more entries in directory"
        };
