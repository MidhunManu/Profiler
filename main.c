#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define MAX_FRAMES 50
#define SAMPLE_FREQ 1000

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
        return 1
    }

#if defined(__x86_64__)

    ng rbp = regs.rbp;
    int frame = 0;

    while (rbp && frame < 50) {  // limit to avoid infinite loops
        errno = 0;

        long next_rbp = ptrace(PTRACE_PEEKDATA, pid, (void*)rbp, NULL);
        if (next_rbp == -1 && errno != 0) {
            perror("ptrace peek next rbp");
            break;
        }

        long ret_addr = ptrace(PTRACE_PEEKDATA, pid, (void*)(rbp + 8), NULL);
        if (ret_addr == -1 && errno != 0) {
            perror("ptrace peek return addr");
            break;
        }

        printf("frame %d: return address = 0x%lx\n", frame, ret_addr);

        rbp = next_rbp;
        frame++;
    }
#endif

    if (ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1) {
        perror("ptrace detach");
        return 1;
    }

    return 0;
}
