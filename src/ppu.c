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
	236, 180, 176,
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

static inline uint32_t getargbfrompal(ppu* PPU, uint8_t pixel, uint8_t palette){
	uint8_t palent = pixel | (palette << 2);
	return (color_palette[PALETTE(palent)][2]) | 
	(color_palette[PALETTE(palent)][1]<<8) | (color_palette[PALETTE(palent)][0]<<16) | (0xFF << 24);
}

static uint8_t	next_tile_id, next_tile_attr, next_pattern_lsb, next_pattern_msb;
static uint16_t patternshiftreg1, patternshiftreg2, attribshiftreg1, attribshiftreg2,mux;
static uint16_t tile_addr, attribute_addr;
static uint8_t	spritereg1[8], spritereg2[8], spritex[8], spriteattr[8],
 wait,OAMentry, n, m, zerosprflag; //some variables to make handling the sprite evaluation more accurate and easy

/**Random debug variables, should be ignored for most part*/
static uint8_t temp1 =0;
/*******/
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
	if((v & 0x7000) != 0x7000){
		v+=0x1000;
		PPU->vramaddr= v;
	}

	else{
		v &= ~0x7000;
		int y = (v & 0x03E0) >> 5;
		if(y == 29){
			y = 0;
			v ^= 0x0800;
		}
		else if (y == 31)
			y=0;
		else
			y+=1;
		PPU->vramaddr= (v & ~0x03E0) | (y<<5);
	}
}

void horittov(ppu* PPU){
	PPU->vramaddr &= ~(0x041F);
	PPU->vramaddr |= (PPU->tempvramaddr & 0x41F);
}

void vertttov(ppu* PPU){
	PPU->vramaddr &= ~(0x7BE0);
	PPU->vramaddr |= (PPU->tempvramaddr & 0x7BE0);
}

/****Shifter functions***********/
void loadshifters(ppu* PPU){
	patternshiftreg1	= (patternshiftreg1 & 0xFF00) | next_pattern_lsb;
	patternshiftreg2 	= (patternshiftreg2 & 0xFF00) | next_pattern_msb;
	attribshiftreg1 	= (attribshiftreg1 & 0xFF00)  | ((next_tile_attr & 0x1) ? 0xFF : 0x00);
	attribshiftreg2 	= (attribshiftreg2 & 0xFF00)  | ((next_tile_attr & 0x2) ? 0xFF : 0x00);
}

void updateshifters(ppu* PPU){
	if(SHOWBG){
		patternshiftreg1 <<= 1;
		patternshiftreg2 <<= 1;
		attribshiftreg1	 <<= 1;
		attribshiftreg2  <<= 1;
	}
	if(SHOWCHR && PPU->tick >= 0 && PPU->tick <=  256){
		for (int i = 0; i < 8; i++ ){
			if(spritex[i] > 0)
				spritex[i]--;
			else {
				spritereg1[i]<<=1;
				spritereg2[i]<<=1;
			}
		}
	}
}

/******************************************/

