/*
 * printcap file to translate queuename -> hostname.  
 */

parse arg queuename junk

host = ""
if queuename == "lp" then do
	queuename = "ps"
end

if queuename == "ps" then do
	host = "cbmvax"
end

if queuename == "lj" then do
	host = "rocky"
end

return host
