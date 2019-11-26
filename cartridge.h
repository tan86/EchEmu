#ifndef CARTRIDGE_H
#define CARTRIDGE_H
/*Size declarations*/

#define PRGROMSIZE 		16*1024			//16kb PRG_ROM
#define CHRROMSIZE 		8*1024			//8kb CHR-ROM
#define BATTERYRAMSIZE 	2*16*16*16		//BATTERY RAM $6000-$7FFF
#define TRAINERSIZE 	2*16*16 		//TRAINER $7000-$7FFF
#define INESHEADERSIZE 	16				//iNES Standard header size

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum mirror{
	HORIZONTALMIR,
	VERTICALMIR,
	FOURSCRMIR,
};

typedef struct{
	char type[4];	//type of rom file, iNES/UNIF/etc
	char prgromno;	//no of PRG ROM banks (16kb, program code)
	char chrromno;	//no of CHR ROM banks (8kb, graphics information)
	char control1;	//each bit tells about rom's usage
	char control2;
	char ramno;		//no of 8kb ram banks
	char reserve[7];
	bool chrrampresent;
	char *batteryrampointer;	//pointers to bind with main NES structure
	char *trainerpointer;
	char *prgrompointer;
	char *chrrompointer;
}cartridge;

//read which mirroring is used
enum mirror mirroringcheck(cartridge *cart){
	if(cart->control1 & (1<<3)) return FOURSCRMIR;
	else if(cart->control1 & (1<<0)) return VERTICALMIR;
	else return HORIZONTALMIR;
}

//read if there is battery ram
bool batteryramcheck(cartridge *cart){
	return (cart->control1 & (1<<1));
}

//read if there is a trainer
bool trainercheck(cartridge *cart){
	return (cart->control1 & (1<<2));
}

//read what mapper is used
int mapperinfo(cartridge *cart){
	return ((cart->control1 & 0xf0)>>4) | ((cart->control2& 0xf0)>>0);
}

//cart loading function
void loadcart(cartridge* cart, char* file){
	FILE* game=fopen(file,"rb");
	if (!game){
		printf("Can't open the file %s \n", file);
		exit(1);
	}
	fread(cart, INESHEADERSIZE, 1, game);
	char tempsign[4]={'N','E','S',0x1a};
	if(!(strcmp(cart->type,tempsign))){
		printf("Invalied File!!\n");
		exit(1);
	}
	if(batteryramcheck(cart)){
		if(!(cart->batteryrampointer=(char*)malloc(BATTERYRAMSIZE))){
			printf("Failed allocation for battery ram\n");
			exit(1);
		}
	}
	if(trainercheck(cart)){
		cart->trainerpointer=(char*)malloc(TRAINERSIZE);
		if(!cart->trainerpointer){ 
			printf("Failed allocation memory for trainer\n");
			exit(1);
		}
		else fread(cart->trainerpointer,TRAINERSIZE,1, game);
	}
	if(cart->prgromno > 0){
		cart->prgrompointer=(char*) malloc (cart->prgromno * PRGROMSIZE);
		if(!cart->prgrompointer){
			printf("Failed allocation for PRG-ROM\n");
			exit(1);
		}
		else fread(cart->prgrompointer,cart->prgromno*0x4000,1,game);
	}

	if(cart->chrromno) cart->chrrampresent=0;
	else{
		cart->chrrampresent=1;
		cart->chrromno=1;
	}
	cart->chrrompointer=(char*) malloc(cart->chrromno * CHRROMSIZE);
	if(!cart->chrrompointer){
		printf("Failed allocation for CHRROM\n");
		exit(1);
	}
	
	if(cart->chrrampresent) memset(cart->chrrompointer,0,
		cart->chrromno*0x2000);
	else fread(cart->chrrompointer,cart->chrromno*CHRROMSIZE,1,game);
	fclose(game);
}

#endif