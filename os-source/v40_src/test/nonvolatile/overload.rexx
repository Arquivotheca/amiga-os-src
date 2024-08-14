/* see what happens when you overstress nv-lib */

parse arg items bytes nvpro clean .

signal on break_c

i = 0
bytes = strip(bytes)
items = strip(items)
nvpro = strip(nvpro)
clean = strip(clean)

usage = "overload <numberOfItems> <bytesPerItem> [protect] [clean]"

/* we MUST know how many items to deal with */
if (datatype(items) ~= 'NUM') then do
    say "non-numeric items:" items
    say usage
    exit 10
end

/* allow usage of "overload 3 clean" and variants, too */
if (((clean ~= '') & (upper(clean) ~= 'PROTECT')) | (upper(nvpro) = 'CLEAN') | (upper(bytes) = 'CLEAN')) then do
    say "Erasing items 1 to" items "owned by Overload."
    do i = 1 to items
        address command 'setnvprotection' 'name Overload item test' || i 'flags 0x00'
        address command 'deletenv' 'name Overload item test' || i 
    end
    exit 0
end

/* deal with args */
if (datatype(bytes) ~= 'NUM') then do
    say usage
    exit 10
end
if nvpro = '' then
    nvpro = 0
else
    nvpro = 1

do i = 1 to items
    address command 'storenv' 'name Overload item test' || i 'bufsize' bytes 'buffill 65'
    if (nvpro) then
        address command 'setnvprotection' 'name Overload item test' || i 'flags 0x01'
end



exit

break_c:
    say "Last item was" i || ".  Goodbye."
    exit
return


