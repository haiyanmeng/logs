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

#define errExit(msg)	do { perror(msg); exit(EXIT_FAILURE); \
						} while (0)

static void
usage(char *pname)
{
	fprintf(stderr, "Usage: %s [options] <pid>\n\n", pname);
	fprintf(stderr, "Create a child process that joins the user namespace of another process <pid>, and executes `sh`"
			"in a new mount|uts|pid|ipc|network namespace,\n"
			"and possibly the combinations of these five namespaces.\n\n");
	fprintf(stderr, "Options can be:\n\n");
#define fpe(str) fprintf(stderr, "	%s", str);
	fpe("-i		  New IPC namespace\n");
	fpe("-m		  New mount namespace\n");
	fpe("-n		  New network namespace\n");
	fpe("-p		  New PID namespace\n");
	fpe("-u		  New UTS namespace\n");
	exit(EXIT_FAILURE);
}

static int					  /* Startup function for cloned child */
childFunc(void *arg)
{
	cap_t caps;
	int r;

	printf("\n\n******* info of the child process - start ********\n");
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

static char child_stack[STACK_SIZE];	/* Space for child's stack */

int
main(int argc, char *argv[])
{
	cap_t caps;
	pid_t pid;
	char *existing_pid;
	char path[PATH_MAX];
	int fd;
	int flags, opt;

	while ((opt = getopt(argc, argv, "+imnpuh")) != -1) {
		switch (opt) {
		case 'i': flags |= CLONE_NEWIPC;		break;
		case 'm': flags |= CLONE_NEWNS;		 break;
		case 'n': flags |= CLONE_NEWNET;		break;
		case 'p': flags |= CLONE_NEWPID;		break;
		case 'u': flags |= CLONE_NEWUTS;		break;
		default:  usage(argv[0]);
		}
	}

	existing_pid = argv[optind];
	
	/* Create child; child commences execution in childFunc() */
	snprintf(path, sizeof(path), "/proc/%s/ns/user", existing_pid);

	fd = open(path, O_RDONLY);
	if(fd == -1) {
		printf("open(%s) failed: %s\n", path, strerror(errno));
		exit(EXIT_FAILURE);
	}

	printf("******* info of the parent process - start ********\n");
	caps = cap_get_proc();
	printf("before joining user namespace of process %s, \ncapabilities: %s\n\n", existing_pid, cap_to_text(caps, NULL));
	
	if(setns(fd, 0) == -1) {
		printf("setns(%d) failed: %s\n", fd, strerror(errno));
		close(fd);
		exit(EXIT_FAILURE);
	}
	close(fd);

	caps = cap_get_proc();
	printf("after joining user namespace of process %s, \ncapabilities: %s\n\n", existing_pid, cap_to_text(caps, NULL));

	/*
	good flags: CLONE_NEWPID CLONE_NEWUTS CLONE_NEWIPC CLONE_NEWNET
	bad flags: CLONE_NEWNS 
	*/

	pid = clone(childFunc, child_stack + STACK_SIZE,	/* Assume stack grows downward */
		flags|SIGCHLD, argv[1]);
		//CLONE_NEWPID|CLONE_NEWUTS|CLONE_NEWIPC|CLONE_NEWNET|SIGCHLD, argv[1]);

	if (pid == -1)
		errExit("clone");

	printf("the parent pid is: %ld; the child pid is: %ld\n", (long)getpid(), (long)pid);

	/* Parent falls through to here.  Wait for child. */

	if (waitpid(pid, NULL, 0) == -1)
		errExit("waitpid");

	exit(EXIT_SUCCESS);
}
