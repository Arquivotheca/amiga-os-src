/* turn temp file into code */
signal on break_c

parse arg debug

if ~open('inf','t2','r') then
    exit 20

atab = '    '
atab2 = atab || atab
atab3 = atab || atab2

/* finished inet_addr handling stuff.......
    if (ti = FindTagItem(QUERY_IPADDR, tagArr)) {
        STRPTR atmp;
        if (atmp = addrFromULong(ti->ti_Data)) {
            Printf("query_ipaddr: '%s'\n", atmp);
            FreeVec(atmp);
        }
    }
*/

do while ~eof('inf')
    curr = readln('inf')
    if curr = '' then
        break;
    parse var curr tcase ttype
    tcase = strip(tcase)
    ttype = strip(ttype)
    
    lcase = lower(tcase) || ": "

    midbit = ''
    closebit = '0a'x || atab || '}'
    bit2 = atab2 || 'Printf("' || lcase

    select
        /* hex arg */
        when ttype = 'x' then do
            prbit = 'ti->ti_Data'
            bit3 = '$%08lx\n", ' || prbit || ');'
        end

        /* inet addr arg */
        when ttype = 'i' then do
            prbit = 'atmp'
            bit2 = atab3 || 'Printf("' || lcase
            bit3 = "'%s'" || '\n", ' || prbit || ');' || '0a'x
            midbit = atab2 || 'STRPTR atmp;' || '0a'x || atab2 
            midbit = midbit || 'if (atmp = addrFromULong(ti->ti_Data)) {' || '0a'x
            closebit = atab3 || 'FreeVec(atmp);' || '0a'x || atab2 || '}' || closebit
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

    bit1 = atab || "if (ti = FindTagItem(" || tcase || ", tagArr)) {"
    if debug ~= '' then
        bit2 = '#ifdef DEBUG' || '0a'x || bit2 || '$%08lx\n", ' || prbit || ');' || '0a'x || '#endif' || '0a'x || bit2


    say bit1
    say midbit || bit2 || bit3 || closebit

end
call close 'inf'
exit

break_c:
    exit 10
return

lower: procedure
    parse arg gak
return (translate(gak,'abcdefghijklmnopqrstuvwxyz_','ABCDEFGHIJKLMNOPQRSTUVWXYZ_'))

