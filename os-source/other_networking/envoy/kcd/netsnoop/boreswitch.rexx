/* turn temp file into code */
/* temp file has the nipc inquiry tags from <envoy/nipc.h>
   like so:
       QUERY_IPADDR i
       QUERY_HOSTS  s
       MATCH_HOST   s
       QUERY_ATTNFLAGS x
       QUERY_MAXAVAILFAST d
       QUERY_KICKVERSION v
   where: i = code for IPAddress
          s = code for string
          x = code for hex
          d = code for number (ULONG)
          v = code for version number

   No comments, blank lines, or extraneous junk are allowed.
*/

signal on break_c

parse arg debug

if ~open('inf','t2','r') then
    exit 20

/* "tabs" */
atab = '    '
atab2 = atab || atab
atab3 = atab || atab2
atab4 = atab || atab3
atab5 = atab || atab4

/* Code to be generated
    switch (ti->ti_Tag) {
  string type stuff
        case QUERY_HOSTS:
            Printf("query_hosts: '%s'\n", ti->ti_Data);
            break;
  finished inet_addr handling stuff.......
  must make atmp function-wide.
        case QUERY_IPADDR:
            if (g->pr_ipaddr) {
                if (atmp = addrFromULong(ti->ti_Data)) {
                    Printf("query_ipaddr: '%s'\n", atmp);
                    FreeVec(atmp);
                }
            }
            break;
    } / end switch /

  Code that was generated...
  string type stuff
    if (ti = FindTagItem(QUERY_HOSTS, tagArr)) {
        Printf("query_hosts: '%s'\n", ti->ti_Data);
    }

  finished inet_addr handling stuff.......
    if (ti = FindTagItem(QUERY_IPADDR, tagArr)) {
        STRPTR atmp;
        if (atmp = addrFromULong(ti->ti_Data)) {
            Printf("query_ipaddr: '%s'\n", atmp);
            FreeVec(atmp);
        }
    }
*/

dbline = ''

say atab || "switch (ti->ti_Tag) {"

do while ~eof('inf')
    curr = readln('inf')
    if curr = '' then
        break;
    parse var curr tcase ttype
    tcase = strip(tcase)
    ttype = strip(ttype)
    
    lcase = lower(tcase) || ": "

    midbit = ''
    closebit = '0a'x || atab3 || 'break;'
    bit2 = atab3 || 'Printf("' || lcase

    select
        /* hex arg */
        when ttype = 'x' then do
            prbit = 'ti->ti_Data'
            bit3 = '$%08lx\n", ' || prbit || ');'
        end

        /* inet addr arg */
        when ttype = 'i' then do
            prbit = 'atmp'
            bit2 = atab5 || 'Printf("' || lcase
            bit3 = "'%s'" || '\n", ' || prbit || ');' || '0a'x
            midbit = atab3 || 'if (g->pr_ipaddr) {' || '0a'x || atab4 
            midbit = midbit || 'if (atmp = addrFromULong(ti->ti_Data)) {' || '0a'x
            closebit = atab5 || 'FreeVec(atmp);' || '0a'x || atab4 || '}' || '0a'x || atab3 || '}' || closebit
        end

        /* string arg */
        when ttype = 's' then do
            prbit = 'ti->ti_Data'
            bit3 = "'%s'" || '\n", ' || prbit || ');'
        end

        /* ulong arg */
        when ttype = 'd' then do
            prbit = 'ti->ti_Data'
            bit3 = '%lu\n", ' || prbit || ');'
        end

        /* version arg */
        when ttype = 'v' then do
            prbit = 'ti->ti_Data'
            bit3 = '%lu.%lu\n", (ti->ti_Data >> 16), (ti->ti_Data & 0x0000FFFF));'
        end

        otherwise do
            prbit = 'ti->ti_Data'  /* but why bother? */
            bit3 = "/* you're out of luck:" tcase ttype "*/"
        end
    end

    bit1 = '0a'x || atab2 || "case" tcase || ":"
    if debug ~= '' then
        bit2 = '#ifdef DEBUG' || '0a'x || bit2 || '$%08lx\n", ' || prbit || ');' || '0a'x || '#endif' || '0a'x || bit2


    say bit1
    say midbit || bit2 || bit3 || closebit

end
call close 'inf'

say atab || "}"  /* end switch */

exit

break_c:
    exit 10
return

lower: procedure
    parse arg gak
return (translate(gak,'abcdefghijklmnopqrstuvwxyz_','ABCDEFGHIJKLMNOPQRSTUVWXYZ_'))

