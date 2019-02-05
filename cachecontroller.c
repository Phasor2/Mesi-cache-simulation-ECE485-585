// cachecontroller.c
//
// Authors: Alex Olson, Phong Nguyen, Ali Boshehri, Adel Alkharraz, Jennifer Lara
// Class: ECE 585
// Term: Fall 2018
// Group: 11
//
// This file contains the source code for a cache controller of a split 4-way set 
// associative instruction cache and an 8-way set associative data cache.
//
// We made the simplification of only simulating one set of the cache, ergo the set bytes
// of the address are ignored and all reads/writes map to the same set.
//

/* testvector
8 FFFFFFFF
2 974DE100
2 967DE100
2 947DE100
2 967DE100
2 667DE100
2 967DE100
2 967DE100
2 967DE100
0 999DE132
0 999DE132
0 999DE132
0 999DE132
0 999DE132
0 999DE132
0 999DE132
0 999DE132
0 999DE132
0 999DE132
0 999DE132
0 999DE132
0 999DE132
0 999DE132
1 116DE123
1 666DE135
1 333DE12C
0 846DE10C
0 777DE136
1 ABCDE128
0 116DE101
1 100DE101
1 AAADE101
1 EDCDE101
4 0AAADE10
0 999DE132
1 116DE123
1 666DE135
1 333DE12C
4 333DE12C
3 333DE12C
9 FFFFFFFF 
*/
/********************************************************************************
 *  			     DECLARATIONS
 * *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>

#define TAG 12
#define SET 14
#define BYTE 6
#define SETMASK 0x000FFFFF
#define BYTEMASK 0x0000003F

/* TAG SET BYTE caclulations references from addr
 	tag = addr >> (BYTE + SET);
	set = (addr & SETMASK) >> BYTE;
	byte = addr & BYTEMASK;
 */

/* Function declarations */
int parser(char *filename);
void reset_cache_controller(void);
int read(unsigned int addr);
int write(unsigned int addr);
int snooping(unsigned int addr);
void print_cache(void);
int fetch(unsigned int addr);
int invalidate(unsigned int addr);
int matching_tag_data(unsigned int tag);
int matching_tag_inst(unsigned int tag);
int search_LRU_data(void);
int search_LRU_inst(void);
int check_for_invalid_MESI_data(void);
int check_for_invalid_MESI_inst(void);
void LRU_instruction_update(unsigned int set);
void LRU_data_update(unsigned int set);

/* Trace file operations */
typedef enum ops {
    READ  = 0,				// L1 cache read
    WRITE = 1,				// L1 cache write
    FETCH = 2,				// L1 instruction fetch
    INVAL = 3,				// Invalidate command from L2 cache
    SNOOP = 4,				// Data request to L2 cache (response to snoop)
    RESET = 8,				// Reset cache and clear the statistics
    PRINT = 9,				// Print the contents of the cache
}OPS;

/* Cache line set data */
typedef struct my_cache {
    unsigned int tag;			// Tag bits
    unsigned int LRU;			// LRU bits
    char MESI;				// MESI bits
    unsigned char data[64];		// 64 bytes of data
    unsigned int address;		// Address (for reference)
}CACHE;

/* Keep track of the Cache hits and misses */
typedef struct my_stat {
    unsigned int data_cache_hit;	// Cache hit count
    unsigned int data_cache_miss;	// Cache miss count
    unsigned int data_cache_read;	// Data cache read count
    unsigned int data_cache_write;	// Data cache write count
    float data_ratio;			// Data hit/miss ration
    unsigned int inst_cache_hit;	// Cache hit count
    unsigned int inst_cache_miss;	// Cache miss count
    unsigned int inst_cache_read;	// Instruction cache read count
    float inst_ratio;			// Data hit/miss ration
}STAT;

// Global mode setting
unsigned int mode = 2;			// Begin in invalid state for input check

/* Instruction and Data Caches */	// NOTE: list assumption of testing a single set in final report, 16K sets would take unneeded space
CACHE data_cache[8];
CACHE instruction_cache[4];

