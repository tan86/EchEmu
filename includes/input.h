#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>
#include <stdint.h>

#include "memory.h"
typedef struct {
  uint8_t strobe;
  uint8_t data0;
  uint8_t data1;
  uint8_t data2;
  uint8_t data3;
  uint8_t data4;
  uint8_t counter;
} controller;

void write_controller(memmap* memory, uint16_t addr, uint8_t data);

uint8_t read_controller(memmap* memory, uint16_t addr);

void poll_input(controller* pad, uint8_t* state);

#endif