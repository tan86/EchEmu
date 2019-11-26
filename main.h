#ifndef MAIN_H
#define MAIN_H
#include <stdlib.h>
#include <stdio.h>
#include "cartridge.h"
#include "cpu.h"
typedef struct{

	//Components
	cartridge cart;
	cpu CPU;
	/*ppu PPU;
	apu APU;
	mapper MAPPER;*/

	//Memory locations based stuffs
}nes;
//initializes the NES structure
void init_nes(nes* NES, char* game){
	/*load the file and it's contents*/
	char temp[24];
	loadcart(&(NES->cart),game);
	memset(NES, 0, sizeof(NES));
	cpu_init(&(NES->CPU));
	registermem(&((NES->CPU).cpumap[0]),2048,0x0000,NES->CPU.RAM);
	registermem(&(NES->CPU.cpumap[1]),8,0x2000,0x0);
	registermem(&(NES->CPU.cpumap[2]),0x20,0x4000,temp);
	registermem(&((NES->CPU).cpumap[3]),2*16*16*16,0x6000,NES->cart.batteryrampointer);
	registermem(&((NES->CPU).cpumap[4]),4*16*16*16,0x8000,NES->cart.prgrompointer);
}

void nes_reset(nes* NES){
	//reset cpu, ppu and apu
	/*cpu_reset(&(NES->CPU));*/
}

#endif 