//For keeping track of Stats
STAT stats;


/********************************************************************************
 *			  CACHE CONTROLLER
 * *****************************************************************************/
int main(int argc, char **argv) {
  
    // Check for command line argument
    if (argc != 2) {
      	printf("\n\tERROR: No input file provided.\n\t\tUsage: ./a.out filename.txt\n");
      	exit(1);
    }
  
    // Initialize the caches once at the beginning
    reset_cache_controller();
  
    // Read in file name from command line
    char *input_file = argv[1];

    // Select a mode
    printf("\n\tMode Selection\n");
    printf("Mode 0: Summary of usage statistics and print contents and state of cache\n");
    printf("Mode 1: Information from Mode 0 and messages to L2.\n");
    do {
	printf("\nMode: ");
	scanf("%u", &mode);
    } while (mode > 1);
   
    // Test parsing function
    if (parser(input_file))
	printf("\n\tERROR: parsing file\n");

    printf("\n\n\n\tClosing Program...\n\n\n\n");

    return 0;
}


/********************************************************************************
 * 			    FUNCTIONS
 * *****************************************************************************/
/* Text file parser
 * Parses ascii data in the format of <n FFFFFFFF> where
 * n is the operation number and FFFFFFFF is the address.
 * Calls appropiate operation based on parsing result.
 *
 * Input: string of .txt trace file to parse
 * Output: pass=0, fail=nonzero
 */
int parser(char *filename) {

    unsigned int op;		// Parsed operation from input
    unsigned int addr;		// Parsed address from input
    FILE *fp;			// .txt file pointer
  
    // Open the file for reading
    if (!(fp = fopen(filename, "r")))
	printf("\n\tERROR: opening file\n");

    // Read the file and decode the operation and address
    while (fscanf(fp, "%d %x", &op, &addr) != EOF) {
	switch(op) {
	    case READ:
		if (read(addr))
		    printf("\n\tERROR: L1 data cache read"); 
            break;

            case WRITE:
		if (write(addr))
		    printf("\n\tERROR: L1 data cache write");
            break;
          
            case FETCH: 
		if (fetch(addr))
		    printf("\n\tERROR: L1 instruction cache fetch");
            break;

            case INVAL: 
		if (invalidate(addr))
		    printf("\n\tERROR: L2 cache invalidate");
            break;
          
	    case SNOOP: 
		if (snooping(addr))
		    printf("\n\tERROR: L2 data request from snoop");
            break;

            case RESET:	reset_cache_controller();
            break;
          
            case PRINT: print_cache();
            break;

	    default: printf("\n\tERROR: invalid trace number\n");
		return -1;
	}
    }
    fclose(fp);	    // Close the file

    return 0;
}
 
        
/* Reset the cache controller and clear statistics
 * 
 * Input: Void
 * Output: Void
 */
void reset_cache_controller(void)
{
    int i;
  
    printf("\n\t Resetting Cache Controller...\n\n");
    
    // Clear the instruction cache
    for (i = 0; i < 4; ++i) {
    	instruction_cache[i].tag = 0;
	instruction_cache[i].LRU = 0;
  	instruction_cache[i].MESI = 'I';
    }

    // Clear the data cache
    for (i = 0; i < 8; ++i) {
    	data_cache[i].tag = 0;
	data_cache[i].LRU = 0;
  	data_cache[i].MESI = 'I';
        data_cache[i].address = 0;
    }
     
    // Reset all statistics
    stats.data_cache_hit = 0;
    stats.data_cache_miss = 0;
    stats.data_cache_read = 0;
    stats.data_cache_write = 0;
    stats.data_ratio = 0.0;
    stats.inst_cache_hit = 0;
    stats.inst_cache_miss = 0;
    stats.inst_cache_read = 0;
    stats.inst_ratio = 0.0;

    return;
}


/* This function will attempt to read a line from the cache
 * On a cache miss, the LRU member is evicted if the cache is full
 * 
 * Input: Address to read from cache
 * Output: Void
 */
