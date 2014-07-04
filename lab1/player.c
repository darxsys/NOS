#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <unistd.h>

struct m_buf {
    long type;
    char text[200];
};

key_t key_read;
key_t key_write;
int msqid_read;
int msqid_write;
struct m_buf message;
int proc_id;

void initialize_waters(int *ocean, int width, int height, int num_boats) {
    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            ocean[i * width + j] = 0;
        }
    }

    while (num_boats > 0) {
        i = rand() % height;
        j = rand() % width;

        if (ocean[i * width + j] == 1) {
            continue;
        }

        ocean[i * width + j] = 1;
        num_boats--;
    }
}

void print_waters(int *ocean, int width, int height) {
    printf("Player %d\n", proc_id);
    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            printf("%c ", ocean[i * width + j] == 0 ? '-' : 'o');
        }
        printf("\n");
    }
}

void play(int num_boats, int width, int height, int *ocean) {
    int i, j;
    char buf[200];
    char c;
    char ret_message[200];
    // char 
    if (proc_id == 1) {
        fprintf(stdout, "Player %d\nCoordinates to hit: ", proc_id);
        // fscanf(stdin, "%d %d", &i, &j);
        fgets(buf, 190, stdin);
        sscanf(buf, "%d %d", &i, &j);
        while(i > height || j > width || i < 1 || j < 1) {
            fprintf(stderr, "Coordinates are not in the proper range.\n");
            fprintf(stdout, "Player %d\nCoordinates to hit: ", proc_id);
            fgets(buf, 190, stdin);
            sscanf(buf, "%d %d", &i, &j);
        }

        sprintf(buf, "%d:%d", i, j);
        strcpy(message.text, buf);
        message.type = 1;
        if (msgsnd(msqid_write, &message, sizeof(message.text), 0) == -1) {
            fprintf(stderr, "Didn't send anything.Exiting.");
            exit(1);
        }

        if (msgrcv(msqid_read, &message, sizeof(message.text), 0, 0) == -1) {
            perror("Did not get anything.");
            exit(1);
        }

        // fprintf(stdout, "Process %d is already out.\n", proc_id);
        // fprintf("")
    }

    while(1) {
        if (msgrcv(msqid_read, &message, sizeof(message.text), 0, 0) == -1) {
            fprintf(stderr, "Didn't get anything.");
            exit(1);
        }

        // fprintf(stdout, "Process %d is already out.\n", proc_id);

        if (strcmp(message.text, "Victory") == 0) {
            fprintf(stdout, "Player %d is victorious. Press any key.\n", proc_id);
            // fscanf(stdin, "%c", &c);
            getchar();
            return;
        }

        // fprintf(stdout, "Process %d is already out.\n", proc_id);
        // fprintf(stdout, "%s\n", message.text);  
        sscanf(message.text, "%d:%d", &i, &j);
        fprintf(stdout, "Shot at element: %d %d -> ", i, j);
        // printf("Here\n");
        if (ocean[(i-1) * width + j-1] == 0) {
            // printf("Here\n");
            strcpy(ret_message, "miss");    
        } else {
            // printf("Here\n");
            strcpy(ret_message, "hit");
        }
        fprintf(stdout, "%s\n", ret_message);
        strcpy(message.text, ret_message);

        if (ocean[(i-1) * width + j-1] == 1) {
            num_boats--;
            ocean[(i-1) * width + j-1] = 0;
        }

        // printf("here %d\n", proc_id);
        // fprintf(stdout, "Process %d is already out.\n", proc_id);

        if (msgsnd(msqid_write, &message, sizeof(message.text), 0) == -1) {
            perror("Did not send anything.");
            exit(1);
        }
        // fprintf(stdout, "Process %d is already out.\n", proc_id);

        // block this player until the other one prints out - maybe

        if (num_boats <= 0) {
            strcpy(message.text, "Victory");
            fprintf(stdout, "Player %d lost :(. Press any key.\n", proc_id);
            // fscanf(stdin, "%c", &c);
            getchar();
            if (msgsnd(msqid_write, &message, sizeof(message.text), 0) == -1) {
                perror("Did not send anything.");
                exit(1);
            }
            return;
        }

        // main part
        fprintf(stdout, "Player %d\nCoordinates to hit: ", proc_id);
        fgets(buf, 190, stdin);
        sscanf(buf, "%d %d", &i, &j);
        while(i > height || j > width || i < 1 || j < 1) {
            fprintf(stderr, "Coordinates are not in the proper range.\n");
            fprintf(stdout, "Player %d\nCoordinates to hit: ", proc_id);
            fgets(buf, 190, stdin);
            sscanf(buf, "%d %d", &i, &j);
        }

        sprintf(buf, "%d:%d", i, j);
        strcpy(message.text, buf);
        message.type = 1;
        if (msgsnd(msqid_write, &message, sizeof(message.text), 0) == -1) {
            fprintf(stderr, "Didn't send anything.Exiting.");
            exit(1);
        }

        if (msgrcv(msqid_read, &message, sizeof(message.text), 0, 0) == -1) {
            perror("Did not get anything.");
            exit(1);
        }
    }
}

