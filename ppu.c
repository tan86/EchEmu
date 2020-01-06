#include <stdint.h>
#include <stddef.h>
#include "ppu.h"
#include "main.h"
#include "cpu.h"
//RGB values for our color palette
uint8_t color_palette[64][3]={
	84,  84,  84,
	0,   30,  116,
	8,   16,  144,
	48,  0,   136, 
	68,  0,   100,
	92,  0,   48,
	84,  4,   0,
	60,  24,  0, 
	32,  42,  0,
	8,   58,  0,
	0,   64,  0,
	0,   60,  0,
	0,   50,  60,
	0,   0,   0,
	0,   0,   0,
	0,   0,   0,
	152, 150, 152,
	8,   76,  196,
	48,  50,  236,
	92,  30,  228,
	136, 20,  176,
	160, 20,  100,
	152, 34,  32,
	120, 60,  0,
	84,  90,  0,
	40,  114, 0,
	8,   124, 0,
	0,   118, 40,
	0,   102, 120,
	0,   0,   0,
	0,   0,   0,
	0,   0,   0,
	236, 238, 236,
	76,  154, 236, 
	120, 124, 236, 
	176, 90,  236,
	228, 84,  236,
	236, 88,  180,
	236, 106, 100,
	212, 136, 32,
	160, 170, 0,
	116, 196, 0,
	76,  208, 32,
	56,  204, 108,
	56,  180, 204,
	60,  60,  60,
	0,   0,   0,
	0,   0,   0,
	236, 238, 236,
	168, 204, 236,
	188, 188, 236,
	212, 178, 236,
	236, 174, 236, 
	236, 174, 212, 
	236, 100, 176,
	228, 196, 144,
	204, 210, 120,
	180, 222, 120,
	168, 226, 144,
	152, 226, 180,
	160, 214, 228,
	160, 162, 160,
	0,   0,   0,
	0,   0,   0
};

static uint8_t patternshiftreg1, patternshiftreg2;
static uint16_t attribshiftreg1, attribshiftreg2, tile_addr, attribute_addr;
/*************X/Y increments******************/

void incrementx(ppu* PPU){
	uint16_t v = PPU->vramaddr;
	if((v & 0x001F)==31){
		v &= ~0x001F;
		v ^= 0x0400;
	}
	else
		v+=1;
	PPU->vramaddr = v;
}

void incrementy(ppu* PPU){
	uint16_t v = PPU->vramaddr;
	if((v & 0x7000) != 0x7000)
		v+=0x1000;
	else{
		v &= ~0x7000;
		int y = (v & 0x03E0) >> 5;
		if(y == 29){
			y = 0;
			v ^= 0x8000;
		}
		else if (((v & 0x3E0)>>5) == 31)
			y=0;
		else
			y+=1;
		PPU->vramaddr= (v & ~0x03E0) | (y<<5);
	}
}

#define INCX() incrementx(PPU);
#define INCY() incrementy(PPU);

/***************TILES*********************/
//to extract the needed attribute info
uint8_t extractattribute(uint16_t addr, uint8_t data){
	uint8_t portion = (addr >> 1) & 0x1;
	portion |= (((addr >> 6) & 0x1) << 1);
	switch (portion){
		case 0x0:
			return data & 0x3;
		case 0x1:
			return (data >> 2) & 0x3;
		case 0x2:
			return (data >> 4) & 0x3;
		case 0x3:
			return (data >> 6) & 0x3;
	}
}

uint16_t fetchpalettedata(ppu* PPU, uint16_t tile_addr){
	uint8_t ntbyte = ppu_readb(PPU->ppumap, tile_addr);
	uint16_t palettedata, paletteaddr;
	paletteaddr = ((PPU->registers.PPUCTRL & (1<<4))<<8) | (PPU->vramaddr >> 12) | (ntbyte << 4);
	palettedata = ppu_readb(PPU->ppumap, paletteaddr);
	palettedata |= ppu_readb(PPU->ppumap, paletteaddr + 8) << 8;
	return palettedata;
}
 uint8_t attributedata(ppu* PPU, uint16_t attr_addr){
 	uint8_t attrdata = ppu_readb(PPU->ppumap, attr_addr);
 	return attrdata;
}
uint8_t fetchpixel(ppu* PPU, uint16_t tile_addr, uint16_t attribute_addr){
	uint16_t paldata = fetchpalettedata(PPU, tile_addr);
	if(!paldata) return 0x0;
	uint8_t attrdata = attributedata(PPU,attribute_addr);
	attrdata = extractattribute(tile_addr, attrdata);
	uint8_t pixel = paldata>>(8-FINEX-1) & 1;
	pixel |= (paldata>>(16-FINEX-1) & 1) << 1;
	pixel |= attrdata << 2;
	return pixel;
}

