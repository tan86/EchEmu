#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "memory.h"
#include "cpu.h"

#define DEBUG (1)
//defining macros for CPU's structure readability 

#define AC (CPU->A)	//Accummulator
#define XR (CPU->X)	//X register
#define YR (CPU->Y)	//Y register
#define PC (CPU->PrgCount)	//Program Counter
#define ST (CPU->S)	//Stack pointer
#define PF (CPU->P)	//Flag register

//Variables to transfer data and do operations 
unsigned char byte1,byte2,byte3;
uint16_t word1,word2;
//byte1 and word1 usually will hold memory addresses
//word2 and byte2 usually hold data to operate on or transfer
//byte1 turned out to be useless and byte1 was sufficient to address
//would be taken care of in later revisions
/****************Macros for easy r/w*******************/

#define READB(addr) cpureadb(&(CPU->cpumap),addr);
#define READW(addr) cpureadw(&(CPU->cpumap),addr);
#define READZW(addr) zpcpureadw(&(CPU->cpumap),addr);
#define WRITEB(addr,data) cpuwriteb(&(CPU->cpumap),addr,data);
#define WRITEW(addr,data) cpuwritew(&(CPU->cpumap),addr,data);
#define WRITEZW(addr,data) zpcpuwritew(&(CPU->cpumap),addr,data);
//writes the byte2 to byte1 address
#define WRITEBB() cpuwriteb(&(CPU->cpumap),byte1,byte2);
#define WRITEWB() cpuwritew(&(CPU->cpumap),byte1,word2);
#define WRITEBW() cpuwriteb(&(CPU->cpumap),word1,byte2);
#define WRITEWW() cpuwritew(&(CPU->cpumap),word1,word2);

/******************Addressing modes********************/

void zp(cpu* CPU){				 //zero page
	byte1=READB(PC++);
	byte2=READB(byte1);
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		byte1,AC,XR,YR,PF,ST);
}
void zpx(cpu* CPU){				//Indexed Zero Page
	byte1=READB(PC++);
	byte1+=XR;
	byte2=READB(byte1);
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		byte1,AC,XR,YR,PF,ST);
}

void zpy(cpu* CPU){				//Indexed Zero Page with Y
	byte1=READB(PC++);
	byte1+=YR;
	byte2=READB(byte1);
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		byte1,AC,XR,YR,PF,ST);
}

void ab(cpu* CPU){				//Absolute addressing
	word1=READW(PC);
	byte2=READB(word1);
	PC+=2;
	if(DEBUG) printf("%02X %02X      A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		(word1 & 0x00ff),(word1 & 0xff00)>>8,AC,XR,YR,PF,ST);
}

void abx(cpu* CPU){				//Absolute addressing indexed with X
	word1=READW(PC);
	word1+=XR;
	byte2=READB(word1);
	PC+=2;
	if(DEBUG) printf("%02X %02X      A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		(word1 & 0x00ff),(word1 & 0xff00)>>8,AC,XR,YR,PF,ST);
}

void aby(cpu* CPU){				//Absolute addressing indexed with Y
	word1=READW(PC);
	word1+=YR;
	byte2=READB(word1);
	PC+=2;
	if(DEBUG) printf("%02X %02X      A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		(word1 & 0x00ff),(word1 & 0xff00)>>8,AC,XR,YR,PF,ST);
}

void im(cpu* CPU){				//Immediate addressing
	byte2=READB(PC++);
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		byte2,AC,XR,YR,PF,ST);
}

void xi(cpu* CPU){				//Indexed Indirect
	byte3=READB(PC++);
	byte3 = byte3 + XR;
	word1=READZW(byte3);
	byte2=READB(word1);
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		byte3,AC,XR,YR,PF,ST);
}

void ix(cpu* CPU){				//Indirect Indexed
	byte3=READB(PC++);
	word1=READZW(byte3);
	word1 = word1 + YR;
	byte2=READB(word1);
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		byte3,AC,XR,YR,PF,ST);
}

#define ZP() 	zp(CPU);
#define ZPX() 	zpx(CPU);
#define ZPY()	zpy(CPU);
#define AB()	ab(CPU);
#define ABX()	abx(CPU);
#define ABY()	aby(CPU);
#define IM()	im(CPU);
#define INDX()	xi(CPU);
#define INDY()	ix(CPU);

/**********************FLAGS******************/