int read(unsigned int addr)
{
    unsigned int tag;	// Cache tag decoded from incoming address
    int way = -1;	// Way in the cache set
    int i = 0;

    stats.data_cache_read++;

    // Cast the tag here so we can search the tag hit
    tag = addr >> (BYTE + SET);

    // Check for an empty set in the cache line
    for (i = 0; way < 0 && i < 8; ++i) {
	// Check for an empty set
	if (data_cache[i].tag == 0)
	    way = i;
    }

    // Place the empty position 
    if (way >= 0) {
	data_cache[way].tag = tag;
	data_cache[way].MESI = 'E';
	LRU_data_update(way);
	data_cache[way].address = addr;
	stats.data_cache_miss++;
	
	// Simulate L2 cache read
	if (mode == 1)
	    printf("\n\tRead from L2 %x [data]", addr);
    }
    else {  // no gap then search for hit/miss
	way = matching_tag_data(tag);	// Search for a matching tag first
	
	if (way < 0) {	    // Miss
	    stats.data_cache_miss++;
	    // Check for a line with an invalid state to evict
	    way = check_for_invalid_MESI_data();
	    if (way < 0) { 	// If no invalid states, evict LRU
		way = search_LRU_data();
        	if (way >= 0) {	
		    data_cache[way].tag = tag;
		    data_cache[way].MESI = 'E';
		    LRU_data_update(way);
		    data_cache[way].address = addr;
		}
          	else {
		    printf("LRU data is invalid");
		    return -1;
		}
	    }
	    else {  // Else, evict the invalid member
		data_cache[way].tag = tag;
		data_cache[way].MESI = 'E';
		LRU_data_update(way);  
		data_cache[way].address = addr;
	    }
	
	    // Simulate L2 cache read
	    if (mode == 1)
		printf("\n\tRead from L2 %x [data]", addr);

	}
	else {	// Hit
	    stats.data_cache_hit++;
	    switch (data_cache[way].MESI) {
		case 'M':data_cache[way].tag = tag;
		    data_cache[way].MESI = 'M';
		    LRU_data_update(way);
		    data_cache[way].address = addr;
		break;
	    
		case 'E':data_cache[way].tag = tag;
		    data_cache[way].MESI = 'S';
		    LRU_data_update(way);
		    data_cache[way].address = addr;
		break;
	      
		case 'S':data_cache[way].tag = tag;
		    data_cache[way].MESI = 'S';
		    LRU_data_update(way);
		    data_cache[way].address = addr;
		break;
	    
		case 'I':data_cache[way].tag = tag;
		    data_cache[way].MESI = 'S';
		    LRU_data_update(way);
		    data_cache[way].address = addr;
		break;
            }
        }
    }

    return 0;
}


/* This function will attempt to write an address from the cache.
 * On a cache miss, the LRU member is evicted if the cache is miss.
 * 
 * Input: Address to read from cache
 * Output: Void
 */
