#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
	if (argc != 2) {
		perror("insufficient arguments\n");
		return 1;
	}

	pid_t pid = atoi(argv[1]);

	if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
		perror("ptrace attach");
		return 1;	
	}

	waitpid(pid, NULL, 0);

	struct user_regs_struct regs;
	if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) == -1) {
		perror("ptrace get registers");
		return 1;
	}

#if defined(__x86_64__)
	printf("Instruction Pointer : 0x%llx\n", regs.rip);
	printf("Stack Pointer : 0x%llx\n", regs.rsp);

	errno = 0;
	long data = ptrace(PTRACE_PEEKDATA, pid, (void*) regs.rsp, NULL);
	if (data == -1 && errno != 0) {
		perror("ptrace peek data");
		return 1;
	} else {
		printf("data at rsp: 0x%llx\n", data);
	}
#endif

	if (ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1) {
		perror("ptrace detach");
		return 1;
	}

	return 0;
}
