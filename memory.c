
#include "memory.h"
#include "cpu.h"

curr_mem memdata;
uint8_t temp1,temp2;

void registermem(memmap *memory,uint32_t size, uint16_t start, unsigned char *pointer, memtype type){
	memory->size=size;
	memory->start=start;
	memory->pointer=pointer;
	memory->type=type;
}

void find_memcpu(memmap* memory, uint16_t addr){
	for(int i=4; i>=0;i--){		//this implementation is supposed to have 5 groups of memory for cpu implementation
		if(memory[i].start<=addr){
			memdata.type=memory[i].type;
			if(addr==memory[i].start) memdata.pointer = memory[i].pointer;
			else memdata.pointer = memory[i].pointer + ((addr - memory[i].start) % memory[i].size);
			return;
		}
	}
}

void find_memppu(memmap* memory, uint16_t addr){
	for(int i=3; i>=0;i--){		//this implementation is supposed to have 4 groups of memory for cpu implementation
		if(memory[i].start<=addr){
			memdata.type=memory[i].type;
			if(addr==memory[i].start) memdata.pointer = memory[i].pointer;
			else memdata.pointer = memory[i].pointer + ((addr - memory[i].start) % memory[i].size);
			return;
		}
	}
}

void cpuwritew(memmap* cpumap, uint16_t addr, uint16_t word){
	(find_memcpu(cpumap,addr));
	*(memdata.pointer)=(uint8_t)(word & 0x00ff);
	(find_memcpu(cpumap,addr+1));
	*(memdata.pointer)=(uint8_t)((word & 0xff00)>>8);
}

void zpcpuwritew(memmap* cpumap, uint16_t addr, uint16_t word){
	if(addr==0xff){
		(find_memcpu(cpumap,0xff));
		*(memdata.pointer)=(uint8_t)(word & 0x00ff);
		(find_memcpu(cpumap,0x00));
		*(memdata.pointer)=(uint8_t)((word & 0xff00)>>8);
	}
	else {
	(find_memcpu(cpumap,addr));
	*(memdata.pointer)=(uint8_t)(word & 0x00ff);
	(find_memcpu(cpumap,addr+1));
	*(memdata.pointer)=(uint8_t)((word & 0xff00)>>8);
	}
}

void cpuwriteb(memmap* cpumap, uint16_t addr, unsigned char byte){
	find_memcpu(cpumap,addr);
	if(memdata.type == PPU_REG){
		ppu_write_reg(cpumap, addr,byte);
	}
	else *(memdata.pointer)=(uint8_t)byte;
}

uint16_t cpureadw(memmap* cpumap, uint16_t addr){
	find_memcpu(cpumap,addr);
	temp1=*(memdata.pointer);
	find_memcpu(cpumap,addr+1);
	temp2=*(memdata.pointer);
	return temp1 | (temp2 << 8);
}

uint16_t zpcpureadw(memmap* cpumap, uint16_t addr){	//special zero page case
	if(addr == 0xff){
		find_memcpu(cpumap,addr);
		temp1=*(memdata.pointer);
		find_memcpu(cpumap,0x00);
		temp2=*(memdata.pointer);
		return temp1 | (temp2 << 8);
	}
	find_memcpu(cpumap,addr);
	temp1=*(memdata.pointer);
	find_memcpu(cpumap,addr+1);
	temp2=*(memdata.pointer);
	return temp1 | (temp2 << 8);
}

unsigned char cpureadb(memmap* cpumap, uint16_t addr){
	find_memcpu(cpumap, addr);
	if(memdata.type == PPU_REG){
		ppu_read_reg(cpumap, addr);		
	}
	else return *(memdata.pointer);
}
unsigned char ppu_readb(memmap* cpumap, uint16_t addr){
	find_memppu(cpumap, addr);
	return *(memdata.pointer);
}

void ppu_writeb(memmap* cpumap, uint16_t addr, unsigned char byte){
	find_memppu(cpumap,addr);
	*(memdata.pointer)=(uint8_t)byte;
}