#define CFLAG  (1<<0)
#define ZFLAG  (1<<1)
#define IFLAG  (1<<2)
#define DFLAG  (1<<3)
#define B1FLAG (1<<4)
#define B2FLAG (1<<5)
#define VFLAG  (1<<6)
#define NFLAG  (1<<7)
int check_flag(cpu* CPU, int f){	//check if flag is set or not
	return (PF & f);
}
void set_flag(cpu* CPU, unsigned int f){	//sets flag
	PF= PF & (~(f));
	PF= PF | f;
}

void test_flag(cpu* CPU, bool condition, unsigned int f){
	PF= PF & (~(f));
	if (condition) PF=PF | f;
}

void clear_flag(cpu* CPU, int f){	//clear flag
	PF = PF &  ~(f);
}
void setznflag(cpu* CPU, int value){
	PF=PF & ~(NFLAG | ZFLAG);
	if(!value) PF |= ZFLAG;
	else if(value >> 7) PF |= NFLAG;
}

#define CHKFLAG(f) check_flag(CPU, f);
#define SETFLAG(f) set_flag(CPU, f);
#define TSTFLAG(c,f) test_flag(CPU,c,f);
#define SETZN(f) setznflag(CPU,f);

void cyclecountsomething(){}; //not umplemented

/*****************MISC Functions and Macros**********************/
//interrupt locations
#define IRQ (0xFFFE)
#define NMI (0xFFFA)
#define RESET (0xFFFC)
//Functions to help in code
#define PUSH(data) push(CPU, data);
#define POP() pop(CPU);
void push(cpu *CPU, char data){
	cpuwriteb(&(CPU->cpumap), ST + 0x100, data);
	ST--;
}

unsigned char pop(cpu *CPU){
	ST++;
	byte2=cpureadb(&(CPU->cpumap), ST + 0x100);
	return byte2;
}
void rjump(cpu* CPU, int condition){
	byte2=READB(PC++);
	if(condition){
		if(byte2 & 0x80)
			PC=PC - (0x80 - (byte2 & 0x7F));
		else
			PC=PC+byte2;
	}
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		byte2,AC,XR,YR,PF,ST);
	//cycle check skipped
}

#define RJ(condition) rjump(CPU,condition);

/***************INSTRUCTIONS******************/

#define ADC(){											\
	word2=AC + byte2 + (PF & CFLAG);					\
	TSTFLAG(word2 > 0xFF,CFLAG);						\
	TSTFLAG(~(AC^byte2) & (AC^word2) & 0x80, VFLAG);	\
	AC=(uint16_t)word2;									\
	TSTFLAG(!AC, ZFLAG);								\
	TSTFLAG(AC & (1<<7),NFLAG);							\
}

#define AND(){											\
	AC &= byte2;										\
	SETZN(AC);											\
}

#define ASL(){											\
	TSTFLAG(byte2 & 0x80, CFLAG);						\
	byte2=byte2<<1;										\
	SETZN(byte2);										\
}

#define BCC(){											\
	RJ(!(PF & CFLAG));									\
}

#define BCS(){											\
	RJ(PF & CFLAG);										\
}

#define BEQ(){											\
	RJ(PF & ZFLAG);										\
}

#define BIT(){											\
	byte1=AC & byte2;									\
	SETZN(byte1);										\
	TSTFLAG(byte2 & 0x40, VFLAG);						\
	TSTFLAG(byte2 & 0x80, NFLAG);						\
}

#define BMI(){											\
	RJ(NFLAG & PF);										\
}

#define BNE(){											\
	RJ(!(ZFLAG & PF));									\
}

#define BPL(){											\
	RJ(!(NFLAG & PF));									\
}

#define BRK(){											\
	PC++;												\
	PUSH((PC>>0) & 0x00FF);								\
	PUSH((PC>>8) & 0xFF);								\
	PUSH(PF);											\
	SETFLAG(B1FLAG);									\
	PC=READW(IRQ);										\
}

#define BVC(){											\
	RJ(!(VFLAG & PF));									\
}

#define BVS(){											\
	RJ(VFLAG & PF);										\
}

#define CLC(){											\
	if(DEBUG) printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	TSTFLAG(0,CFLAG);									\
}

#define CLD(){											\
	if(DEBUG) printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	TSTFLAG(0,DFLAG);									\
}

#define CLI(){											\
	if(DEBUG) printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	TSTFLAG(0,IFLAG);									\
}