void ppu_run(ppu* PPU){
	if((((PPU->scanline >= 0) && (PPU->scanline < 240)) || (PPU->scanline == 261)) && (SHOWBG || SHOWCHR)){
		if(((PPU->tick > 0) && (PPU->tick <= 257)) || ((PPU->tick >= 321) && (PPU->tick <= 337))){
			if(!(PPU->tick == 321 || PPU->tick == 1)) updateshifters(PPU);
			if(PPU->tick!=1)
				switch((PPU->tick - 1)%8){
					case 0:
						loadshifters(PPU);
						next_tile_id = ppu_readb(PPU->ppumap, 0x2000 | (PPU->vramaddr & 0x0FFF));
						break;
					case 2:
						next_tile_attr = ppu_readb(PPU->ppumap, 0x23C0 | (PPU->vramaddr & (0x3 << 10))
							| ((PPU->vramaddr & 0x380) >> 4) | ((PPU->vramaddr & 0x1C) >> 2));
						if(PPU->vramaddr & 0x40) next_tile_attr >>= 4;
						if(PPU->vramaddr & 0x2) next_tile_attr >>=2;
						next_tile_attr &= 0x3;
						break;
					case 4:
						 next_pattern_lsb = ppu_readb(PPU->ppumap, ((PPU->registers.PPUCTRL & 0x10)<<8) |
						 	(next_tile_id << 4) | ((PPU->vramaddr & 0x7000)>>12) + 0);
						break;
					case 6:
						next_pattern_msb = ppu_readb(PPU->ppumap, ((PPU->registers.PPUCTRL & 0x10)<<8) |
						 	(next_tile_id << 4) | ((PPU->vramaddr & 0x7000)>>12) + 8);
						break;
					case 7:
						incrementx(PPU);
						break;
				}
			if((PPU->scanline < 240)){
				if((PPU->tick > 0) && (PPU->tick < 257)){
					uint8_t pixel = 0,palette = 0;
					uint8_t bgpixel = 0 , bgpalette = 0, fgpixel = 0, fgpalette = 0,fgpriority = 0;
					//doing the Secondary OAM clearing process in one go, everyone recommended so but less accurate
					if(PPU->tick == 1){
						OAMentry = 0;
						n = m = 0;
						wait = 0;
						memset(PPU->currOAM, 0xFF, 32);
						PPU->OAMpointer = ((char*)PPU->OAMdata) + PPU->registers.OAMADDR;
						zerosprflag = 0;
					}

					else if(PPU->tick >= 65){
						if(!wait){
							uint8_t y = *((uint8_t*)PPU->OAMpointer + n*4);
							uint8_t diff = PPU->scanline - y;
							if((diff >= 0) && (diff < ((PPU->registers.PPUCTRL & (1<<5)) ? 16 : 8 ))) {
								if(OAMentry != 8){
									memcpy(&(PPU->currOAM[OAMentry]), PPU->OAMpointer + n*4, 4);
									OAMentry++;
									if(n==0) 
										zerosprflag = 1;
								}
								n = (n+1) & 0x3F;
								wait = 7;
							}
							else {
								if(OAMentry != 8){
									memcpy(&(PPU->currOAM[OAMentry]), PPU->OAMpointer + n*4, 1);
								}
								n = (n+1) & 0x3F;
								wait = 1;
							}
						}
						else wait--;
					}
					if(SHOWBG){
						bgpixel   = (((patternshiftreg2 & mux) ? 1 : 0) << 1) | ((patternshiftreg1 & mux) ? 1 : 0);
						bgpalette = (((attribshiftreg2 & mux) ? 1 : 0) << 1)  | ((attribshiftreg1 & mux) ? 1 : 0);
					}
					if(SHOWCHR){
						for(int i = 0; i < 8; i++){
							if(!spritex[i]){
								fgpixel = (((spritereg2[i] & 0x80) ? 1 : 0) << 1) | ((spritereg1[i] & 0x80) ? 1:0);
								if(fgpixel){
									fgpalette  = ((spriteattr[i] & 0x3) + (1 << 2));
									fgpriority =  (spriteattr[i] & 0x20) ? 1 : 0;
									if(!i) zerosprflag++;
									break;
								}
							}
						}
					}
					if(!(bgpixel || fgpixel)){
						pixel = 0; palette = 0;
					}
					else if(bgpixel && fgpixel){
						if(fgpriority){
							pixel = bgpixel;
							palette = bgpalette;
						}
						else {
							pixel = fgpixel;
							palette = fgpalette;
						}

						if(zerosprflag >= 2){
							zerosprflag=0;
							PPU->registers.PPUSTATUS |= 0x40;
						}
					}
					else if (bgpixel){
						pixel = bgpixel;
						palette = bgpalette;
					}
					else if (fgpixel){
						pixel = fgpixel;
						palette = fgpalette;
					}
					PPU->gfx[(PPU->scanline*256) + (PPU->tick - 1)] = getargbfrompal(PPU, pixel, palette);
				}
			}
		}
		if(PPU->tick >= 257 || PPU->tick <= 320 ) PPU->registers.OAMADDR = 0;
		if (PPU->tick == 257){
			horittov(PPU);
			memset(spritex,0,8);
			memset(spriteattr,0,8);
			memset(spritereg1,0,8);
			memset(spritereg2,0,8);
			for(int i = 0; i < 8; i++){
				if(PPU->currOAM[i].xpos == 0xFF)
					break;
				uint8_t spritepattern1, spritepattern2;
				uint16_t spritepatternaddr;
				uint8_t diff = PPU->scanline - PPU->currOAM[i].ypos;
				//taking different routes based on sprite size
				if(!(PPU->registers.PPUCTRL & (1<<5))){
					if(PPU->currOAM[i].attr & 0x80){
						spritepatternaddr = ((PPU->registers.PPUCTRL & 0x8)<<9) | 
						((PPU->currOAM[i].index & 0xFF)<<4) | (7 - diff);
					}
					else{
						spritepatternaddr = ((PPU->registers.PPUCTRL & 0x8)<<9) | 
						((PPU->currOAM[i].index & 0xFF)<<4) | diff;	
					}
				}
				else {
					if(PPU->currOAM[i].attr & 0x80){
						if((PPU->scanline - PPU->currOAM[i].ypos) < 8){
							spritepatternaddr = ((PPU->currOAM[i].attr & 0x1) << 12) | 
							((PPU->currOAM[i].attr & 0xFE + 1) << 4) |  (7 - diff);
						}
						else 
							spritepatternaddr = ((PPU->currOAM[i].attr & 0x1) << 12) | 
							((PPU->currOAM[i].attr & 0xFE) << 4) | (7 - diff);
					}
					else {
						if((PPU->scanline - PPU->currOAM[i].ypos) < 8){
							spritepatternaddr = ((PPU->currOAM[i].attr & 0x1) << 12) | 
							((PPU->currOAM[i].attr & 0xFE) << 4) | (7 - diff);		
						}
						else {
							spritepatternaddr = ((PPU->currOAM[i].attr & 0x1) << 12) | 
							((PPU->currOAM[i].attr & 0xFE + 1) << 4) |  (7 - diff);
						}
					}
				}
				spritepattern1 = ppu_readb(PPU->ppumap,spritepatternaddr);
				spritepattern2 = ppu_readb(PPU->ppumap,spritepatternaddr + 8);
				//flip the bytes, a bit optimized way to do it
				if(PPU->currOAM[i].attr & 0x40){
					/** got this divide&conquer trick from OLC*****/
					spritepattern1 = (spritepattern1 & 0xF0) >> 4 | (spritepattern1 & 0x0F) << 4;
					spritepattern1 = (spritepattern1 & 0xCC) >> 2 | (spritepattern1 & 0x33) << 2;
					spritepattern1 = (spritepattern1 & 0xAA) >> 1 | (spritepattern1 & 0x55) << 1;
					spritepattern2 = (spritepattern2 & 0xF0) >> 4 | (spritepattern2 & 0x0F) << 4;
					spritepattern2 = (spritepattern2 & 0xCC) >> 2 | (spritepattern2 & 0x33) << 2;
					spritepattern2 = (spritepattern2 & 0xAA) >> 1 | (spritepattern2 & 0x55) << 1;
				}
				spritereg1[i] = spritepattern1;
				spritereg2[i] = spritepattern2;

				spriteattr[i] = PPU->currOAM[i].attr;
				spritex[i] = PPU->currOAM[i].xpos;
			}
		}
		if(PPU->tick == 256)
			incrementy(PPU);
		if(PPU->scanline == 261){
			if(PPU->tick == 1)
				PPU->registers.PPUSTATUS = 0x0;
			else if((PPU->tick > 279) && (PPU->tick < 305))
				vertttov(PPU);
		}
	}
	else if ((PPU->scanline == 241) && (PPU->tick == 1)){
		PPU->drawflag = 1;
		PPU->registers.PPUSTATUS |= (1<<7);
		if(PPU->registers.PPUCTRL & (1<<7)) PPU->innmi = 1;
	}
	PPU->tick++;
	if(PPU->tick > 340){
		PPU->tick = 0;
		PPU->scanline++;
		if(PPU->scanline > 261){
			PPU->scanline = 0;
			PPU->framecount++;
		}
	}
}
void ppu_reset(ppu* PPU){
	PPU->registers.PPUCTRL 	= 0x0;
	PPU->registers.PPUMASK 	= 0x0;
	PPU->registers.PPUSTATUS= 0x0 | (PPU->registers.PPUSTATUS & (1<<7));
	PPU->registers.PPUSCROLL= 0x0;
	PPU->registers.PPUDATA	= 0x0;
}

