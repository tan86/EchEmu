#ifndef CPU_H
#define CPU_H
#include <stdint.h>
#include "memory.h"
#include "stdbool.h"
#include "ppu.h"
typedef struct{
	uint8_t A;			//accumulator
	uint8_t X,Y;    	//multi purpose register
	uint16_t PrgCount;    	//program counter
	uint8_t S;      	//stack pointer
	uint8_t P;      	//flags
	char* RAM;	//Main RAM
	memmap cpumap[6];
	ppu* PPU;
	unsigned int clockcount;
	bool innmi;
}cpu;

void cpu_init(cpu *CPU);	//initialize CPU
void cpu_reset(cpu *CPU);	//reset CPU
void cpu_nmi(cpu *CPU, int nmi);//nmi triggered
void cpu_irq(cpu *CPU, int irq);//irq triggered
void cpu_run(cpu *CPU); 	//run one CPU clock



#endif