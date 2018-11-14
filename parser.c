// parser.c
//
// Author: Alex Olson
// Class: ECE 585
// Term: Fall 2018
// Group: 11
//
// This file contains the source that parses the input into a command
// as well as decoding the address into Tag, Index, and Byte Offset bits.
//

#include <stdlib.h>
#include <stdio.h>

#define TAG 12
#define SET 14
#define BYTE 6
#define SETMASK 0x000FFFFF
#define BYTEMASK 0x0000003F

struct decoded_addr{
    int op;
    unsigned int tag;
    unsigned int set;
    unsigned int byte;
};


int parser(char*);

// Parser test
int main(void) {
    
    char *input_file = "input.txt";
    
    // Test parsing function
    if (parser(input_file))
	printf("\n\tERROR: parsing file\n");

    return 0;
}


// Parser
int parser(char *filename) {

    char c;
    int op;
    unsigned int addr;
    unsigned int count = 0;
    unsigned int i = 0;
    struct decoded_addr this;

    // Open the file
    FILE *fp = fopen(filename, "r");
    if (!fp)
	printf("\n\tERROR: opening file\n");

    // Count the number of traces
    for (c = getc(fp); c != EOF; c = getc(fp))
	if (c == '\n')
	    ++count;

    // Close the file
    fclose(fp);

    struct decoded_addr trace[count];

    // Open the file
    fp = fopen(filename, "r");
    if (!fp)
	printf("\n\tERROR: opening file\n");

    // Read the file
    while (fscanf(fp, "%d %x", &op, &addr) != EOF) {
	this.op = op;
	this.tag = addr >> (BYTE + SET);
	this.set = (addr & SETMASK) >> BYTE;
	this.byte = addr & BYTEMASK;
	trace[i++] = this;
    }

    // Close the file
    fclose(fp);

    // DEBUG check the trace file
    for (i = 0; i < count; ++i)
	printf("\n\top: %d\ttag: %x\tset: %x\tbyte: %x\n", 
	    trace[i].op, trace[i].tag, trace[i].set, trace[i].byte);

    return 0;
}


