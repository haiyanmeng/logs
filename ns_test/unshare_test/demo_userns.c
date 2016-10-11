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
	printf("Before unshare, the capabilities are:\n");
	printf("capabilities: %s\n", cap_to_text(caps, NULL));

	r = unshare(CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWUSER );
	if(r == -1) {
		printf("unshare failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	caps = cap_get_proc();
	printf("After unshare, the capabilities are:\n");
	printf("capabilities: %s\n", cap_to_text(caps, NULL));

	printf("the process pid is: %ld\n", (long)getpid());

	r = execlp("sh", "sh", (char *)0);
	if(r == -1) {
		printf("execlp failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
