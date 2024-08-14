#include    "FixHeader.h"

#define FindNextTab(x) (((x/TABSIZE) + 1) * TABSIZE) + 1

unsigned int	ENTAB = 0;

int
entab(Inbuffer)
char *Inbuffer;
{
    char        Outbuffer[MAXINPUTLINELEN];
    static char InSQuote = 0, InDQuote = 0;
    static char InComment = 0;
    unsigned int LinePosition, InbufferPtr, OutbufferPtr, EndingLinePosition;
   
    if(!ENTAB)
	return 1;
    LinePosition = 1;
   
    if(strlen(Inbuffer))
    {
        OutbufferPtr = -1;
        
        for(InbufferPtr = 0; Inbuffer[InbufferPtr] != '\0'; InbufferPtr++)
        {
            switch(Inbuffer[InbufferPtr])
            {

                case '\\':
                    LinePosition++;
                    Outbuffer[++OutbufferPtr] = Inbuffer[InbufferPtr++];
                    Outbuffer[++OutbufferPtr] = Inbuffer[InbufferPtr];
                    if(Outbuffer[OutbufferPtr])
                    {
                        if(Outbuffer[OutbufferPtr] == '\t')
                            LinePosition = FindNextTab(LinePosition);
                        else
                            LinePosition++;
                    }
                    else
		    {
			strcpy(Inbuffer, Outbuffer);
                        return 1;
		    }
                    break;
                
                
                case '"':
                    Outbuffer[++OutbufferPtr] = Inbuffer[InbufferPtr];
                    if(!InComment && !InSQuote)
                        InDQuote = !InDQuote;
                    LinePosition++;
                    break;
                    
                case '\'':
                    Outbuffer[++OutbufferPtr] = Inbuffer[InbufferPtr];
                    if(!InComment && !InDQuote)
                        InSQuote = !InSQuote;
                    LinePosition++;
                    break;
                    
                case ' ':
                case '\t':
                    if(InSQuote || InDQuote)
                    {
                        Outbuffer[++OutbufferPtr] = Inbuffer[InbufferPtr];
                        if(Inbuffer[InbufferPtr] == '\t')
                            LinePosition = FindNextTab(LinePosition);
                        else
                            LinePosition++;
                    }
                    else
                    {
                        EndingLinePosition = LinePosition;
                        if(Inbuffer[InbufferPtr] == ' ')
                            EndingLinePosition++;
                        else
                            EndingLinePosition = FindNextTab(EndingLinePosition);
                        
                        while((Inbuffer[++InbufferPtr] == ' ' ) ||
                              (Inbuffer[InbufferPtr]   == '\t'))
                        {
                            if(Inbuffer[InbufferPtr] == ' ')
                                EndingLinePosition++;
                            else
                                EndingLinePosition = FindNextTab(EndingLinePosition);
                        };
                        if(Inbuffer[InbufferPtr] != '\0') 
                        {
                            if(EndingLinePosition > LinePosition+1)
				while(FindNextTab(LinePosition) <= EndingLinePosition)
                            	{
                                    Outbuffer[++OutbufferPtr] = '\t';
                                    LinePosition = FindNextTab(LinePosition);
                            	};
                            
                            while(LinePosition < EndingLinePosition)
                            {
                                Outbuffer[++OutbufferPtr] = ' ';
                                LinePosition++;
                            };
                        };
                            
                            InbufferPtr--;
                    };
                    break;
                
		case '*':
		    if((Inbuffer[InbufferPtr+1] == '/')	&&
			!InSQuote && !InDQuote && InComment)
			InComment = 0;
                    Outbuffer[++OutbufferPtr] = Inbuffer[InbufferPtr];
                    LinePosition++;
                    break;

		case '/':
		    if((Inbuffer[InbufferPtr+1] == '*')	&&
			!InSQuote && !InDQuote && !InComment)
			InComment = 1;
                    Outbuffer[++OutbufferPtr] = Inbuffer[InbufferPtr];
                    LinePosition++;
                    break;

                default:
                    Outbuffer[++OutbufferPtr] = Inbuffer[InbufferPtr];
                    LinePosition++;
                    break;
            };
        };
	Outbuffer[++OutbufferPtr] = '\0';
        strcpy(Inbuffer, Outbuffer);
	return 1;
    };
}