int write(unsigned int addr)
{
    unsigned int tag;		// Cache tag from incoming address
    int way = -1;		// Way from the cache set
    int i = 0;

    stats.data_cache_write++;

    // Cast the tag here so we can search the tag hit
    tag = addr >> (BYTE + SET);

    // Check for an empty set in the cache line
    for (i = 0; way < 0 && i < 8; ++i) {
	// Check for an empty set
	if (data_cache[i].tag == 0)
	    way = i;
    }

    // Place the empty position 
    if (way >= 0) {
	data_cache[way].tag = tag;
	data_cache[way].MESI = 'M';
	data_cache[way].address=addr;
	stats.data_cache_miss++;
	LRU_data_update(way);
	
	// Simulate L2 cache write-through
	if (mode == 1)
	    printf("\n\tWrite to L2 %x [write-through]", addr);
    }
    else {		    // no gap then search for hit/miss
	way = matching_tag_data(tag);	// Search for a matching tag first
	if (way < 0) {  // Miss
	    stats.data_cache_miss++;
	    
	    //Simulate L2 cache RFO
	    if (mode == 1)
		printf("\n\tRead for Ownership from L2 %x", addr);

	    way = check_for_invalid_MESI_data();
	    
	    // If no invalid states, evict LRU
	    if (way < 0) { 			
		way = search_LRU_data();
        	if (way >= 0) {
		    
		    //Simulate L2 cache write-back
		    if (mode == 1)
			printf("\n\tWrite to L2 %x [write-back]", addr);

		    data_cache[way].tag = tag;
		    data_cache[way].MESI = 'M';
		    data_cache[way].address=addr;
		    LRU_data_update(way);
		}
		else {
		    printf("ERROR: LRU data is invalid");
		    return -1;
		}
	    }
	
	    // Else, evict the invalid member
	    else {			
		data_cache[way].tag = tag;
		data_cache[way].MESI = 'M';
		data_cache[way].address = addr;
		LRU_data_update(way);        	
	    }
	}
	else {	// Hit
	    stats.data_cache_hit++;
	    switch (data_cache[way].MESI) {
		case 'M':data_cache[way].tag = tag;
		    data_cache[way].MESI = 'M';
		    data_cache[way].address = addr;
		    LRU_data_update(way);
		break;
	    
		case 'E':data_cache[way].tag = tag;
		    data_cache[way].MESI = 'M';
		    data_cache[way].address = addr;
		    LRU_data_update(way);
		break;
	      
		case 'S':data_cache[way].tag = tag;
		    data_cache[way].MESI = 'E';
		    data_cache[way].address = addr;
		    LRU_data_update(way);
		break;
	    
		case 'I':data_cache[way].tag = tag;
		    data_cache[way].MESI = 'E';
		    data_cache[way].address = addr;
		    LRU_data_update(way);
		break;
            }
        }
    }

    return 0;
}


/* Read an instruction in from the instruction cache
 * Note: Since the instruction cache only reads, MESI state M is not possible
 * 
 * Input: address to read
 * Output: pass=0, fail=nonzero
 */
int fetch(unsigned int addr)
{
    unsigned int tag;	// Cache tag decoded from incoming address
    int way = -1;	// Way in the cache set
    int i = 0;

    stats.inst_cache_read++;

    // Cast the tag here so we can search the tag hit
    tag = addr >> (BYTE + SET);

    // Check for an empty set in the cache line
    for (i = 0; way < 0 && i < 4; ++i) {
	// Check for an empty set
	if (instruction_cache[i].tag == 0)
	    way = i;
    }

    // Place the empty position 
    if (way >= 0) {
	instruction_cache[way].tag = tag;
	instruction_cache[way].MESI = 'E';
	instruction_cache[way].address=addr;
	LRU_instruction_update(way);
	stats.inst_cache_miss++;
	
	// Simulate L2 cache read
	if (mode == 1)
	    printf("\n\tRead from L2 %x [Instruction]", addr);
    }
    else { // no gap then search for hit/miss
	way = matching_tag_inst(tag);
	if (way < 0) {  	// Miss
	    stats.inst_cache_miss++;

	    // Check for a line with an invalid state to evict
	    way = check_for_invalid_MESI_inst();
	    if (way < 0) { 	// No invalid states, evict LRU
		way = search_LRU_inst();
		instruction_cache[way].tag = tag;
		instruction_cache[way].MESI = 'E';
		instruction_cache[way].address=addr;
		LRU_instruction_update(way);
	    }
	    else {			// Evict the invalid member
		instruction_cache[way].tag = tag;
		instruction_cache[way].MESI = 'E';
		instruction_cache[way].address=addr;
		LRU_instruction_update(way);        	
	    }
	    
	    // Simulate L2 cache read
	    if (mode == 1)
		printf("\n\tRead from L2 %x [Instruction]", addr);
	}
	else {				// Else, there was a hit
	    stats.inst_cache_hit++;
	    switch (instruction_cache[way].MESI) {
		case 'M':instruction_cache[way].tag = tag;
		    instruction_cache[way].MESI = 'M';
		    instruction_cache[way].address=addr;
		    LRU_instruction_update(way);
		break;

		case 'E':instruction_cache[way].tag = tag;
		    instruction_cache[way].MESI = 'S';
		    instruction_cache[way].address=addr;  
		    LRU_instruction_update(way);
		break;

		case 'S':instruction_cache[way].tag = tag;
		    instruction_cache[way].MESI = 'S';
		    instruction_cache[way].address=addr;
		    LRU_instruction_update(way);
		break;

		case 'I':instruction_cache[way].tag = tag;
		    instruction_cache[way].MESI = 'S';
		    instruction_cache[way].address=addr;
		    LRU_instruction_update(way);
		break;
            }
        }
    }
    return 0;
}
       