/******************************************/

void ppu_run(ppu* PPU){
	if(PPU->tick==0 && (PPU->scanline==261)){
		if(PPU->oddframe) PPU->tick++;
		PPU->tempfinex = PPU->finex;
		if(PPU->registers.PPUMASK & (1<<3)){
			PPU->vramaddr = PPU->tempvramaddr;
			PPU->registers.PPUSTATUS=0x0;
		}
	}
	if(PPU->scanline == 240 && PPU->tick == 1){
		PPU->drawflag=1;
		if(PPU->registers.PPUMASK & (1<<3))
			PPU->vramaddr = PPU->tempvramaddr;
		PPU->registers.PPUSTATUS |= (1<<7);
		if(PPU->registers.PPUCTRL & (1<<7)) PPU->innmi=1;
		PPU->framecount++;
	}
	else if((PPU->scanline < 240) && (PPU->tick < 257) && (PPU->scanline >= 0) && (PPU->tick > 0)){
		//render graphics
		if(SHOWBG){
			if(PPU->finex == 8 || PPU->tick == 1){
				if(PPU->finex ==8){
					PPU->finex=0;
				}
				tile_addr= 0x2000 | (PPU->vramaddr & 0x0FFF);
				attribute_addr= 0x23C0 | (PPU->vramaddr & 0x0C00) | ((PPU->vramaddr >>4) & 0x38) | ((PPU->vramaddr >>2) & 0x07);
				/*fetchtile(PPU,tile_addr,attribute_addr);*/
				PPU->vramaddr++;
			}
			uint8_t pixel = fetchpixel(PPU, tile_addr, attribute_addr);
			PPU->gfx[(PPU->scanline)*256 + (PPU->tick -1)] = color_palette[PALETTE(pixel)][2] | (color_palette[PALETTE(pixel)][1]<<8) | (color_palette[PALETTE(pixel)][0]<<16);
			PPU->gfx[(PPU->scanline)*256 + (PPU->tick - 1)] |= 0xFF<<24;
			PPU->finex++;
		}
	}
	else if((PPU->tick==257)&&(PPU->scanline >= 0) && (PPU->scanline < 240)) {
		/*PPU->vramaddr &= ~(0x41F);
		PPU->vramaddr |= ((PPU->tempvramaddr & 0x400) | (PPU->tempvramaddr & 0x1F));*/
		PPU->registers.OAMADDR=0x0;
		if(PPU->registers.PPUMASK & (1<<3)){
			PPU->vramaddr--;		//an additional addition is made during horizontal transition, overflowing to the coarse Y
			if((PPU->vramaddr & 0x7000) == 0x7000){
				PPU->vramaddr &= ~(0x701F);
				PPU->vramaddr += 0x20;
				PPU->vramaddr |= (PPU->tempvramaddr & 0x1F);
			}
			else{
				PPU->vramaddr += 0x1000;
				PPU->vramaddr &= ~(0x1F);
				PPU->vramaddr |= (PPU->tempvramaddr & 0x1F);
			}
			PPU->finex = PPU->tempfinex;
		}
	}
	PPU->tick++;
	if(PPU->tick == 340){
		PPU->scanline++;
		PPU->tick = 0;
	}
	if(PPU->scanline == 262) {
		PPU->scanline = 0;
		PPU->oddframe = !(PPU->oddframe);
		PPU->registers.PPUSTATUS = 0x0;
		/*PPU->vramaddr &= ~(0x7BE0);
		PPU->vramaddr |= (PPU->tempvramaddr & 0x7BE0);*/
	}
}

void ppu_reset(ppu* PPU){
	PPU->registers.PPUCTRL = 0x0;
	PPU->registers.PPUMASK = 0x0;
	PPU->registers.PPUSTATUS = 0x0 | (PPU->registers.PPUSTATUS & (1<<7));
	PPU->registers.PPUSCROLL=0x0;
	PPU->registers.PPUDATA=0x0;
}

