#ifndef CPU_H
#define CPU_H
#include <stdint.h>

#include "memory.h"
#include "ppu.h"
#include "stdbool.h"

typedef struct {
  uint8_t A;          // accumulator
  uint8_t X, Y;       // multi purpose register
  uint16_t PrgCount;  // program counter
  uint8_t S;          // stack pointer
  uint8_t P;          // flags
  char *RAM;          // Main RAM
  memmap cpumap[6];
  ppu *PPU;
  unsigned int clockcount;
  bool innmi;
} cpu;

void cpu_init(cpu *CPU);          // initialize CPU
void cpu_reset(cpu *CPU);         // reset CPU
void cpu_nmi(cpu *CPU, int nmi);  // nmi triggered
void cpu_irq(cpu *CPU, int irq);  // irq triggered
void cpu_run(cpu *CPU);           // run one CPU clock

void write_control_reg(memmap *memory, uint16_t addr, uint8_t byte);
uint8_t read_control_reg(memmap *memory, uint16_t addr);

#endif