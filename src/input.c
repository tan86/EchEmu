#include "input.h"
#include <SDL2/SDL.h>
#include "main.h"
char keymap[8]={
	SDL_SCANCODE_K,
	SDL_SCANCODE_L,
	SDL_SCANCODE_V,
	SDL_SCANCODE_B,
	SDL_SCANCODE_W,
	SDL_SCANCODE_S,
	SDL_SCANCODE_A,
	SDL_SCANCODE_D,
};

uint8_t read_controller(memmap* memory, uint16_t addr){
	uint8_t data = 0x0;
	controller* J1 = &(((nes*)(memory[5].pointer))->controller1);
	controller* J2 = &(((nes*)(memory[5].pointer))->controller2);
	switch(addr & 0x1F){
		case 0x16:
			data |= (J1->data0 >> (J1->counter)) & 0x1;
			data |= ((J1->data1 >> (J1->counter)) & 0x1)<<1;
			data |= ((J1->data2 >> (J1->counter)) & 0x1)<<2;
			data |= ((J1->data3 >> (J1->counter)) & 0x1)<<3;
			data |= ((J1->data4 >> (J1->counter)) & 0x1)<<4;
			J1->counter++;
			return data;
		case 0x17:
			data |= (J2->data0 << (J2->counter)) & 0x1;
			data |= ((J2->data1 << (J2->counter)) & 0x1)<<1;
			data |= ((J2->data2 << (J2->counter)) & 0x1)<<2;
			data |= ((J2->data3 << (J2->counter)) & 0x1)<<3;
			data |= ((J2->data4 << (J2->counter)) & 0x1)<<4;			
			J2->counter++;
			return data;
	}
}

void write_controller(memmap* memory, uint16_t addr, uint8_t data){
	controller* J1 = &(((nes*)(memory[5].pointer))->controller1);
	controller* J2 = &(((nes*)(memory[5].pointer))->controller2);

	J1->strobe = data & 0x1; J1->counter = 0;
	J2->strobe = data & 0x1; J2->counter = 0;
}

void poll_input(controller* pad, uint8_t* state){
	//SDL_PumpEvents();
	pad->data0 = 0x0;
	for(int i=0; i<8; i++){
		pad->data0 |= state[keymap[i]] * (1<<i);
	}
}