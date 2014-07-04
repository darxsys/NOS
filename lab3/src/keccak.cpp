/**
    FER-CS-NOS
    keccak.cpp
    Implements the SHA3-256 algorithm.

    @author Dario Pavlovic
*/

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#include "utils.h"
#include "keccak.h"

/**
    Inputs message from the file and adds the needed padding
    @param path to the file
    @return padded input message read from file and size of the padded message
*/
pair<llint*, int> input_and_pad(char* filename) {
    char *message;
    ifstream is(filename, ifstream::binary);

    if (is) {
        bool padded = false;
        is.seekg(0, is.end);
        int len = is.tellg();
        int pad_len = len * BYTE;
        is.seekg(0, is.beg);

        // calculate stuff for padding
        if (len % R != 0 || len == 0) {
            // if blocks are not multiple of 128 bytes
            padded = true;
            int tail_len = (len * BYTE) % RBIT;
            if (tail_len == RBIT - 2) {
                // only 11 needs to be added
                pad_len += 2;
            } else if(tail_len == RBIT - 1) {
                // worst case, we add RBIT + 1
                pad_len += 1 + RBIT;
            } else {
                pad_len += RBIT - tail_len;
            }
        }
        pad_len /= BYTE;
        // printf("PAD LEN:%d\n", pad_len);
        // printf("LEN:%ld\n", len);

        message = new char[pad_len];
        is.read(message, len);
        is.close();

        if (padded) {
            if (len == pad_len - 1) {
                message[len] = 0x01 | 0x80;
            } else {
                // first added byte
                message[len] = 0x01;
                // last added byte
                message[pad_len-1] = 0x80;

                for(int i = len+1; i < pad_len-1; i++) {
                    message[i] = 0;
                }
            }
        }

        // transform the message to llint and reverse it at the same time
        llint* msg = new llint[pad_len/BYTE];
        int cutoff = 255;

        for (int i = 0; i < pad_len/BYTE; i++) {
            llint store = 0;
            int start = i * 8;
            for (int j = start; j < start + 8; j++) {
                llint in = message[j] & cutoff;
                store |= (in << (j-start) * 8);
            }
            msg[i] = store;
        }

        delete[] message;
        return make_pair(msg, pad_len/BYTE);
    } else {
        return make_pair((llint*)NULL, -1);
    }
}

/**
    Returns the number with reverse byte order.
    @param number
    @return number with reversed bytes
*/
llint reverse_byte_order(llint value) {
     llint retval;
     retval = value & 0xFF;
     retval = (retval<<8) | ((value >>8) & 0xFF);
     retval = (retval<<8) | ((value >>16) & 0xFF);
     retval = (retval<<8) | ((value >>24) & 0xFF);
     retval = (retval<<8) | ((value >>32) & 0xFF);
     retval = (retval<<8) | ((value >>40) & 0xFF);
     retval = (retval<<8) | ((value >>48) & 0xFF);
     retval = (retval<<8) | ((value >>56) & 0xFF);

     return retval;
}

/**
    Helper function to print out the current state.
    @param state
    @return
*/
void print_state(llint *state) {
    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 5; j++) {
            printf("%lx ", state[i*5 + j]);
        }
        printf("\n");
    }
}

/**
    Initializes a state to all zeroes.
    @param state to be initialized. 5x5 matrix of llint
*/
llint* initialize_state(llint *state) {
    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 5; j++) {
            state[i*5 + j] = 0;
        }
    }

    return state;   
}

/**
    Xors a state given as 2D matrix and the block given as char array.
    @param state - 2D matrix of state
        block - ll array block
    @return state xored with block
*/
llint* xor_state_block(llint *state, llint *block) {
    for(int i = 0; i*64 < RBIT; i++) {
        state[(i/5)*5 + i%5] ^= block[i];
    }

    return state;
}

/**
    Helper function that does rotation of a number to the left.
    @param value
    @param shift - how much
    @return rotated number
*/
llint rotl(const llint value, int shift) {
    if ((shift &= sizeof(value) * 8 - 1) == 0)
      return value;
    return (value << shift) | (value >> (sizeof(value) * 8 - shift));
}

/**
    Implements the theta step of the algorithm.
    @param state
    @return state
*/
llint* theta_step(llint *state) {
    llint* C = new llint[5];
    for (int j = 0; j < 5; j++) {
        llint temp = 0;
        for(int i = 0; i < 5; i++) {
            temp ^= state[5*i + j];
        }
        C[j] = temp;
    }

    llint* D = new llint[5];
    for (int j = 0; j < 5; j++) {
        D[j] = C[(j+5-1)%5] ^ rotl(C[(j+1)%5], 1);
    }

    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 5; j++) {
            state[5*i + j] ^= D[j];
        }
    }

    delete[] C;
    delete[] D;
    return state;
}

/**
    Implements the rho, pi and hi step of the algorithm.
    @param state
    @return state
*/
llint* rho_pi_hi_step(llint *state) {
    // ro pi
    llint* B = new llint[5*5];
    // B[0] = state[0];
    for(int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            // if (i == 0 && j == 0) continue;
            B[j*5 + ((2*i + 3*j)%5)] = rotl(state[j*5 + i], rot[j][i]);
        }
    }

    // hi
    for (int i = 0; i < 5; i++) { 
        for (int j = 0; j < 5; j++) {
            state[j*5 + i] = B[i*5 + j] ^ (~(B[((i+1)%5)*5 + j]) & B[((i+2)%5)*5 + j]);
        }

    }

    delete[] B;
    return state;
}

/**
    Implements the iota step of the algorithm.
    @param state
    @return state
*/
llint* iota_step(llint *state, int i) {
    // ro pi
    state[0] ^= RC[i];
    return state;
}

/**
    Does one round of keccak permutations.
    @param state 
        round_constant
    @return state
*/
llint* keccak_round(llint *state, int index) {
    state = theta_step(state);
    state = rho_pi_hi_step(state);
    state = iota_step(state, index);

    return state;
}

/** 
    Does the invocation of one keccak sponge round 24 times.
    @param state - current state
    @return state after doing the 24 steps
*/
llint* keccak_f(llint *state) {
    for(int i = 0; i < 24; i++) {
        state = keccak_round(state, i);
    }

    return state;
}

/**
    Calls all the necessary keccak functions after initializing current state.
    This one is exposed for other programs to use.
    @param path to the message
    @return Hash of the message
*/
char* keccak(char *filename) {
    pair<llint*, int> message = input_and_pad(filename);
    if (message.first == NULL) {
        printf("Error while opening input file.\n");
        exit(1);
    }    
    // for every block do the rounds
    llint* state = new llint[5*5];
    state = initialize_state(state);
    llint* blocks = message.first;
    int len = message.second;
    // for each block
    // absorbing
    for(int i = 0; i < len; i += BLOCK_SIZE) {
        state = xor_state_block(state, blocks+i);
        // print_state(state);
        state = keccak_f(state);
    }

    //squeezing
    llint* output = new llint[5*5];
    output = initialize_state(output);

    int c = 0;
    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 5; j++) {
            if (5*i + j < RBIT/W) {
                // output[c] ^= state[5*i + j];
                output[c] ^= reverse_byte_order(state[5*i + j]);
                c++;
            }
        }
    }

    // make char
    char *hash = new char[32];
    for(int i = 0; i < 4; i++) {
        int start = 8*i;
        for(int j = 0; j < 8; j++) {
            hash[start+j] = (output[i] >> (56-8*j)) & 0xFF;
        }
    }

    delete[] output;
    delete[] state;
    return hash;
}
