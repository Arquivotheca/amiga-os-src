default_hunkname(hunktype)
word hunktype;
{
	word i;
	word namesize = 2;
	char *dale;
	word type;
	word memtype;

	hunkname = F0023_vec_get(namesize);

	dale = (char *)hunkname;
    *dale++ = 'r';
    *dale++ = 'o';
    *dale++ = 'm';
    *dale++ = 'h';
    *dale++ = 'u';
    *dale++ = 'n';
    *dale++ = 'k';
    *dale++ = 0;

	memtype = hunktype & memmask;
	myresult2 = hunktype;

	if (memtype == memmask) return t_err ;
	switch (hunktype & typemask)
	{
		case hunk_bss:
			type = t_bss | memtype;
			break;
		case hunk_code:
			type = t_code | memtype;
			break;
		case hunk_data:
			type = t_data | memtype;
			break;
		case hunk_res:
			type = t_res;
			break;
		default:
			type = t_err;
			break;
	}

	if ( type == t_err )
      return no;
   {
      word* s = F0025_getblk(size_hname);
      s[ hname_len ] = namesize;
      s[ hname_ptr ] = (word)hunkname;
      s[ hname_type ] = type;
      s[ hname_zero ] = 0L;
      {  
         word* t = F0012_lookup(s, type);
         if ( t == 0L )
         {
            /*  new name */
            s[ hname_base ] = 0L;
            /*           hname.parent ! s            is filled in later */
            s[ hname_zero ] = 0L;
            s[ hname_size ] = 0L;
            s[ hname_link ] = 0L;
            F0013_insert ( s );   /*  add to symbol table */
            hunkname = s;
         }
         else
         {
            /*  old hunkname */
            F0026_freesymbol ( s );   /*  free space */
            hunkname = t;   /*  ptr to hunkname */
            conthunk = yes;
         }
      }   
	}
   return type == t_res ? type : hunktype;
}
