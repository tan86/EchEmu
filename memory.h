#ifndef MEMORY_H
#define MEMORY_H
#include <stdint.h>

typedef struct{	
	uint32_t size;			//size of memory component
	unsigned char *pointer;		//pointer to memory portion in host machine
	uint16_t start;
}memmap;


//registers all the implemented register and memory which are to be accessed 
void registermem(memmap *memory,uint32_t size, uint16_t start, unsigned char *pointer);

unsigned char* find_memcpu(memmap* memory, uint16_t addr);

unsigned char cpureadb(memmap* cpumap, uint16_t addr);

uint16_t cpureadw(memmap* cpumap, uint16_t addr);

void cpuwriteb(memmap* cpumap, uint16_t addr, unsigned char byte);

void cpuwritew(memmap* cpumap, uint16_t addr, uint16_t word);
#endif