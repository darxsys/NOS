#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <unistd.h>

key_t key_first = 2345;
key_t key_second = 5432;
int msqid_one;
int msqid_two;

void clean(int code) {
    if(msgctl(msqid_one, IPC_RMID, NULL) == -1 || 
            msgctl(msqid_two, IPC_RMID, NULL) == -1) {
        perror("Could not delete one or both queues.\nPlease delete them yourself.");
        // exit(1);
    }

    putenv("KEY_ONE=");
    putenv("KEY_TWO=");
    exit(1);
}

int main(void) {
    // give all permissions
    int msgflag = 666 | IPC_CREAT;

    if ((msqid_one = msgget(key_first, msgflag)) == -1) {
        perror("Could not create queue.");
        exit(1);
    }

    if ((msqid_two = msgget(key_second, msgflag)) == -1) {
        perror("Could not create queue.");
        exit(1);
    }

    char buf_one[200];
    char buf_two[200];
    sprintf(buf_one, "KEY_ONE=%d", key_first);
    // fprintf(stdout, "%s\n", buf);
    if (putenv(buf_one) != 0) {
        fprintf(stderr, "Could not set environment variable %s.", buf_one);
    }

    sprintf(buf_two, "KEY_TWO=%d", key_second);
    // fprintf(stdout, "%s\n", buf);
    if (putenv(buf_two) != 0) {
        fprintf(stderr, "Could not set environment variable %s.", buf_two);
    }

    sigset(SIGINT, clean);
    char arg[5];
    int i;
    for(i = 1; i <= 2; i++) {
        pid_t pid = fork();
        if(pid == -1) {
            printf("Could not create player %d.\n", i);
            clean(0);
            exit(1);
        } else if (pid == 0) {
            sprintf(arg, "%d", i);
            execl("./player", "player", arg, NULL);
            break;
        } else {
            continue;
        }
    }

    for (i = 1; i <= 2; i++) {
        wait(NULL);
    }

    clean(0);
    return 0;
}
