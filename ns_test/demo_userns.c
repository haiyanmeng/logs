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

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static int                      /* Startup function for cloned child */
childFunc(void *arg)
{
    cap_t caps;

	printf("\n\n******* info of the child process - start ********\n");
    printf("eUID = %ld;  eGID = %ld\n", (long) geteuid(), (long) getegid());
    printf("pid = %ld;  ppid = %ld\n", (long) getpid(), (long) getppid());

    caps = cap_get_proc();
    printf("capabilities: %s\n", cap_to_text(caps, NULL));

	execlp ("sh", "sh", (char *)0);
}

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];    /* Space for child's stack */

int
main(int argc, char *argv[])
{
	cap_t caps;
    pid_t pid;

    /* Create child; child commences execution in childFunc() */
	printf("******* info of the parent process - start ********\n");
    caps = cap_get_proc();
    printf("capabilities: %s\n", cap_to_text(caps, NULL));

    pid = clone(childFunc, child_stack + STACK_SIZE,    /* Assume stack grows downward */
                CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWUSER | SIGCHLD, argv[1]);
    if (pid == -1)
        errExit("clone");

	printf("the parent pid is: %ld; the child pid is: %ld\n", (long)getpid(), (long)pid);

    /* Parent falls through to here.  Wait for child. */

    if (waitpid(pid, NULL, 0) == -1)
        errExit("waitpid");

    exit(EXIT_SUCCESS);
}
