
#include "memory.h"
#include "cpu.h"
#include "ppu.h"
#include "input.h"

/*curr_mem memdata;
uint8_t temp1,temp2;*/

void registermem(memmap *memory,uint32_t size, uint16_t start, unsigned char *pointer, memtype type){
	memory->size=size;
	memory->start=start;
	memory->pointer=pointer;
	memory->type=type;
}

uint8_t* find_memcpu(memmap* memory, uint16_t addr, memtype* type){
	for(int i=4; i>=0;i--){		//this implementation is supposed to have 5 groups of memory for cpu implementation, another hack for now
		if(memory[i].start<=addr){
			if(type) *type = memory[i].type;
			if(addr==memory[i].start) return memory[i].pointer;
			return memory[i].pointer + ((addr - memory[i].start) % memory[i].size);
		}
	}
}

uint8_t* find_memppu(memmap* memory, uint16_t addr, memtype* type){
	for(int i=5; i>=0;i--){		//this implementation is supposed to have 4 groups of memory for ppu implementation, another hack for now
		if(memory[i].start<=addr){
			if(type) *type = memory[i].type;
			if(addr==memory[i].start) return memory[i].pointer;
			return memory[i].pointer + ((addr - memory[i].start) % memory[i].size);
		}
	}
}

/********following commented functions turned out to be redundant at end, still keeping them here because why not ********/

/*void cpuwritew(memmap* cpumap, uint16_t addr, uint16_t word){
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
}*/

void cpuwriteb(memmap* cpumap, uint16_t addr, unsigned char byte){
	static uint8_t* pointer; static memtype curr_type;
	pointer = find_memcpu(cpumap,addr, &curr_type);
	if(curr_type == PPU_REG)
		ppu_write_reg(cpumap, addr,byte);
	else if(curr_type == CONTROLREG)
		write_control_reg(cpumap, addr, byte);
	if((addr & 0xF000) == 0x6000){
		addr--;
		addr++;
	}
	else *(pointer)=(uint8_t)byte;
}

uint16_t cpureadw(memmap* cpumap, uint16_t addr){
	static uint8_t* pointer; static memtype curr_type;
	static uint8_t temp1,temp2;
	pointer = find_memcpu(cpumap,addr, &curr_type);
	temp1=*(pointer);
	pointer = find_memcpu(cpumap,addr+1, &curr_type);
	temp2=*(pointer);
	return temp1 | (temp2 << 8);
}

uint16_t zpcpureadw(memmap* cpumap, uint16_t addr){	//special zero page case
	static uint8_t* pointer; static memtype curr_type;
	static uint8_t temp1,temp2;
	if(addr == 0xff){
		pointer = find_memcpu(cpumap,addr, &curr_type);
		temp1=*(pointer);
		pointer = find_memcpu(cpumap,0x00, &curr_type);
		temp2=*(pointer);
		return temp1 | (temp2 << 8);
	}
	pointer = find_memcpu(cpumap,addr,&curr_type);
	temp1=*(pointer);
	pointer = find_memcpu(cpumap,addr+1,&curr_type);
	temp2=*(pointer);
	return temp1 | (temp2 << 8);
}

unsigned char cpureadb(memmap* cpumap, uint16_t addr){
	static uint8_t* pointer; static memtype curr_type;
	pointer = find_memcpu(cpumap, addr, &curr_type);
	if(curr_type == PPU_REG)
		return ppu_read_reg(cpumap, addr);		
	else if(curr_type == CONTROLREG)
		return read_control_reg(cpumap, addr);
	else return *(pointer);
}
unsigned char ppu_readb(memmap* cpumap, uint16_t addr){
	static uint8_t* pointer; static memtype curr_type;
	pointer = find_memppu(cpumap, addr, &curr_type);
	if((curr_type == PALETTE) && (!(addr & 0x3))){
		if(addr & 0x10) pointer -= 0x10;
		else pointer -= addr & 0xC;
	}
	return *(pointer);
}

void ppu_writeb(memmap* cpumap, uint16_t addr, unsigned char byte){
	static uint8_t* pointer; static memtype curr_type;
	pointer = find_memppu(cpumap,addr, &curr_type);
	if((curr_type == PALETTE) && (!(addr & 0x3)) && (addr &0x10))
		pointer -= addr & 0x10;
	*(pointer)=(uint8_t)byte;
}
