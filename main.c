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

char keymap[]={}; //to be written

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

    
    SDL_Texture* sdlTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	for(int i=0;;i++){
		printf("%d %d %d %d ",i, NES->PPU.framecount,NES->PPU.scanline,NES->PPU.tick);
		cpu_run(&(NES->CPU));
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
	/*while(true){
		//cycle

		SDL_event e;
		while(SDL_PollEvent(&e)){
			if(e.type==SDL_Quit) exit(0);

			if(e.type==SDL_KEYDOWN){
				if(e.key.keysym.sym==SDLK_ESCAPE) exit(0);

				for(int i=0; i<16; i++){
					if(e.key.keysym.sym==keymap[i]) //joypad set
				}
			}
			if(e.type==SDL_KEYUP){
				for(int i=0; i<16; i+=){
					if(e.key.keysym.sym==keymap[i]) //joypad clear
				}
			}
		}
 
		//graphics pixels setting here

		SDL_UpdateTexture(sdlTexture, NULL, NES.PPU.gfx, ;
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer,sdlTexture,NULL, NULL);
		SDL_RenderPresetn(renderer);
	}
	SDL_DestroyTexture(sdlTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);/*
	return 0;*/
}

void init_nes(nes* NES, char* game){
	/*load the file and it's contents*/
	char* temp = (char *)malloc(24*sizeof(char));
	loadcart(&(NES->cart),game);
	memset(NES, 0, sizeof(NES));
	cpu_init(&(NES->CPU));
	ppu_init(&(NES->PPU));
	NES->CPU.RAM = (char*)malloc(0x2000 * sizeof(char));
	registermem(&((NES->CPU).cpumap[0]),2048,0x0000,NES->CPU.RAM,RAMT);
	registermem(&(NES->CPU.cpumap[1]),8,0x2000,&(NES->PPU.registers),PPU_REG);
	registermem(&(NES->CPU.cpumap[2]),0x20,0x4000,temp,RAMT);
	registermem(&((NES->CPU).cpumap[3]),2*16*16*16,0x6000,NES->cart.batteryrampointer,RAMT);
	registermem(&((NES->CPU).cpumap[4]),4*16*16*16,0x8000,NES->cart.prgrompointer,RAMT);
	NES->CPU.PrgCount = cpureadw(NES->CPU.cpumap,NES->CPU.PrgCount);

	NES->CPU.cpumap[5].pointer = (char *)&(NES->PPU);
	NES->CPU.cpumap[5].start = 0;
	NES->CPU.cpumap[5].size = 0;
	NES->CPU.cpumap[5].type = RAMT;

	ppu_init(&(NES->PPU));

	NES->PPU.nametable = (uint8_t*)malloc(4*16*16*16*sizeof(uint8_t));
	NES->PPU.gfx = (uint32_t*)malloc(256*240*sizeof(uint32_t));
	NES->PPU.palette = (uint8_t*)malloc(0x20*sizeof(uint8_t));
	memset(NES->PPU.nametable,0,4*16*16*16);
	memset(NES->PPU.gfx,0,256*240*sizeof(uint32_t));
	memset(NES->PPU.palette,0,0x20*sizeof(uint8_t));

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