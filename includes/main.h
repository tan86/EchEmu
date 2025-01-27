#ifndef MAIN_H
#define MAIN_H
#include <stdio.h>
#include <stdlib.h>

#include "cartridge.h"
#include "cpu.h"
#include "input.h"
#include "memory.h"
#include "ppu.h"

typedef struct {
  // Components
  cartridge cart;
  cpu CPU;
  ppu PPU;
  uint8_t controlregs[20];
  controller controller1;
  controller controller2;
  /*apu APU;*/
} nes;
// initializes the NES structure
void init_nes(nes* NES, char* game);

void nes_reset(nes* NES);

void free_nes(nes*);

#endif