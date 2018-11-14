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

/* function declaration goes here.*/
int parser(char *filename);
int reset_cache(int num);
int LRU_update(int num);
int read (int addr);
int write (int addr);
int snooping (int addr);


typedef struct my_cache
{
	unsigned int addr;
	unsigned int tag;
	unsigned int LRU;
	char MESI;
}my_cache;

my_cache cache[7];

// Parser test
int main(void) {
    
	char *input_file = "input.txt";
    
	if(reset_cache==0)
	{	
		printf("\nfail to reset cache");

	}
	else
	{
		printf("\ncache is reseted");
	}
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
    unsigned int i = 0;


    // Open the file
    FILE *fp = fopen(filename, "r");
    if (!fp)
	printf("\n\tERROR: opening file\n");

    // Read the file
    while (fscanf(fp, "%d %x", &op, &addr) != EOF) {
	if (op==0)
	{
		read(addr);
	}
	else if (op==1)
	{
		write(addr);
	}
	else if (op==4)
	{
		snooping(addr);
	}

//keep the tag
//	cache[i].set = (addr & SETMASK) >> BYTE;
//	cache[i].byte = addr & BYTEMASK;
	i++;
    }

    // Close the file
    fclose(fp);

    return 0;
}
 

int reset_cache(int num)
{
	int i =0;
	while (cache[i]<8)
	{
		cache[i].addr=0;
		cache[i].tag=0;
		cache[i].LRU=0;
		cache[i].MESI=0;
		num++;
	}

	return num;
}

//this fuction will update the cache LRU
//i is for location that is going to have LRU 111 (meaning most recent used)
int LRU_update(int i)
{
	int j =0
	cache[i].LRU=0b111;
	while(

}


int read (int addr)
{
	unsigned int tag;
	int i=0;
	//cast the tag here so we can search the tag hit
	tag = addr >> (BYTE + SET);
	printf("\n%x \n%x",addr, tag);
	//first read  
	while (cache[i].tag==0)
	{	
		i++;
	}
	if (i<8)
	{
		cache[i].tag=tag;
		cache[i].addr=addr;
		cache[i].MESI='E';
		//passing the cache way locatio for LRU_update
		LRU_update(i);
	}
}	



int write(int addr)
{

}

int snooping(int addr)
{


}


