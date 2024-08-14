/*------------------------------------------
*  PARSER routine
*
*/


/********* EXAMPLE CALLING CODE FOR PARSER ******************
 *  parse( str, seperators ) will return a pointer the the 
 *  NULL TERMINATED 'next word' in the string 'str' using the
 *  characters in the 'seperators' array to define 'word'
 *  boundaries. The global pointer 'next_token' points to the
 * remainder of the string.
 * 
 *
 *  main()  
 *  {
 *
 *  TEXT *tokep ;
 *
 *  tokep = strtok( s, seperators ) ;
 *  if( tokep ) {
 *      printf( "Token   = %s\n", tokep ) ;
 *      if( next_token )
 *         printf( "Next    = %s\n\n", next_token ) ;
 *      }
 *  }
 *
 *********************/

 
TEXT * 
parse(buf, separators)
TEXT *buf, *separators ; 
{
    register TEXT *token, *end, *tmp ;     /* Start and end of token. */
    extern   TEXT *strpbrk() ; 
    static   TEXT *fromLastTime ; 

    if (token = buf ? buf : fromLastTime) 
        {
        token += strspn(token, separators) ;     /* Find token! */ 
        if (*token == STRING_END) 
            {
            return(NULL) ; 
            }
        tmp = ((end = strpbrk(token,separators))) ? &end[1] : NULL ; 
        while( tmp && (*tmp == ' ') )
            {
            tmp++ ;
            }
        fromLastTime = tmp ? tmp : NULL ;
        next_token = fromLastTime ;
        *end = STRING_END ;                      /* Cut it short! */ 
        } 
    return(token) ; 
} 


/*---------------------------------------------*/

/* Return the number of characters from "charset" that are at the BEGINNING 
 * of string "str". 
*/ 
 
SHORT
strspn(str, charset) 
register TEXT *str, *charset ; 
{ 
   REGISTER TEXT *s ; 

     s = str ; 
     while (index(charset, *s)) 
         {
         s++ ; 
         }
     return(s - str) ; 
} 


TEXT *strpbrk(str, charset) 
TEXT *str, *charset ; 
{ 
        
   REGISTER TEXT *s ; 

     s = str ; 
     while ((*s != STRING_END) && (!index(charset, *s))) 
         {
         s++ ; 
         }
     return((*s!=STRING_END) ? s : NULL) ; 
} 