/* Invalidate an L2 command
 * 
 * Input: address to invalidate
 * Output: pass=0, fail=nonzero
 *
 * Assumption that L2 is telling L1 that this address needs to be invalidated. 
 * Therefore we just have to set MESI bit to 'I'
 */
int invalidate(unsigned int addr)
{
    int i;
    unsigned int tag = addr >> (BYTE + SET);
    
    // Search data cache for a matching tag
    for (i = 0; i < 8; ++i) {
	if (data_cache[i].tag == tag) {
	    switch (data_cache[i].MESI) {
      		case 'M': data_cache[i].MESI = 'I';	// Changes MESI bit set to invalidate
      		break;
		
		case 'E': data_cache[i].MESI = 'I';	// Changes MESI bit set to invalidate
      		break;

		case 'S': data_cache[i].MESI = 'I';	// Changes MESI bit set to invalidate
		break;

		case 'I': return 0;			// Do nothing, already invalid
		default: return -1;			// Non-MESI state
	    } 
	}
    }
    return 0;
}                      


/* Check the bus for other processors reading/writing addresses of interest
 * 
 * Input: Address of interest
 * Output: pass=0, fail=nonzero
 */     
int snooping(unsigned int addr)
{
    //Total of 4 states, Modified, Exclusive, Shared, Invalid
    //Assumption that L2 is telling L1 that this address needs to be invalidated. 
    //Therefore we just have to set MESI bit to 'I'
    unsigned int tag = addr >> (BYTE + SET);
    int i = 0;

    // Check the data cache of other simulated processors for matching tags
    for (i = 0; i < 8; ++i) {
	if(data_cache[i].tag == tag) {
	    switch (data_cache[i].MESI) {
      		case 'M': data_cache[i].MESI = 'I'; //changes MESI bit set to invalidate
      		case 'E': data_cache[i].MESI = 'I'; //changes MESI bit set to invalidate
      		case 'S': data_cache[i].MESI = 'I'; //changes MESI bit set to invalidate
          	case 'I': return 0; 		    //do nothing, already invalid
            } 
	
	    // Simulate returning data to L2 cache
	    if (mode == 1)
		printf("\n\tReturn data to L2 %x", addr);
	}
    }
    return 0;
}

       
/* This function will update the data cache LRU
 * Anything less than current LRU will increment
 * Anything that greater than current LRU doesn't change
 * MRU: 000   LRU: 111
 * 
 * Input: Set way to start LRU comparison at
 * Output: Void
 */
void LRU_data_update(unsigned int way)
{
    int current_LRU = data_cache[way].LRU;
    
    for (int i = 0; i < 8; ++i) {
	if (data_cache[i].LRU <= current_LRU)
	    data_cache[i].LRU++;
    }
    data_cache[way].LRU = 0;

    return;
}


/* This function will update the instruction cache LRU
 * Anything less than current LRU will increment
 * Anything that greater than current LRU doesn't change
 * MRU: 000   LRU: 111
 * 
 * Input: Set way to start LRU comparison at
 * Output: Void
 */
void LRU_instruction_update(unsigned int way)
{
    int current_LRU = instruction_cache[way].LRU;
    
    for (int i = 0; i < 4; ++i) {
	if (instruction_cache[i].LRU <= current_LRU)
	    instruction_cache[i].LRU++;
    }
    instruction_cache[way].LRU = 0;

    return;
}

                       
/* Find the invalid member to evict
 * 
 * Input: void
 * Output: error = -1 or set index of matching tag
 */
