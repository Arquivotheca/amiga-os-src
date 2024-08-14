		Specification for Startup Animation


rev 1.0:	2/18/93	Allan Havemose


Design Goals:
=============

	o Show Amiga-CD-32 logo
	o Use AA and overscan as much as possible
	o Sound
	o Display Trademark sign (TM) somewhere.



Description of Startup-animation and 'load-animation':
======================================================

In this description the term 'startup-animation' designates
the initial animation with clouds and mountains, while
'load-animation' designates the smaller animation used to hide
load times.


Sequence without a disk in the drive at boot:
---------------------------------------------
startup-animation:

	o Screen is black
	o Fade in color-cycling sky/clouds and mountains
	o Slide translucent 'Amiga' in from the left.
	  Slide spinning metal 'CD' in from the foreground.
	  '32' "burns"

	No disk has been inserted in drive:

	o Wait 1 min.
	o loop "startup-animation" (slide in and out) in 20 sec.
	  cycles

	Disk has been inserted in drive:

	o CD device driver signals the startup-animation when
	  CD drive door has been closed. This is a sign to the
	  animation that it should get ready to start the
	  'load-animation'

	o CD device signals startup-animation that a CD has been
	  inserted or that it was "false alarm", in which case
	  the startup-animation continues it cycle.

	o Depending on in which state the startup-animation is in
	  one of the following should happen:

	  1. All of 'Amiga-CD-32' on screen not moving:
	  	Fade background out.
	  2. 'Amiga-CD-32' entering screen:
	  	Allow logo to finish entering then fade background
		out.
	  3. 'Amiga-CD-32' exiting screen.
	  	Allow logo to finish exiting. Fade background out.
	  	Face 'amiga-cd-32' in in final position.


	o Depending on CD type one of 3 things happens
	  CDGS title: go to load-animation.
	  CDTV title: Fade screen to black
	  	      reboot (in ECS)
	  	      load title without startup or load animation
 	  Audio CD:   Fade completely to black
 	  	      Go to Audio Player.


load-animation:

	o Free bitplane data
	o Keep CD Spinning.
	o Animate 'Amiga' and '32' if possible.
	o Dissolve quickly (<0.5 sec) when signalled to
	  BeginShutDownAnim().



Sequence with a CDGS CD in the drive:
-------------------------------------

	o Do not display startup-animation.
	o Place 'Amiga-CD-32' in the "final" positions on screens
	o run the load-animation

Sequence with a CDTV CD in the drive:
-------------------------------------

	o Do not display startup-animation.
	o Fade to black
	o Load title. (no animations at all, remember this is ECS)

Sequence with an Audio CD in the drive:
---------------------------------------

	o Do not display startup-animation.
	o go to CD Audio Player


