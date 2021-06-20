#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum mirror { HORIZONTALMIR, VERTICALMIR, FOURSCRMIR, NOMIR };

typedef struct {
  char type[4];      // type of rom file, iNES/UNIF/etc
  uint8_t prgromno;  // no of PRG ROM banks (16kb, program code)
  uint8_t chrromno;  // no of CHR ROM banks (8kb, graphics information)
  uint8_t control1;  // each bit tells about rom's usage
  uint8_t control2;
  uint8_t ramno;  // no of 8kb ram banks
  uint8_t reserve[7];
  bool chrrampresent;
  char *batteryrampointer;  // pointers to bind with main NES structure
  char *trainerpointer;
  char *prgrompointer;
  char *chrrompointer;
  enum mirror mir;
} cartridge;

// read which mirroring is used
enum mirror mirroringcheck(cartridge *cart);

// read if there is battery ram
bool batteryramcheck(cartridge *cart);

// read if there is a trainer
bool trainercheck(cartridge *cart);

// read what mapper is used
int mapperinfo(cartridge *cart);

// cart loading function
void loadcart(cartridge *cart, char *file);

#endif