#define CLV(){											\
	if(DEBUG) printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	TSTFLAG(0,VFLAG);									\
}

#define CMP(){											\
	TSTFLAG(AC>=byte2, CFLAG);							\
	SETZN((char)(AC-byte2));							\
}

#define CPX(){											\
	TSTFLAG(XR>=byte2, CFLAG);							\
	SETZN(XR-byte2);									\
}

#define CPY(){											\
	TSTFLAG(YR>=byte2, CFLAG);							\
	SETZN(YR-byte2);									\
}

#define DEC(){											\
	SETZN(--byte2);										\
}

#define DEX(){											\
	SETZN(--XR);										\
}

#define DEY(){											\
	SETZN(--YR);										\
}

#define EOR(){											\
	AC^=byte2;											\
	SETZN(AC);											\
}

#define INC(){											\
	SETZN((unsigned char)++byte2);						\
}

#define INX(){											\
	SETZN(++XR);										\
}

#define INY(){											\
	SETZN(++YR);										\
}

#define JMP(){											\
	PC=word1;											\
}

#define JSR(){											\
	word2=PC-1;											\
	PUSH(( word2 & 0xff00)>>8);							\
	PUSH( word2 & 0x00ff);								\
	PC=word1;											\
}

#define LDA(){											\
	AC=byte2;											\
	SETZN(byte2);										\
}

#define LDX(){											\
	XR=byte2;											\
	SETZN(XR);											\
}

#define LDY(){											\
	YR=byte2;											\
	SETZN(YR);											\
}

#define LSR(){											\
	TSTFLAG((byte2 & 0x01), CFLAG);						\
	byte2 = byte2>>1;									\
	SETZN(byte2);										\
}

#define NOP(){ \
	if(DEBUG) printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
}

#define ORA(){											\
	AC|=byte2;											\
	SETZN(AC);											\
}

#define PHA(){											\
	if(DEBUG) printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	PUSH(AC);											\
}

#define PHP(){											\
	if(DEBUG) printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	PUSH((PF | 0x30));									\
}

#define PLA(){											\
	if(DEBUG) printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	AC=POP();											\
	SETZN(AC);											\
}

#define PLP(){											\
	if(DEBUG) printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	byte2=POP();										\
	byte1=(PF & (B1FLAG | B2FLAG));						\
	PF=byte2 & ~(B1FLAG | B2FLAG);						\
	PF=PF | byte1;										\
}

#define ROL(){											\
	unsigned int temp;									\
	temp=(PF & CFLAG);									\
	TSTFLAG(byte2 & 0x80, CFLAG);						\
	byte2=byte2<<1;										\
	byte2|=temp;										\
	SETZN(byte2);										\
}

#define ROR(){											\
	unsigned int temp;									\
	temp=(PF & CFLAG);									\
	TSTFLAG(byte2 & 0x01, CFLAG);						\
	byte2= byte2>>1 ; 									\
	byte2|= (temp<<7);									\
	SETZN(byte2);										\
}

#define RTI(){											\
	if(DEBUG) printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	byte2=POP();										\
	byte1=(PF & (B1FLAG | B2FLAG));						\
	PF=byte2 & ~(B1FLAG | B2FLAG);						\
	PF=PF | byte1;										\
	byte2=POP();										\
	PC=(byte2);											\
	byte2=POP();										\
	PC|=(byte2<<8);										\
}

#define RTS(){											\
	if(DEBUG) printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	byte2=POP();										\
	PC=byte2;											\
	byte2=POP();										\
	PC|=(byte2)<<8;										\
	PC++;												\
}

#define SBC(){											\
	word2=AC-byte2- (1-(PF & CFLAG));					\
	TSTFLAG(word2<0x100, CFLAG);						\
	TSTFLAG((AC^byte2) & (AC^word2) & 0x80, VFLAG);		\
	AC=(char)word2;										\
	SETZN(AC);											\
}

#define SEC(){											\
	printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	TSTFLAG(1,CFLAG);									\
}

#define SED(){											\
	printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	TSTFLAG(1,DFLAG);									\
}

#define SEI(){											\
	printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	TSTFLAG(1,IFLAG);									\
}

#define STA(){											\
	byte2=AC;											\
}

#define STX(){											\
	byte2=XR;											\
}

#define STY(){											\
	byte2=YR;											\
}

#define TAX(){											\
	XR=AC;												\
	SETZN(XR);											\
}