void ppu_init(ppu *PPU){
	PPU->registers.PPUCTRL = 0x0;
	PPU->registers.PPUMASK = 0x0;
	PPU->registers.PPUSTATUS = 0x0;
	PPU->registers.OAMADDR = 0x0;
	PPU->registers.PPUSCROLL=0x0;
	PPU->registers.PPUADDR=0x0;
	PPU->registers.PPUDATA=0x0;
	PPU->scanline=0x0;
	PPU->innmi =0;
	PPU->finex=0;
	PPU->framecount=0;
	PPU->tick=0;
}

uint8_t ppu_read_reg(memmap* map, uint16_t addr){
	ppu* PPU = (ppu*) &(((nes*)map[5].pointer)->PPU);
	VRAMADDR = VRAMADDR & 0x3FFF;
	uint8_t byte;
	switch(addr & 0x7){
		case 0x0:
			break;
		case 0x1:
			break;
		case 0x2:
			PPU->toggle=0;
			if(VBLANK){
				PPU->registers.PPUSTATUS &= ~(1<<7);
				return (PPU->registers.PPUSTATUS | (1<<7));
			}
			return PPU->registers.PPUSTATUS;
			break;
		case 0x3:
			break;
		case 0x4:
			break;
		case 0x5:

			break;
		case 0x6:
			break;
		case 0x7:
			if(VRAMADDR < 0x3F00){
				byte=PPU->readbuffer;
				PPU->readbuffer=ppu_readb(PPU->ppumap,VRAMADDR);
				if(!RENDER){
					if((PPU->registers.PPUCTRL) & 0x4) PPU->vramaddr += 32;
					else PPU->vramaddr++;
				}
				else{
					INCX();
					INCY();
				}
				return byte;
			}
			else{
				PPU->readbuffer=ppu_readb(PPU->ppumap,(VRAMADDR & 0x0EFF) | 0x2000);
				byte = ppu_readb(PPU->ppumap,VRAMADDR);
				if(!RENDER){
					if((PPU->registers.PPUCTRL) & 0x4) PPU->vramaddr += 32;
					else PPU->vramaddr++;
				}	
				else{
					INCX();
					INCY();
				}
				return byte;
			}
			break;
	}
}

void ppu_write_reg(memmap* map, uint16_t addr, uint8_t byte){
	ppu* PPU = (ppu*) &(((nes*)map[5].pointer)->PPU);
	switch(addr & 0x7){
		case 0x0:
			PPU->tempvramaddr &= ~(0x30 <<10);
			PPU->tempvramaddr |= (byte & 0x3)<<10;
			PPU->registers.PPUCTRL = byte;
			break;
		case 0x1:
			PPU->registers.PPUMASK = byte;
			break;
		case 0x2:
			break;
		case 0x3:
			break;
			PPU->registers.OAMADDR = byte;
		case 0x4:
			PPU->OAMpointer[PPU->registers.OAMADDR] = byte;
			break;
		case 0x5:
			if(PPU->toggle){
				PPU->tempvramaddr &= ~(0x39f <<5);
				PPU->tempvramaddr |= (0xF8 & byte)<<2;
				PPU->tempvramaddr |= (0x7 & byte)<<12;
				PPU->toggle = !(PPU->toggle);
				PPU->registers.PPUSCROLL = byte;
			}
			else {
				PPU->tempvramaddr &= ~(0x1f);
				PPU->tempvramaddr |= (byte >>3);
				PPU->finex &= ~(0x7);
				PPU->finex = (byte & 0x7);
				PPU->toggle = !(PPU->toggle);
				PPU->registers.PPUSCROLL = byte;
			}
			break;
		case 0x6:
			if (PPU->toggle){
				PPU->tempvramaddr &= ~(0xFF);
				PPU->tempvramaddr |= byte;
				PPU->vramaddr = PPU->tempvramaddr;
				PPU->toggle = !PPU->toggle;
				PPU->registers.PPUADDR = byte;
			}
			else {
				PPU->tempvramaddr &= ~(0x3F << 8);
				PPU->tempvramaddr |= (byte & 0x3F)<<8;
				PPU->tempvramaddr &= 0x3fff;
				PPU->toggle = !PPU->toggle;
				PPU->registers.PPUADDR = byte;
			}
			break;
		case 0x7:
			ppu_writeb(PPU->ppumap, VRAMADDR, byte);
			if(!RENDER){
				if((PPU->registers.PPUCTRL) & (1<<2)) 
					VRAMADDR+=32;
				else VRAMADDR++;
			}
			else{
				INCX();
				INCY();
			}
			break;
	}
}