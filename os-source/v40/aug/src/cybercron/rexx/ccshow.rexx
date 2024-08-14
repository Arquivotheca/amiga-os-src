/***********************/
/* CCShow.rexx         */
/* 5/1/92 - caw        */
/* based on            */
/* NCShow.rexx         */
/* 7/20/89 - Don Nafis */
/***********************/


/* Remove the comments below to trace this script. */
/* trace ?r */

options failat 20
options results

address cybercron

'LIST_EVENTS'
leResult = result
if (result = "") then say "LIST_EVENTS error "
else
do
  eventlist = result
  numwords = words(eventlist)

  if (numwords > 0) then
   if (word(eventlist,1) ~= "<None>") then do

    say
    say 'CyberCron - Active Events:'
    say

    do j=1 to numwords
      event = word(eventlist,j)
      'SHOW_EVENT '|| event
	eventDisp = result
      if (eventDisp == "") then
        say '->' || event || ' not found.'
      else
        say '->' || eventDisp
    end
  end
  else
  do
    say
    say 'CyberCron has no active events.'
  end

  say

  'SHOW_STATUS'
    say 'Status: '||result
  say

end