#define TAY(){											\
	YR=AC;												\
	SETZN(YR);											\
}

#define TSX(){											\
	XR=ST;												\
	SETZN(XR);											\
}

#define TXA(){											\
	AC=XR;												\
	SETZN(AC);											\
}

#define TXS(){											\
	ST=XR;												\
}

#define TYA(){											\
	AC=YR;												\
	SETZN(AC);											\
}

/****************INSTRUCTIONS END HERE*****************/

/****************CPU Initialization********************/
void cpu_init(cpu *CPU){
	CPU->A=0;
	CPU->X=0;
	CPU->Y=0;
	PC=0xC000;
	CPU->P=0x24;
	CPU->S=0xFD;
}
void cpu_run(cpu *CPU){
	byte3=byte1=byte2=(char)0;
	word1=word2=0;
	printf("%04X ",PC);
	unsigned char opcode=READB(PC++);
	printf("%02X ",opcode);
	//row/column referenced from NESDev's opcode matrix
	int block=(opcode & 0x03);
	int column=(opcode & 0x1f)>>2;	//shifting to needed bits
	int row=(opcode &  0xe0)>>5;
	switch(block){
		case 0x0:
			//control block
			//there are less patterns here so it could be mess to interpret
			switch(column){
				case 0x0:
					switch(row){
						case 0x0:
							BRK();
							break;
						case 0x1:
							AB();
							JSR();
							break;
						case 0x2:
							RTI();
							break;
						case 0x3:
							RTS();
							break;
						case 0x4:
							IM();
							NOP();
							break;
						case 0x5:
							IM();
							LDY();
							break;
						case 0x6:
							IM();
							CPY();
							break;
						case 0x7:
							IM();
							CPX();
							break;
					}
					break;
				case 0x1:
					ZP();
					switch(row){
						case 0x0:
							NOP();
							break;
						case 0x1:
							BIT();
							break;
						case 0x2:
						case 0x3:
							NOP();
							break;
						case 0x4:
							STY();
							WRITEBB();
							break;
						case 0x5:
							LDY();
							break;
						case 0x6:
							CPY();
							break;
						case 0x7:
							CPX();
							break;
					}
					break;
				case 0x2:
					switch(row){
						case 0x0:
							PHP();
							break;
						case 0x1:
							PLP();
							break;
						case 0x2:
							PHA();
							break;
						case 0x3:
							PLA();
							break;
						case 0x4:
							DEY();
							break;
						case 0x5:
							TAY();
							break;
						case 0x6:
							INY();
							break;
						case 0x7:
							INX();
							break;
					}
					break;
				case 0x3:
					AB();
					switch(row){
						case 0x0:
							NOP();
							break;
						case 0x1:
							BIT();
							break;
						case 0x2:
							JMP();
							break;
						case 0x3:
							if(!((word1 & 0x00ff) == 0xff)){
								PC=READW(word1);
							}
							else{
								byte1=READB(word1);
								byte2=READB(word1 & 0xFF00);
								PC=byte1 | (byte2<<8);
							}
							break;
						case 0x4:
							STY();
							WRITEBW();
							break;
						case 0x5:
							LDY();
							break;
						case 0x6:
							CPY();
							break;
						case 0x7:
							CPX();
							break;
					}
					break;
				case 0x4:
					switch(row){
						case 0x0:
							BPL();
							break;
						case 0x1:
							BMI();
							break;
						case 0x2:
							BVC();
							break;
						case 0x3:
							BVS();
							break;
						case 0x4:
							BCC();
							break;
						case 0x5:
							BCS();
							break;
						case 0x6:
							BNE();
							break;
						case 0x7:
							BEQ();
							break;
					}
					break;
				case 0x5:
					ZPX();
					switch(row){
						case 0x0:
						case 0x1:
						case 0x2:
						case 0x3:
							NOP();
							break;
						case 0x4:
							STY();
							WRITEBB();
							break;
						case 0x5:
							LDY();
							break;
						case 0x6:
						case 0x7:
							NOP();
							break;
					}
					break;
				case 0x6:
					switch(row){
						case 0x0:
							CLC();
							break;
						case 0x1:
							SEC();
							break;
						case 0x2:
							CLI();
							break;
						case 0x3:
							SEI();
							break;
						case 0x4:
							TYA();
							break;
						case 0x5:
							CLV();
							break;
						case 0x6:
							CLD();
							break;
						case 0x7:
							SED();
							break;
					}
					break;
				case 0x7:
					ABX();
					switch(row){
						case 0x0:
						case 0x1:
						case 0x2:
						case 0x3:
							NOP();
							break;
						case 0x4:
							byte2=YR & (((PC & 0xff00)>>8)-1);
							WRITEBW();
							break;
						case 0x5:
							LDY();
							break;
						case 0x6:
						case 0x7:
							NOP();
							break;
					}
					break;
			}
			break;
		case 0x1:
			//ALU block
			//has some patterns this time to make it quick to code
			switch(column){
				case 0x0:
					INDX();
					break;
				case 0x1:
					ZP();
					break;
				case 0x2:
					IM();
					break;
				case 0x3:
					AB();
					break;
				case 0x4:
					INDY();
					break;
				case 0x5:
					ZPX();
					break;
				case 0x6:
					ABY();
					break;
				case 0x7:
					ABX();
					break;
			}
			switch(row){
				case 0x0:
					ORA();
					break;
				case 0x1:
					AND();
					break;
				case 0x2:
					EOR();
					break;
				case 0x3:
					ADC();
					break;
				case 0x4:
					if(column == 0x2) break;
					else if((column==0x1)||(column==0x5))
						word1=byte1;
					byte2=AC;
					WRITEBW();
					break;
				case 0x5:
					LDA();
					break;
				case 0x6:
					CMP();
					break;
				case 0x7:
					SBC();
					break;
			}
			break;
		case 0x2:
			//RMW block
			//another confusing block of switch, I will try to be optimizing it as time passes
			switch(column){
				case 0x0:
					switch(row){
						case 0x0:
						case 0x1:
						case 0x2:
						case 0x3:
						//STP to be implemented
							break;
						case 0x4:
							IM();
							break;
						case 0x5:
							IM();
							LDX();
							break;
						case 0x6:
						case 0x7:
							IM();
							NOP();
							break;
					}
					break;
				case 0x1:
					ZP();
					switch(row){
						case 0x0:
							ASL();
							WRITEBB();
							break;
						case 0x1:
							ROL();
							WRITEBB();
							break;
						case 0x2:
							LSR();
							WRITEBB();
							break;
						case 0x3:
							ROR();
							WRITEBB();
							break;
						case 0x4:
							STX();
							WRITEBB();
							break;
						case 0x5:
							LDX();
							break;
						case 0x6:
							DEC();
							WRITEBB();
							break;
						case 0x7:
							INC();
							WRITEBB();
							break;
					}
					break;
				case 0x2:
					byte2=AC;
					switch(row){
						case 0x0:
							ASL();
							AC=byte2;
							break;
						case 0x1:
							ROL();
							AC=byte2;
							break;
						case 0x2:
							LSR();
							AC=byte2;
							break;
						case 0x3:
							ROR();
							AC=byte2;
							break;
						case 0x4:
							TXA();
							break;
						case 0x5:
							TAX();
							break;
						case 0x6:
							DEX();
							break;
						case 0x7:
							NOP();
							break;
					}
					break;
				case 0x3:
					AB();
					switch(row){
						case 0x0:
							ASL();
							WRITEBW();
							break;
						case 0x1:
							ROL();
							WRITEBW();
							break;
						case 0x2:
							LSR();
							WRITEBW();
							break;
						case 0x3:
							ROR();
							WRITEBW();
							break;
						case 0x4:
							STX();
							WRITEBW();
							break;
						case 0x5:
							LDX();
							WRITEBW();
							break;
						case 0x6:
							DEC();
							WRITEBW();
							break;
						case 0x7:
							INC();
							WRITEBW();
							break;
					}
					break;

				case 0x4:
					//STP to be implemented
					break;

				case 0x5:
					switch(row){
						case 0x0:
							ZPX();
							ASL();
							WRITEBB();
							break;
						case 0x1:
							ZPX();
							ROL();
							WRITEBB();
							break;
						case 0x2:
							ZPX();
							LSR();
							WRITEBB();
							break;
						case 0x3:
							ZPX();
							ROR();
							WRITEBB();
							break;
						case 0x4:
							ZPY();
							STX();
							WRITEBB();
							break;
						case 0x5:
							ZPY();
							LDX();
							WRITEBB();
							break;
						case 0x6:
							ZPX();
							DEC();
							WRITEBB();
							break;
						case 0x7:
							ZPX();
							INC();
							WRITEBB();
							break;
					}
					break;
				case 0x6:
					switch(row){
						case 0x0:
						case 0x1:
						case 0x2:
						case 0x3:
							NOP();
							break;
						case 0x4:
							TXS();
							break;
						case 0x5:
							TSX();
							break;
						case 0x6:
						case 0x7:
							NOP();
							break;
					}
					break;
				case 0x7:
				//cam be broken down more column>>2
					switch(row){
						case 0x0:
							ABX();
							ASL();
							WRITEBW();
							break;
						case 0x1:
							ABX();
							ROL();
							WRITEBW();
							break;
						case 0x2:
							ABX();
							LSR();
							WRITEBW();
							break;
						case 0x3:
							ABX();
							ROR();
							WRITEBW();
							break;
						case 0x4:
							ABY();
							byte2= XR & (((PC & 0xff00)>>8)-1);
							WRITEBW();
							break;
						case 0x5:
							ABY();
							LDX();
							WRITEBW();
							break;
						case 0x6:
							ABX();
							DEC();
							WRITEBW();
							break;
						case 0x7:
							ABX();
							INC();
							WRITEBW();
							break;
					}
					break;
			}
			break;

		case 0x3:
			//unofficial blocks
			switch(column){
				case 0x0:
					INDX();
					break;
				case 0x1:
					ZP();
					break;
				case 0x2:
					IM();
					break;
				case 0x3:
					AB();
					break;
				case 0x4:
					INDY();
					break;
				case 0x5:
					switch(row){
						case 0x0:
						case 0x1:
						case 0x2:
						case 0x3:
							ZPX();
							break;
						case 0x4:
						case 0x5:
							ZPY();
							break;
						case 0x6:
						case 0x7:
							ZPX();
							break;
					}
					break;
				case 0x6:
					ABY();
					break;
				case 0x7:
					switch(row){
						case 0x0:
						case 0x1:
						case 0x2:
						case 0x3:
							ABX();
							break;
						case 0x4:
						case 0x5:
							ABY();
							break;
						case 0x6:
						case 0x7:
							ABX();
							break;
					}
					break;
			}
			switch(row){
				case 0x0:
					if(column==0x2){
						AND();
					}
					else{
						ASL();
						if(byte1) word1=byte1;
						WRITEBW();
						ORA();
					}
					break;
				case 0x1:
					if(column == 0x2){
						AND();
					}
					else{
						ROL();
						if(byte1) word1=byte1;
						WRITEBW();
						AND();
					}
					break;
				case 0x2:
					if(column==0x2){
						byte2=byte2 & AC;
						LSR();
						AC=byte2;
					}
					else{
						LSR();
						if(byte1) word1=byte1;
						WRITEBW();
						EOR();
					}
					break;
				case 0x3:
					if(column == 0x2){
						byte2=byte2 & AC;
						ROR();
						AC=byte2;
					}
					else{
						ROR();
						ADC();
						if(byte1) word1=byte1;
						WRITEBW();
					}
					break;
				case 0x4:
					if((column==0x0)||(column==0x1)||(column==0x3)||(column==0x5)){
						byte2= XR & AC;
						//SETZN(byte2);
						if(byte1) word1=byte1;
						WRITEBW();
					}
					else if((column==0x4)||(column==0x7)){
						byte2=(XR & AC)& 0x7;
						WRITEBW();
					}
					else if(column==0x6){
						ST = XR & AC;
						byte2=ST & (((PC & 0xff00)>>8)-1);
						WRITEBW();
					}
					else{
						printf(" to be implemented ");
					}
					break;
				case 0x5:
					if(column==0x2){
						XR=byte2 & AC;
						SETZN(XR);
					}
					else if(column==0x6){
						AC=ST & byte2;
						XR=AC;
						ST=AC;
						SETZN(AC);
					}
					else{
						LDX();
						LDA();
					}
					break;
				case 0x6:
					if(column==0x2){
						XR=XR & AC;
						word1=XR-byte2;
						TSTFLAG(word1 < 0x100, CFLAG);
						SETZN(word1);
						XR=word1;
					}
					else {
						byte2=byte2-1;
						if(byte1) word1=byte1;
						CMP();
						WRITEBW();
					}
					break;
				case 0x7:
					if(column==0x2){
						SBC();
					}
					else{
						byte2++;
						if(byte1) word1=byte1;
						WRITEBW();
						SBC();
					}
					break;
			}
			break;
	}
	printf("\n");
}