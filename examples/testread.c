/* The contents within this file are in the public domain */

#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>

#include "../ttyproc.h"


#define MAXLEN 10
#define MAXBCNT 1000

/* define USE_TTYP_PATH here to switch from using ttyproc_getpath_node 
to call open() */
//#define USE_TTYP_PATH


/* example usage "./test 1163 0100" */
int main(int argc, char *argv[])
{
	serialent_t *node;
	char path[20];
	int vid, pid;
	int match = 0;
	int totbytes = 0;
	char buf;
	int fd = -1, x;

	if (argc < 3) {
		printf("Usage is \"$ %s VendorID ProductID\"\n", argv[0]);
		return 1;
	}

	sscanf(argv[1], "%x", &vid);
	sscanf(argv[2], "%x", &pid);

	if (ttyproc_init()<0) {
		return 1;
	}
	
	printf("Looking for device with VendorID 0x%04x and Product ID 0x%04x\n", vid, pid);
	
	/* perform an update of the list */
	ttyproc_scanlist();

	if (totalents > 0) {
	
		node = begin;
	
		while (node) {
		
			if (node->vid == vid && node->pid == pid &&
			    node->portid == 1) {
				match = 1;
				printf("Found %s connected at %s\n",                      node->devname, node->buspath);
				if (ttyproc_getpath_node(path, node) == 0) {
					printf("Found device node path at: %s\n", path);
					break;
				} else
					printf("Could not resolve node path.\n");
			}

			node = node->next;
		}

	} else
		printf("No entries available to parse.\n");

#ifdef USE_TTYP_PATH
	if (path)
		fd = ttyproc_getfd_node(node, O_RDONLY);
#endif
	ttyproc_close();

	if (!match) {
		printf("Could not find device.\n");
		goto exit;
	}

	struct termios tty_termios, dev_termios, tty_oldtermios;
	int ttyfd;

	ttyfd = open("/dev/tty", O_RDONLY);
	
	tcgetattr(ttyfd, &tty_termios);
	tty_termios.c_cflag = (B4800 | CS8 | HUPCL | CLOCAL | CREAD);

	tcgetattr(ttyfd, &tty_oldtermios);
	tcsetattr(ttyfd, TCSANOW, &tty_termios);
	
	if (path) {
#ifndef USE_TTYP_PATH
		if (fd = open(path, O_RDONLY)<0) {
			printf("Failed opening device node %s\n", path);
			goto exit;
		}
#endif
		
		printf("File descriptor is %d\n", fd);
		
		tcgetattr(fd, &dev_termios);
		dev_termios.c_cflag = (B4800 | CREAD | HUPCL | CLOCAL | CS8);
		dev_termios.c_iflag = dev_termios.c_lflag = dev_termios.c_oflag = 0;
		tcsetattr(fd, TCSANOW, &dev_termios);

		while (totbytes < MAXBCNT) {
			totbytes += read(fd, &buf, 1);
			
			if (totbytes<0)
				goto exit;
		
			printf("%c", buf);
		}
		
		close(fd);
	}
	
exit:
	
	tcsetattr(ttyfd, TCSANOW, &tty_oldtermios);
	close(fd);
	close(ttyfd);

	return 0;
}
