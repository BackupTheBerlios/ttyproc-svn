/* ttyproc lib - simple access to serial information vai proc, under linux
 *
 *
 *	This program is free software,
 *	licensed under the terms of the General Public License V2.
 */

/*! \file pathdefs.h
    \brief Contains prefixes and paths.

    This is an include file containing common device node prefixes and paths.
*/

/*! \var char devpaths[]
    \brief array containing common device node prefixes
*/

char *devpaths[] = {	
	 "/dev/ttyUSB",
	 "/dev/usb/tts/",
	 "/dev/tts/USB",
	 NULL
};