int search_LRU_data(void)
{
    for (int i = 0; i < 8; ++i) {
    	if (data_cache[i].LRU == 0x7)
            return i;
    }
  
    return -1;
}


/* Find the invalid member to evict
 * 
 * Input: void
 * Output: error = -1 or set index of matching tag
 */
int search_LRU_inst(void)
{
    for (int i = 0; i < 4; ++i) {
    	if (instruction_cache[i].LRU == 0x3)
            return i;
    }
  
    return -1;
}


/* Compare two data cache tags for equality
 *
 * Input: Decoded address tag
 * Output: error=-1 or way index of matching tag
 */
int matching_tag_data(unsigned int tag)
{	
    int i = 0;
  
    // Check for matching tags
    while (data_cache[i].tag != tag) {
	i++;
    	if (i > 7)
	    return -1;
    }

    return i;	
}                   


/* Compare two instruction cache tags for equality
 *
 * Input: Decoded address tag
 * Output: error=-1 or way index of matching tag
 */ 
int matching_tag_inst(unsigned int tag)  
{
    int i = 0;
  
    // Check for matching tags
    while (instruction_cache[i].tag != tag) {
	i++;
    	if (i > 3)
	    return -1;
    }
    
    return i;
}


/* Search for Invalid MESI states within a set
 * 
 * Input: void
 * Output: Index of Invalid state, negative if no invalid states
 */                        
int check_for_invalid_MESI_data(void)
{
    for (int i = 0; i < 8; ++i){
	if (data_cache[i].MESI == 'I')
	    return i;
    }
    
    return -1;
}


/* Search for Invalid MESI states within a set
 * 
 * Input: void
 * Output: Index of Invalid state, negative if no invalid states
 */                        
int check_for_invalid_MESI_inst(void)
{
    for (int i = 0; i < 4; ++i) {
	if (instruction_cache[i].MESI == 'I')
	    return i;
    }

    return -1;
}
  
        
/* Print the contents of the current cache lines and statistics data
 * 
 * Input: cache address to print cache line metadata (Tag, LRU, MESI) for
 * Output: pass=0, fail=nonzero
 */ 
void print_cache(void)
{
    //Input argument later is a mode for what to print. For right now mode = 0
    int i = 0; //To go through cache
    
    printf("\n\n\n\t                 DATA CACHE");
    for (i = 0; i < 8; ++i) {
	printf("\n---------------------------Way %d-------------------------------\n",i+1);
	printf(" Address: %x    Tag: %x    LRU: %u    MESI State: %c \n", 
		data_cache[i].address, data_cache[i].tag, data_cache[i].LRU, data_cache[i].MESI);
    }
    printf("\n\n\t                INSTRUCTION CACHE");
    for (i = 0; i < 4; ++i) {
	printf("\n---------------------------Way %d-------------------------------\n",i+1);
	printf(" Address: %x    Tag: %x    LRU: %u    MESI State: %c \n", 
		instruction_cache[i].address, instruction_cache[i].tag, instruction_cache[i].LRU, instruction_cache[i].MESI);
    }

    //Update values for cache hits and misses
    stats.data_ratio = (float)stats.data_cache_hit / (float)stats.data_cache_miss;
    stats.inst_ratio = (float)stats.inst_cache_hit / (float)stats.inst_cache_miss;
    
    printf("\n\n------------------Statistics Information-------------------\n");
    printf(" Data Cache Hits: %u          Data Cache Misses: %u        \tData Cache Hit/Miss Ratio: %f \n Data Cache Reads: %u         Data Cache Writes: %u\n",
	    stats.data_cache_hit, stats.data_cache_miss, stats.data_ratio, stats.data_cache_read, stats.data_cache_write);	
    printf(" Instruction Cache Hits: %u   Instruction Cache Misses: %u \tInstruction Cache Hit/Miss Ratio: %f \n Instruction Cache Reads: %u\n",
	    stats.inst_cache_hit, stats.inst_cache_miss, stats.inst_ratio, stats.inst_cache_read);	
    
    return;
}
  