int main(int argc, char **argv) {
    proc_id = atoi(argv[1]);
    char *env_one, *env_two;
    char substring[200];

    int width = 4;
    int height = 4;
    int num_boats = 2;
    int msgflag = 666 | IPC_CREAT;

    env_one = getenv("KEY_ONE");
    env_two = getenv("KEY_TWO");
    // fprintf(stderr, "%s\n", env_one);
    // fprintf(stderr, "%s\n", env_two);

    if (env_one == NULL || env_two == NULL) {
        fprintf(stderr, "Environment not set as it should be. ");
        fprintf(stderr, "Player %d exiting.\n", proc_id);
        exit(1);
    }

    // strncpy(substring, )
    key_write = atoi(env_one);
    key_read = atoi(env_two);
    if (proc_id == 2) {
        // switch values
        key_write ^= key_read;
        key_read ^= key_write;
        key_write ^= key_read;
    }
    srand((unsigned)(time(NULL)+proc_id));

    msqid_read = msgget(key_read, msgflag);
    msqid_write = msgget(key_write, msgflag);

    if (msqid_read == -1 || msqid_write == -1) {
        perror("Could not open message queue.");
        fprintf(stderr, "Player %d exiting.\n", proc_id);
        exit(1);
    }

    int *ocean = (int*) malloc(sizeof(int) * width * height);
    if (ocean == NULL) {
        fprintf(stderr, "Could not allocate ocean. Player %d exiting.\n", proc_id);
    }
    initialize_waters(ocean, width, height, num_boats);

    message.type = 1;
    strcpy(message.text, "Ready");
    if (proc_id == 1) {
        print_waters(ocean, width, height);
        if (msgsnd(msqid_write, &message, sizeof(message.text), 0) == -1) {
            perror("Could not send 'Ready' to Player 2. Exiting.");
            exit(1);
        }

        if (msgrcv(msqid_read, &message, sizeof(message.text), 0, 0) == -1) {
            perror("Could not receive a 'Ready' message from Player 2. Exiting.");
            exit(1);
        }

        if (strcmp(message.text, "Ready") != 0) {
            fprintf(stderr, "Player 2 didn't write a good message. Exiting.\n");
            exit(1);
        }

    } else {
        if (msgrcv(msqid_read, &message, sizeof(message.text), 0, 0) == -1) {
            perror("Didn't get 'Ready' message from Player 1. Exiting.");
            exit(1);

        }
        print_waters(ocean, width, height);

        if (msgsnd(msqid_write, &message, sizeof(message.text), 0) == -1) {
            perror("Could not send 'Ready' to Player 1. Exiting.");
            exit(1);
        }

        if (strcmp(message.text, "Ready") != 0) {
            fprintf(stderr, "Player 1 didn't write a good message. Exiting.\n");
            exit(1);
        }
    }

    play(num_boats, width, height, ocean);

    free(ocean);
    return 0;
}
