/**
    CS-NOS Lab 2
    main.c
    Purpose: Simulates interprocess communication using named and unnamed pipes

    @author Dario Pavlovic
    @version 1.0 25/03/2014
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#define MSG_SIZE 200

/**
    After instantiating child processess, main process runs 
    this function.

    @param pfd_write, pfd_read - arrays of file descriptors associated with the
    writing and reading pipe, respectively

    @return 
*/
void main_proc(int pfd_write[], int pfd_read[], 
        char pipe_name_write[], char pipe_name_read[]) {
    
    // close the read input
    close(pfd_write[0]);
    // close the write output
    close(pfd_read[1]);

    int named_write = open(pipe_name_write, O_WRONLY);
    int named_read = open(pipe_name_read, O_RDONLY);

    char message[MSG_SIZE];
    char buf[MSG_SIZE] = "";
    char ctrl[5] = "";

    fprintf(stdout, "Controller process\n");
    while (1) {
        fprintf(stdout, "Write a message - ?? to end: ");
        if (fgets(buf, MSG_SIZE, stdin) == NULL) {
            fprintf(stdout, "ERROR: Could not read input in main process.\n");
            continue;
        }

        sscanf(buf, "%s", message);

        if (strcmp(message, "??") == 0) {
            write(pfd_write[1], message, strlen(message) + 1);
            write(named_write, message, strlen(message) + 1);
            return;
        }

        fprintf(stdout, "Choose En(C)rypting or (D)ecrypting: ");
        if (fgets(buf, 5, stdin) == NULL) {
            fprintf(stdout, "ERROR: Could not read input in main process.\n");
            continue;
        }
        sscanf(buf, "%s", ctrl);
        if (strcmp(ctrl, "C") == 0) {
            write(pfd_write[1], message, strlen(message) + 1);
            read(pfd_read[0], message, MSG_SIZE);
        } else if (strcmp(ctrl, "D") == 0) {
            // send to decryption
            write(named_write, message, strlen(message) + 1);
            read(named_read, message, MSG_SIZE);
        } else {
            fprintf(stdout, "ERROR: Invalid command.\n");
            continue;
        }

        fprintf(stdout, "Controller process\n");
        fprintf(stdout, "Received: %s\n", message);
        fprintf(stdout, "\n");
    }
}

/**
    This function is run by a child in charge of
    simulating encryption.

    @param  pfd_write, pfd_read - array of file descriptors associated with the
    writing and reading pipe, respectively
    @return 
*/
void encrypt(int pfd_write[], int pfd_read[]) {
    // close read input
    close(pfd_write[0]);
    // close write output
    close(pfd_read[1]);

    char message[MSG_SIZE] = "";
    char crypt[MSG_SIZE] = "Encrypted(";
    char buf[MSG_SIZE] = "";

    while (1) {
        read(pfd_read[0], message, MSG_SIZE);
        sscanf(buf, "%s", message);
        if (strcmp(message, "??") == 0) {
            return;
        }

        fprintf(stdout, "\n");
        fprintf(stdout, "Encryption process\n");
        fprintf(stdout, "Received: %s\n", message);
        strcat(crypt, message);
        strcat(crypt, ")");
        fprintf(stdout, "Sending: %s\n", crypt);
        fprintf(stdout, "\n");

        write(pfd_write[1], crypt, strlen(crypt) + 1);
    }
}

/**
    This function is run by a child in charge of
    simulating decryption.

    @param  pipe_write - the name of the pipe to which to write
            pipe_read - the name of the pipe from which to read
    @return 
*/
void decrypt(char pipe_write[], char pipe_read[]) {
    int named_read = open(pipe_read, O_RDONLY);
    int named_write = open(pipe_write, O_WRONLY);

    char message[MSG_SIZE] = "";
    char crypt[MSG_SIZE] = "Decrypted(";
    char buf[MSG_SIZE] = "";

    while (1) {
        read(named_read, message, MSG_SIZE);
        sscanf(buf, "%s", message);
        if (strcmp(message, "??") == 0) {
            return;
        }

        fprintf(stdout, "\n");
        fprintf(stdout, "Decryption process\n");
        fprintf(stdout, "Received: %s\n", message);
        strcat(crypt, message);
        strcat(crypt, ")");
        fprintf(stdout, "Sending: %s\n", crypt);
        fprintf(stdout, "\n");

        write(named_write, crypt, strlen(crypt) + 1);
    }

    return;
}

int main(void) {
    // encrypting pipe
    int pfd_write[2], pfd_read[2];
    if (pipe(pfd_write) == -1 || pipe(pfd_read) == -1) {
        fprintf(stdout, "ERROR: Could not instantiate a pipe.\n");
        exit(1);
    }

    char pipe_name_read[] = "./pipe_read";
    char pipe_name_write[] = "./pipe_write";

    // decrypting named pipes
    unlink(pipe_name_read);
    unlink(pipe_name_write);
    if (mknod(pipe_name_read, S_IFIFO | 00600, 0) == -1 || 
            mknod(pipe_name_write, S_IFIFO | 00600, 0) == -1) {
        fprintf(stdout, "ERROR: Could not create a named pipe.\n");
        exit(1);
    }

    // make an encrypt child
    switch(fork()) {
        case -1:
            fprintf(stdout, "ERROR: Could not create a child process. Exiting.\n");
            exit(1);
        case 0:
            encrypt(pfd_read, pfd_write);
            exit(0);
        // default:
    }        

    // make a decrypt child
    switch(fork()) {
        case -1:
            fprintf(stdout, "ERROR: Could not create a child process. Exiting.\n");
            exit(1);
        case 0:
            decrypt(pipe_name_read, pipe_name_write);
            exit(0);
        // default:
    }   

    main_proc(pfd_write, pfd_read, pipe_name_write, pipe_name_read);
    int i;
    for (i = 0; i < 2; i++) {
        wait(NULL);
    }

    // clean up
    unlink(pipe_name_read);
    unlink(pipe_name_write);
    return 0;
}
