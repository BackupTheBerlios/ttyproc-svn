/* ttyproc lib - simple access to serial information via proc, under linux.
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 */

/*! \file ttyproc.c
    \brief Contains library source and test application.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>

#ifdef __HAVE_CONFIG_H
#include "config.h"
#endif

#include "ttyproc.h"
#include "pathdefs.h"

static uid_t euid, ruid;


/* TODO
 *   -Check over logic setting process permissions for file access.
 *   -Figure out why grabbing a file descriptor from outside the
 *    library object code with path given by ttyproc_getpath_node()
 *    causes a system crash along with the invalid file descriptor
 *    returned by open().
 */
  
int ttyproc_init()
{
	DIR *procdir;

	begin = NULL;
	totalents = 0;
	
	/* check /proc is mounted */
	procdir = opendir("/proc/tty");
	if (!procdir) {
		fprintf(stderr, "ttyproc_init: /proc appears to be emtpy - %s\n", strerror(errno));
		return -errno;
	}
	closedir(procdir);

	ruid = getuid();
	euid = geteuid();
	
	return 0;
}


void ttyproc_close()
{
	freenodes();
}


/* fill up linked serial_entry list by parsing /proc/tty/driver/usbserial */
int ttyproc_scanlist()
{
	char *linebuf;
	char temp[30];
	FILE *fp;
	int status;

	if (begin)
		freenodes();
	
	/* change process userid for file access */
	#ifdef _POSIX_SAVED_IDS
		status = seteuid(euid);
	#else
		status = setreuid(ruid, euid);
	#endif
	
	if (status<0) {
		fprintf(stderr, "ttyproc_scanlist: setuid - %s\n", strerror(errno));
		return -errno;
	}
	
	fp = fopen("/proc/tty/driver/usbserial", "r");

	#ifdef _POSIX_SAVED_IDS
		seteuid(ruid);
	#else
		setreuid(euid, ruid);
	#endif
	
	if (!fp) {
		fprintf(stderr, "fopen usbserial: %s\n", strerror(errno));
		return -errno;
	}

	linebuf = (char *)malloc(256);
	if (!linebuf) {
		fprintf(stderr, "ttyproc_scanlist: memory allocation for linebuffer failed");
		fclose(fp);
		return -ENOMEM;
	}

	totalents = 0;
	fgets(temp, 30, fp); /* skip usbserial header */

	while (fgets(linebuf, 256, fp)!=NULL) {
		serialent_t *node;
		node = (serialent_t *)malloc(sizeof(*node));
		if (!node) {
			fprintf(stderr, "ttyproc_scanlist: memory allocation for node failed");
			freenodes();
			fclose(fp);
			return -ENOMEM;
		}

		totalents++;

		sscanf(linebuf, "%d: module:%s name:\"%[^\"]\" vendor:%x product:%x num_ports:%d port:%d path:%s\n", 
	               &node->minor, node->module, node->devname, &node->vid, &node->pid, &node->numports,
		       &node->portid, node->buspath);
		node->nodeid = totalents;

		addnode(node);
	}

	free(linebuf);	
	fclose(fp);
	return 0;
} /* ttyproc_scanlist */


/* fill up linked serial_entry list by parsing /proc/tty/driver/usbserial */
int ttyproc_getpath_node(char *dpath, serialent_t *node)
{
	struct stat sb;
	char path[15];
	int x = 0;
	char minor[2];

	if (!node)
		return -1;

	sprintf(minor, "%d", node->minor);

	while (devpaths[x]) {
		strcpy(path, devpaths[x]);
		strcat(path, minor);
		if (stat(path, &sb) == 0)
			break;
		x++;
	}

	if (devpaths[x])
		strcpy(dpath, path);
	else {
		*dpath = '\0';
		return -1;
	}
	
	return 0;
} /* ttyproc_getpath_node */


/* if calling when parsing nodes, use ttyproc_getpath_node instead */
int ttyproc_getpath_vpid(char *dpath, int vid, int pid, int pnum)
{
	serialent_t *cur;

	cur = begin;

	while (cur) {
		if ((cur->vid == vid) && (cur->pid == pid)) {
			if (cur->portid == pnum) {
				return ttyproc_getpath_node(dpath, cur);	
			}
		}
		cur = cur->next;
	}

	if (!cur) {
		*dpath = '\0';
		return -1;
	}
	
	return 0;
} /* ttyproc_getpath_vpid */


/* returns file descriptor for node - <0 on fail */
int ttyproc_getfd_node(serialent_t *node, u_int16_t flags)
{
	char path[15];

	if (!begin)
		return -1;

	if (ttyproc_getpath_node(path, node)<0)
		return -1;
	
	return open(path, flags);
}


/* TODO: link nodes for FIFO access */
static void addnode(serialent_t *topush)
{	
	topush->next = begin;
	begin = topush;
} /* addnode */


static void freenodes()
{	
	serialent_t *nodeptr;

	totalents = 0;

	if (!begin)
		return;

	while (begin) {
		nodeptr = begin->next;
		free(begin);
		begin = nodeptr;
	}
} /* freenodes */

	
#ifdef TESTLIB

/* example usage "./test 1163 0100" */
int main(int argc, char *argv[])
{
	serialent_t *node;
	char path[20] = { 0 };
	int vid, pid;
	int match = 0;

	if (argc < 3) {
		printf("Usage is \"test VendorID ProductID\"\n");
		return 1;
	}

	sscanf(argv[1], "%x", &vid);
	sscanf(argv[2], "%x", &pid);

	printf("Looking for device(s) with VendorID 0x%04x and Product ID 0x%04x\n", vid, pid);		

	if (ttyproc_init()<0) {
		return 1;
	}

	/* perform an update of the list */
	ttyproc_scanlist();

	if (totalents > 0) {
	
		node = begin;
	
		while (node) {
		
			if (node->vid == vid && node->pid == pid) {
				match = 1;
				printf("Found \"%s\" connected at %s\n", node->devname, node->buspath);
				if (ttyproc_getpath_node(path, node) == 0) {
					printf("Found device node path at: %s\n", path);
				} else
					printf("Could not resolve node path.\n");
			}

			node = node->next;
		}
	} else
		printf("No entries available to parse.\n");

	if (!match)
		printf("Could not find device.\n");

	ttyproc_close();
	return 0;
}
#endif