void ppu_init(ppu *PPU){
	PPU->registers.PPUCTRL	= 0x0;
	PPU->registers.PPUMASK	= 0x0;
	PPU->registers.PPUSTATUS= 0x0;
	PPU->registers.OAMADDR 	= 0x0;
	PPU->registers.PPUSCROLL= 0x0;
	PPU->registers.PPUADDR	= 0x0;
	PPU->registers.PPUDATA	= 0x0;
	PPU->scanline			= 0x0;
	PPU->innmi 				= 0;
	PPU->finex				= 0;
	PPU->framecount			= 0;
	PPU->tick				= 0;
}

uint8_t ppu_read_reg(memmap* map, uint16_t addr){
	ppu* PPU = (ppu*) &(((nes*)map[5].pointer)->PPU);
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
			if(VBLANK)
				return PPU->OAMpointer[PPU->registers.OAMADDR];
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
					incrementx(PPU);
					incrementy(PPU);
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
					incrementx(PPU);
					incrementy(PPU);
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
			//made a change here which could potentially be flawed
			PPU->tempvramaddr &= ~(0x3 <<10);
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
			if((((PPU->scanline >= 0) && (PPU->scanline <= 239)) || (PPU->scanline == 261)) && (SHOWBG || SHOWCHR))
				PPU->registers.OAMADDR+=4;
			else {
				PPU->OAMpointer[PPU->registers.OAMADDR] = byte;
				PPU->registers.OAMADDR++;
			}
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
				mux = 0x8000 >> PPU->finex;
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
				incrementx(PPU);
				incrementy(PPU);
			}
			break;
	}
}
