/* ttyproc lib - simple access to serial information via proc, under linux.
 *
 * 
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 */

/*! \file ttyproc.h
    \brief Include file for ttyproc library.
*/
 
#ifndef _TTYPROC_H
#define _TTYPROC_H

/*! \var typedef struct serial_entry serialent_t
    \brief A type definition for serial_entry.
*/


typedef struct serial_entry serialent_t; 

/* functions used within the library */
static void addnode(serialent_t *topush);
static void freenodes();

/*! \brief An entry representing one line from /proc/tty/driver/usbserial.
    
    This structure is used when creating a linked list from usbserial entries.
*/
struct serial_entry {
/*! \var int minor
    \brief minor of device
    
    The USB serial core registers it's major number 188 with the kernel.  The minor number can range from 0 to 255 for USB serial Adapters.  This is used when finding device path.
*/
	int	minor;
/*! \var char module[15]
    \brief usbserial module name
    
    This is the short name a usbserial driver registers with the core.
*/
	char	module[15];
/*! \var char devname[25]
    \brief device name
    
    This is the string obtained from usb descriptor iProduct.
*/
	char	devname[25];
/*! \var int vid
    \brief vendor identifier
    
    This is a hex value representing the unique vendor identifier assigned to a usb device.
*/
/*! \var int pid
    \brief product identifier
    
    This is a hex value representing the unique product identifier assigned to a usb device.
*/
	int	vid, pid;
/*! \var int numports
    \brief total ports
    
    This is the number of ports a USB serial device has.
*/
	int	numports;
/*! \var int portid
    \brief port identifier
    
    This is the port number assigned to the interface.  USB serial devices can sometimes have multiple ports.
*/
	int	portid;
/*! \var int nodeid
    \brief node identifier
    
    This is a number assigned to each node when the usbserial listing is scanned.  This number can be used to differentiate like devices, although it is impossible to pinpoint which device is which when they are the same.  Some clever hack will need to be thought up to handle this.
*/
	int	nodeid;
/*! \var int devid
    \brief not implemented
    
    This will be used to differentiate like devices in the future.  At the moment it will be set to 0.
*/
	int	devid;
/*! \var char buspath
    \brief raw usb path
    
    This is the usb bus path in raw form.
*/
	char	buspath[25];

/*! \var struct serial_entry *next
    \brief pointer to next node
    
    This is a pointer to the next node in the list, if not null.
*/
	struct serial_entry *next;
};

/*! \var serialent_t *begin
    \brief a pointer to beginning of list
    
    This is a pointer used to assign the node used for parsing to a linked lists beginning.
    
    Never try to assign this value - always use in assignment.
*/

/*! \var int totalents
    \brief total serial_entry nodes
    
    This is a global variable used to keep count of current nodes comprising the linked list.
*/

serialent_t *begin;  /* pointer to beginning of node list, null on empty list */
int totalents;       /* total entries found when parsing usbserial, updated on call to ttyproc_scanlist */

/*! \fn int ttyproc_init()
    \brief checks proc is mounted, and performs various startup tasks
    
    This must be called first before doing anything else.
*/
/*! \fn int ttyproc_scanlist()
    \brief frees current node list and re-scans usbserial data.
    
    In the future, this might scan serial data as well.
*/
/*! \fn int ttyproc_getpath_node(char *dpath,serialent_t *node)
    \brief returns a found node path given a node in the linked list
    \param dpath The string to read device node path into.
    \param node The node to be used when obtaining path.
    
    This should be used if parsing the linked list within a program.  Using ttyproc_getpath_vpid can yield wierd results otherwise.
*/
/*! \fn int ttyproc_getpath_vpid(char *dpath,int vid,int pid,int pnum)
    \brief use vendor and product ids to obtain a node path
    \param dpath The string to read device node path into.
    \param vid The vendor identifier of the usb serial device.
    \param pid The product identifier of the usb serial device.
    \param pnum The port number identifier.  Typically set to 1 with most devices.
*/
/*! \fn int ttyproc_getfd_node(serialent_t *node, u_int16_t flags)
    \brief obtain a file descriptor from a passed node with open flags
    \param node The node to be used for obtaining a file descriptor.
    \param flags The flags that will be passed to open function call.
*/
/*! \fn void ttyproc_close()
    \brief frees current node list, and performs various cleanup tasks
*/

/* available function prototypes */
int  ttyproc_init();    		/* checks proc is mounted, performs various startup tasks */
int  ttyproc_scanlist();		/* free's current node list and re-scans serial and usbserial data */
	/* if successful, found node path will be placed into dpath (NULL when both paths fail stat) */
int  ttyproc_getpath_node(char *, serialent_t *); /* this works best when parsing within program */
int  ttyproc_getpath_vpid(char *, int , int , int); /* pnum should be 1 for single port lookup */
int  ttyproc_getfd_node(serialent_t *, u_int16_t); /* returns file descriptor for node using flags for 'open' */
void ttyproc_close();  			/* frees current node list, performs other cleanup tasks */

#endif /* define _TTYPROC_H */
