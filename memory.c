#include "memory.h"

void registermem(memmap *memory,uint32_t size, uint16_t start, unsigned char *pointer){
	memory->size=size;
	memory->start=start;
	memory->pointer=pointer;
}

unsigned char* find_memcpu(memmap* memory, uint16_t addr){
	for(int i=4; i>=0;i--){		//this implementation is supposed to have 5 groups of memory for cpu implementation
		if(memory[i].start<=addr){
			if(addr==memory[i].start) return memory[i].pointer;
			return memory[i].pointer + ((addr - memory[i].start) % memory[i].size);
		}
	}
}

void cpuwritew(memmap* cpumap, uint16_t addr, uint16_t word){
	*(find_memcpu(cpumap,addr))=(uint8_t)(word & 0x00ff);
	*(find_memcpu(cpumap,addr+1))=(uint8_t)((word & 0xff00)>>8);
}

void zpcpuwritew(memmap* cpumap, uint16_t addr, uint16_t word){
	if(addr==0xff){
		*(find_memcpu(cpumap,0xff))=(uint8_t)(word & 0x00ff);
		*(find_memcpu(cpumap,0x00))=(uint8_t)((word & 0xff00)>>8);
	}
	else {
	*(find_memcpu(cpumap,addr))=(uint8_t)(word & 0x00ff);
	*(find_memcpu(cpumap,addr+1))=(uint8_t)((word & 0xff00)>>8);
	}
}

void cpuwriteb(memmap* cpumap, uint16_t addr, unsigned char byte){
	*(find_memcpu(cpumap,addr))=(uint8_t)byte;
}

uint16_t cpureadw(memmap* cpumap, uint16_t addr){
	return (*(find_memcpu(cpumap, addr)) | (*(find_memcpu(cpumap, addr+1)))<<8);
}

uint16_t zpcpureadw(memmap* cpumap, uint16_t addr){	//special zero page case
	if(addr==0xff) return (*(find_memcpu(cpumap, 0xff)) | (*(find_memcpu(cpumap, 0)))<<8);
	return (*(find_memcpu(cpumap, addr)) | (*(find_memcpu(cpumap, addr+1)))<<8);
}

unsigned char cpureadb(memmap* cpumap, uint16_t addr){
	return *(find_memcpu(cpumap,addr));
}