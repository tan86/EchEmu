#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include "main.h"
#include "cpu.h"
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

	nes NES;
	init_nes(&NES,argv[1]);
	for(int i=0; i<8991; i++){
		printf("%d ", i);
		cpu_run(&(NES.CPU));
	}
	printf("\nOK!");
	/*SDL_Window *window = NULL;

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		printf("SDL couldn't be initialized! SDL_Error: %s \n", SDL_GetError());
		exit(1);
	}
	window=SDL_CreateWindow("NES emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINODWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	if(window == NULL){
		printf("Window could not be created SDL_Error : %s\n"
			SDL_GetError() );
		exit(1);
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RendererSetLogicalSIze(renderer, SCREEN_WIDTH,SCREEN_HEIGHT);

	SDL_Texture* texture= SDL_CreateTexture(renderer, SDL_PIXELFROMAT_RGB888,
		SDL_TEXTUREACCESS_STREAMING, );

	while(true){
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

		SDL_UpdateTexture(sdlTexture, NULL, pixels, 64*sizeof());
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer,sdlTexture,NULL, NULL);
		SDL_RenderPresetn(renderer);
	}
	SDL_DestroyTexture(sdlTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);*/
}