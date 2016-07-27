/* demo_userns.c

   Copyright 2013, Michael Kerrisk
   Licensed under GNU General Public License v2 or later

   Demonstrate the use of the clone() CLONE_NEWUSER flag.

   Link with "-lcap" and make sure that the "libcap-devel" (or
   similar) package is installed on the system.
*/
#define _GNU_SOURCE
#include <sys/capability.h>
#include <sys/wait.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int
main(int argc, char *argv[])
{
	cap_t caps;
	pid_t pid;
	int r;

	/* Create child; child commences execution in childFunc() */
	printf("******* info of the parent process - start ********\n");
	caps = cap_get_proc();
	printf("capabilities: %s\n", cap_to_text(caps, NULL));

	r = unshare(CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWUSER );
	if(r == -1) {
		printf("unshare failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("the process pid is: %ld\n", (long)getpid());

	r = execlp("sh", "sh", (char *)0);
	if(r == -1) {
		printf("execlp failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
