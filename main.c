#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include "main.h"
#include "cpu.h"
#include <stdbool.h>
#include "memory.h"
/*
.
.
.		More includes here later
.
.
.
*/

const int SCREEN_HEIGHT=240;
const int SCREEN_WIDTH=256;

int main(int argc, char *argv[]){
	if (argc!=2){
		printf("./testnes <ROM file path>\n");
		return 1;
	}

	nes* NES = (nes*)malloc(sizeof(nes));
	init_nes(NES,argv[1]);
	/*for(int i=0; i<8991; i++){
		printf("%d ", i);
		cpu_run(&(NES.CPU));
	}
	printf("\nOK!");/**/
    SDL_Window* window = NULL;

    if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        exit(1);
    }

    window = SDL_CreateWindow("NESEmu",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL){
        printf( "Window could not be created! SDL_Error: %s\n",
                SDL_GetError() );
        exit(2);
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    
    SDL_Texture* sdlTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);

    const uint8_t *state = SDL_GetKeyboardState(NULL);

	for(int i=0;;i++){
		if(DEBUG) printf("%d %d %d %d ",i, NES->PPU.framecount,NES->PPU.scanline,NES->PPU.tick);
		
		//one cpu run 
		cpu_run(&(NES->CPU));

		//poll events if controller's strobe is high
		if(NES->controller1.strobe)
			poll_input(&(NES->controller1), state);
		if(state[SDL_SCANCODE_ESCAPE]){
			free_nes(NES);
		}

		//run the PPU 3 times of total CPU cycles
		for(int i=0; i < (3 * (NES->CPU.clockcount)); i++){
			ppu_run(&(NES->PPU));
			if(NES->PPU.innmi){
				NES->CPU.innmi=1;
				NES->PPU.innmi=0;
			}
			if(NES->PPU.drawflag){
				NES->PPU.drawflag=0;
				SDL_UpdateTexture(sdlTexture,NULL,NES->PPU.gfx,256*sizeof(uint32_t));
				SDL_RenderClear(renderer);
				SDL_RenderCopy(renderer,sdlTexture,NULL,NULL);
				SDL_RenderPresent(renderer);
			}
		}
	}
	SDL_DestroyTexture(sdlTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	return 0;
}

void init_nes(nes* NES, char* game){
	/*load the file and it's contents*/

	memset(NES, 0, sizeof(nes));

	loadcart(&(NES->cart),game);

	cpu_init(&(NES->CPU));
	ppu_init(&(NES->PPU));

	NES->CPU.RAM = (char*)malloc(0x2000 * sizeof(char));

	registermem(&(NES->CPU.cpumap[0]),2048,		0x0000,NES->CPU.RAM,				RAMT	  );
	registermem(&(NES->CPU.cpumap[1]),	 8,   	0x2000,&(NES->PPU.registers),		PPU_REG	  );
	registermem(&(NES->CPU.cpumap[2]),0x20,		0x4000,NES->controlregs,			CONTROLREG);
	registermem(&(NES->CPU.cpumap[3]),2*16*16*16,0x6000,NES->cart.batteryrampointer,RAMT	  );
	registermem(&(NES->CPU.cpumap[4]),(NES->cart.prgromno)*4*16*16*16,0x8000,NES->cart.prgrompointer,	RAMT	  );

	NES->CPU.PrgCount = cpureadw(NES->CPU.cpumap,NES->CPU.PrgCount);

	/*a pointer to NES structure to reference to other objects*/
	NES->CPU.cpumap[5].pointer 	= (char *)NES;
	NES->CPU.cpumap[5].start 	= 0;
	NES->CPU.cpumap[5].size 	= 0;
	NES->CPU.cpumap[5].type 	= RAMT;
	/**************************************/

	NES->PPU.nametable 	= (uint8_t*)malloc(4*16*16*16*sizeof(uint8_t));
	NES->PPU.gfx 		= (uint32_t*)malloc(256*240*sizeof(uint32_t));
	NES->PPU.palette 	= (uint8_t*)malloc(0x20*sizeof(uint8_t));
	NES->PPU.OAMdata	= (oamtype*)malloc(64*4*sizeof(uint8_t));
	NES->PPU.OAMpointer = (uint8_t*)(NES->PPU.OAMdata);
	
	memset(NES->PPU.nametable,	0,	4*16*16*16);
	memset(NES->PPU.gfx,		0,	256*240*sizeof(uint32_t));
	memset(NES->PPU.palette,	0,	0x20*sizeof(uint8_t));

	registermem(&((NES->PPU).ppumap[0]),0x2000, 0x0000,NES->cart.chrrompointer,RAMT);
	if(NES->cart.mir == NOMIR){
		registermem(&((NES->PPU).ppumap[1]),0x400, 0x2000,(NES->PPU.nametable),RAMT);
		registermem(&((NES->PPU).ppumap[2]),0x400, 0x2000,(NES->PPU.nametable),RAMT);
	}
	else if(NES->cart.mir == VERTICALMIR){
		registermem(&((NES->PPU).ppumap[1]),0x800, 0x2000,(NES->PPU.nametable),RAMT);
		registermem(&((NES->PPU).ppumap[2]),0x800, 0x2000,(NES->PPU.nametable),RAMT);
	}
	else if(NES->cart.mir == HORIZONTALMIR){
		registermem(&((NES->PPU).ppumap[1]),0x400, 0x2000,(NES->PPU.nametable),RAMT);
		registermem(&((NES->PPU).ppumap[2]),0x400, 0x2800,(NES->PPU.nametable + 2*16*16*16),RAMT);
	}
	registermem(&((NES->PPU).ppumap[3]),0x20,0x3F00,NES->PPU.palette,RAMT);
}

void free_nes(nes* NES){
	free(NES->CPU.RAM);
	free(NES->PPU.nametable);
	free(NES->PPU.gfx);
	free(NES->PPU.palette);
	if(NES->cart.prgrompointer)		free(NES->cart.prgrompointer);
	if(NES->cart.batteryrampointer)	free(NES->cart.batteryrampointer);
	if(NES->cart.trainerpointer)	free(NES->cart.trainerpointer);
	if(NES->cart.chrrompointer)		free(NES->cart.chrrompointer);
	free(NES);
	exit(0);
}