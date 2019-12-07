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
	NOMIR
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
	enum mirror mir;
}cartridge;

//read which mirroring is used
enum mirror mirroringcheck(cartridge *cart);

//read if there is battery ram
bool batteryramcheck(cartridge *cart);

//read if there is a trainer
bool trainercheck(cartridge *cart);

//read what mapper is used
int mapperinfo(cartridge *cart);

//cart loading function
void loadcart(cartridge* cart, char* file);

#endif