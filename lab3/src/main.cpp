/**
    FER-CS-NOS
    main.cpp
    Entry point into the SHA3-256 algorithm implementation.

    @author Dario Pavlovic
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdint.h>
using namespace std;

#include "keccak.h"
#include "utils.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Input file path please.\n");
        exit(1);
    }

    char *file = argv[1];
    char* hash = keccak(file);
    printf("Hash:\n");
    for (int i = 0; i < 32; i++) {
        printf("%02x", (unsigned char)hash[i]);
    }
    printf("\n");
    delete[] hash;
    return 0;
}