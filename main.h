#ifndef MAIN_H
#define MAIN_H
#include <stdlib.h>
#include <stdio.h>
#include "cartridge.h"
#include "cpu.h"
#include "memory.h"
#include "ppu.h"
#include "input.h"

typedef struct{

	//Components
	cartridge cart;
	cpu CPU;
	ppu PPU;
	uint8_t controlregs[20];
	controller controller1;
	controller controller2;
	/*apu APU;
	mapper MAPPER;*/

	//Memory locations based stuffs
}nes;
//initializes the NES structure
void init_nes(nes* NES, char* game);

void nes_reset(nes* NES);

#endif 