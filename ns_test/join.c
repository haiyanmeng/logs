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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static int                      /* Startup function for cloned child */
childFunc(void *arg)
{
    cap_t caps;
	int r;

    printf("eUID = %ld;  eGID = %ld\n", (long) geteuid(), (long) getegid());
    printf("pid = %ld;  ppid = %ld\n", (long) getpid(), (long) getppid());

    caps = cap_get_proc();
    printf("capabilities: %s\n", cap_to_text(caps, NULL));
	
	r = execlp("sh", "sh", (char *)0);
	if(r == -1) {
		printf("execlp failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];    /* Space for child's stack */

int
main(int argc, char *argv[])
{
    cap_t caps;
    pid_t pid;
	char path[PATH_MAX];
	int fd;

    /* Create child; child commences execution in childFunc() */
	snprintf(path, sizeof(path), "/proc/%s/ns/user", argv[1]);

    fd = open(path, O_RDONLY);
	if(fd == -1) {
		printf("open(%s) failed: %s\n", path, strerror(errno));
		exit(EXIT_FAILURE);
	}
    caps = cap_get_proc();
    printf("before joining user namespace of process %s, \ncapabilities: %s\n\n", argv[1], cap_to_text(caps, NULL));
	
	if(setns(fd, 0) == -1) {
		printf("setns(%d) failed: %s\n", fd, strerror(errno));
		close(fd);
		exit(EXIT_FAILURE);
	}
	close(fd);

    caps = cap_get_proc();
    printf("after joint user namespace of process %s, \ncapabilities: %s\n\n", argv[1], cap_to_text(caps, NULL));
	
    pid = clone(childFunc, child_stack + STACK_SIZE,    /* Assume stack grows downward */
		CLONE_NEWNS|SIGCHLD, argv[1]);
                //CLONE_NEWPID|CLONE_NEWUTS|CLONE_NEWIPC|CLONE_NEWNET|SIGCHLD, argv[1]);
// good flags: CLONE_NEWPID CLONE_NEWUTS CLONE_NEWIPC CLONE_NEWNET
// bad flags: CLONE_NEWNS 
    if (pid == -1)
        errExit("clone");

	printf("the parent pid is: %ld; the child pid is: %ld\n", (long)getpid(), (long)pid);

    /* Parent falls through to here.  Wait for child. */

    if (waitpid(pid, NULL, 0) == -1)
        errExit("waitpid");

    exit(EXIT_SUCCESS);
}
