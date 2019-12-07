#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "memory.h"
#include <stdbool.h>
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
static uint8_t byte1,byte2,memory_val,opcode;
uint16_t word1,word2,curr_addr;
static unsigned int clock_check=0;
typedef enum addr_mode{
	IMPLIED,
	ZEROPAGE,
	ZEROPAGEX,
	ZEROPAGEY,
	ABSOLUTE,
	ABSOLUTEX,
	ABSOLUTEY,
	INDEXINDIR,
	INDIRINDEX
} mode;

/*****************ClockFunctions******************************/
#define INCCLOCK CPU->clockcount++;
#define DECCLOCK CPU->clockcount--;


//byte1, byte2, word1 and word2 are all temporary variables
/****************Macros for easy r/w operations*******************/

#define READMEMVAR() memory_val = cpureadb((CPU->cpumap),curr_addr);
#define WRITEMEMVAR() cpuwriteb((CPU->cpumap),curr_addr,memory_val);
#define READBYTE(addr) cpureadb((CPU->cpumap),addr);
#define READWORD(addr) cpureadw((CPU->cpumap),addr);
#define READZEROWRAP(addr) zpcpureadw((CPU->cpumap),addr);
#define WRITEBYTE(addr,data) cpuwriteb((CPU->cpumap),addr,data);
#define WRITEW(addr,data) cpuwritew((CPU->cpumap),addr,data);
#define WRITEZW(addr,data) zpcpuwritew((CPU->cpumap),addr,data);
#define WRITEBB() cpuwriteb((CPU->cpumap),curr_addr,byte2);
#define WRITEWB() cpuwritew((CPU->cpumap),curr_addr,curr_addr);
#define WRITEBW() cpuwriteb((CPU->cpumap),curr_addr,byte2);
#define WRITEWW() cpuwritew((CPU->cpumap),curr_addr,curr_addr);

/******************Addressing modes********************/

void zp(cpu* CPU){				 //zero page
	curr_addr=READBYTE(PC++);
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		curr_addr,AC,XR,YR,PF,ST);
}
void zpx(cpu* CPU){				//Indexed Zero Page
	curr_addr=READBYTE(PC++);
	curr_addr=(unsigned char)(curr_addr + XR);
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		curr_addr,AC,XR,YR,PF,ST);
}

void zpy(cpu* CPU){				//Indexed Zero Page with Y
	curr_addr=READBYTE(PC++);
	curr_addr=(unsigned char)(curr_addr + YR);
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		curr_addr,AC,XR,YR,PF,ST);
}

void ab(cpu* CPU){				//Absolute addressing
	curr_addr=READWORD(PC);
	PC+=2;
	if(DEBUG) printf("%02X %02X      A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		(curr_addr & 0x00ff),(curr_addr & 0xff00)>>8,AC,XR,YR,PF,ST);
}

void abx(cpu* CPU){				//Absolute addressing indexed with X
	curr_addr=READWORD(PC);
	word1=(curr_addr + XR);
	if(((curr_addr & 0xFF00) != (word1 & 0xFF00))){
		INCCLOCK;
		clock_check++;
	}
	curr_addr=word1;
	PC+=2;
	if(DEBUG) printf("%02X %02X      A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		(curr_addr & 0x00ff),(curr_addr & 0xff00)>>8,AC,XR,YR,PF,ST);
}

void aby(cpu* CPU){				//Absolute addressing indexed with Y
	curr_addr=READWORD(PC);
	word1=(curr_addr + YR);
	if(((curr_addr & 0xFF00) != (word1 & 0xFF00))){
		INCCLOCK;
		clock_check++;
	}
	curr_addr=word1;
	PC+=2;
	if(DEBUG) printf("%02X %02X      A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		(curr_addr & 0x00ff),(curr_addr & 0xff00)>>8,AC,XR,YR,PF,ST);
}

void im(cpu* CPU){				//Immediate addressing
	curr_addr=(PC++);
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		byte2,AC,XR,YR,PF,ST);
}

void xi(cpu* CPU){				//Indexed Indirect
	byte1=READBYTE(PC++);
	byte1 = byte1 + XR;
	curr_addr=READZEROWRAP(byte1);
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		byte1,AC,XR,YR,PF,ST);
}

void ix(cpu* CPU){				//Indirect Indexed
	byte1=READBYTE(PC++);
	curr_addr=READZEROWRAP(byte1);
	word1=(curr_addr + YR);
	if(((curr_addr & 0xFF00) != (word1 & 0xFF00))){
		INCCLOCK;
		clock_check++;
	}
	curr_addr=word1;
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		byte1,AC,XR,YR,PF,ST);
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


/*****************MISC Functions and Macros**********************/
//interrupt locations
#define IRQ (0xFFFE)
#define NMI (0xFFFA)
#define RESET (0xFFFC)
//Functions to help in code
#define PUSH(data) push(CPU, data);
#define POP() pop(CPU);
#define RJ(condition) rjump(CPU,condition);

void push(cpu *CPU, char data){
	cpuwriteb((CPU->cpumap), ST + 0x100, data);
	ST--;
}

unsigned char pop(cpu *CPU){
	ST++;
	return cpureadb((CPU->cpumap), ST + 0x100);
}

void rjump(cpu* CPU, int condition){
	byte1=READBYTE(PC++);
	if(condition){
		INCCLOCK;
		if(byte1 & 0x80){
			word1 = (uint16_t)(PC - (unsigned char)(0x80 - (byte1 & 0x7F)));
		}
		else{
			word1 = (uint16_t)(PC + (unsigned char)byte1);
		}
		if((PC & 0xFF00) != (word1 & 0xFF00)) INCCLOCK
		PC = word1;
	}
	if(DEBUG) printf("%02X         A:%02X X:%02X Y:%02X PF:%02X ST:%02X",
		byte1,AC,XR,YR,PF,ST);
}

uint8_t cycle_count_table[256]={
	7, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    2, 6, 0, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
    2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
    2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
};


/***************INSTRUCTIONS******************/

#define ADC(){											\
	word1=AC + memory_val + (PF & CFLAG);				\
	TSTFLAG(word1 > 0xFF,CFLAG);						\
	TSTFLAG(~(AC^memory_val) & (AC^word1) & 0x80, VFLAG);\
	AC=(uint16_t)word1;									\
	TSTFLAG(!AC, ZFLAG);								\
	TSTFLAG(AC & (1<<7),NFLAG);							\
}

#define AND(){											\
	AC &= memory_val;									\
	SETZN(AC);											\
}

//will be using accumulator for $0A case as memory_val in that case
#define ASL(){											\
	TSTFLAG(memory_val & 0x80, CFLAG);					\
	memory_val=memory_val<<1;							\
	SETZN(memory_val);									\
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
	byte1=AC & memory_val;								\
	SETZN(byte1);										\
	TSTFLAG(memory_val & 0x40, VFLAG);					\
	TSTFLAG(memory_val & 0x80, NFLAG);					\
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
	PC=READWORD(IRQ);									\
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
	TSTFLAG(AC>=memory_val, CFLAG);						\
	SETZN((char)(AC - memory_val));						\
}

#define CPX(){											\
	TSTFLAG(XR>=memory_val, CFLAG);						\
	SETZN(XR-memory_val);								\
}

#define CPY(){											\
	TSTFLAG(YR>=memory_val, CFLAG);						\
	SETZN(YR-memory_val);								\
}

#define DEC(){											\
	SETZN(--memory_val);								\
}

#define DEX(){											\
	SETZN(--XR);										\
}

#define DEY(){											\
	SETZN(--YR);										\
}

#define EOR(){											\
	AC^=memory_val;										\
	SETZN(AC);											\
}

#define INC(){											\
	SETZN((unsigned char)++memory_val);					\
}

#define INX(){											\
	SETZN(++XR);										\
}

#define INY(){											\
	SETZN(++YR);										\
}

#define JMP(){											\
	PC=curr_addr;										\
}

#define JSR(){											\
	word1=PC-1;											\
	PUSH(( word1 & 0xff00)>>8);							\
	PUSH( word1 & 0x00ff);								\
	PC=curr_addr;										\
}

#define LDA(){											\
	AC=memory_val;										\
	SETZN(memory_val);									\
}

#define LDX(){											\
	XR=memory_val;										\
	SETZN(XR);											\
}

#define LDY(){											\
	YR=memory_val;										\
	SETZN(YR);											\
}

#define LSR(){											\
	TSTFLAG((memory_val & 0x01), CFLAG);				\
	memory_val = memory_val>>1;							\
	SETZN(memory_val);									\
}

#define NOP(){ \
	if(DEBUG) printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
}

#define ORA(){											\
	AC|=memory_val;										\
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
	byte1=(PF & CFLAG);									\
	TSTFLAG(memory_val & 0x80, CFLAG);					\
	memory_val=memory_val<<1;							\
	memory_val|=byte1;									\
	SETZN(memory_val);									\
}

#define ROR(){											\
	byte1=(PF & CFLAG);									\
	TSTFLAG(memory_val & 0x01, CFLAG);					\
	memory_val= memory_val>>1 ; 						\
	memory_val|= (byte1<<7);							\
	SETZN(memory_val);									\
}

#define RTI(){											\
	if(DEBUG) printf("           A:%02X X:%02X Y:%02X PF:%02X ST:%02X",AC,XR, YR,PF,ST);\
	byte2=POP();										\
	byte1=(PF & (B1FLAG | B2FLAG));						\
	PF=byte2 & ~(B1FLAG | B2FLAG);						\
	PF=PF | byte1;										\
	byte2=POP();										\
	PC=(byte2<<8);										\
	byte2=POP();										\
	PC|=(byte2);										\
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
	word1=AC- memory_val - (1-(PF & CFLAG));			\
	TSTFLAG(word1<0x100, CFLAG);						\
	TSTFLAG((AC^memory_val) & (AC^word1) & 0x80, VFLAG);\
	AC=(char)word1;										\
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
	memory_val=AC;										\
}

#define STX(){											\
	memory_val=XR;										\
}

#define STY(){											\
	memory_val=YR;										\
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
	CPU->PrgCount=RESET;
	CPU->P=0x24;
	CPU->S=0xFD;
	CPU->clockcount=7;
	CPU->innmi=0;
}
void cpu_run(cpu *CPU){
	CPU->clockcount=0;
	word1=word2=byte1=byte2=0;
	clock_check=0;
	int row,column,block;
	printf("%04X ",PC);
	opcode=READBYTE(PC++);
	printf("%02X ",opcode);
	//row/column referenced from NESDev's opcode matrix
	block=(opcode & 0x03);
	column=(opcode & 0x1f)>>2;	//shifting to needed bits
	row=(opcode &  0xe0)>>5;
	CPU->clockcount+=cycle_count_table[opcode & 0xFF];
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
							READMEMVAR();
							NOP();
							break;
						case 0x5:
							IM();
							READMEMVAR();
							LDY();
							break;
						case 0x6:
							IM();
							READMEMVAR();
							CPY();
							break;
						case 0x7:
							IM();
							READMEMVAR();
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
							READMEMVAR();
							BIT();
							break;
						case 0x2:
						case 0x3:
							NOP();
							break;
						case 0x4:
							STY();
							WRITEMEMVAR();
							break;
						case 0x5:
							READMEMVAR();
							LDY();
							break;
						case 0x6:
							READMEMVAR();
							CPY();
							break;
						case 0x7:
							READMEMVAR();
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
							READMEMVAR();
							BIT();
							break;
						case 0x2:
							JMP();
							break;
						case 0x3:
							//special weird case JMP()

							if(!((curr_addr & 0x00ff) == 0xff)){
								PC=READWORD(curr_addr);
							}
							else{
								byte1=READBYTE(curr_addr);
								byte2=READBYTE(curr_addr & 0xFF00);
								PC=byte1 | (byte2<<8);
							}
							break;
						case 0x4:
							STY();
							WRITEMEMVAR();
							break;
						case 0x5:
							READMEMVAR();
							LDY();
							break;
						case 0x6:
							READMEMVAR();
							CPY();
							break;
						case 0x7:
							READMEMVAR();
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
							WRITEMEMVAR();
							break;
						case 0x5:
							READMEMVAR();
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
					clock_check++;
					switch(row){
						case 0x0:
						case 0x1:
						case 0x2:
						case 0x3:
							NOP();
							break;
						case 0x4:
							memory_val=YR & (((PC & 0xff00)>>8)-1);
							WRITEMEMVAR();
							clock_check++;
							break;
						case 0x5:
							READMEMVAR();
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
					clock_check++;
					INDY();
					break;
				case 0x5:
					ZPX();
					break;
				case 0x6:
					clock_check++;
					ABY();
					break;
				case 0x7:
					clock_check++;
					ABX();
					break;
			}
			switch(row){
				case 0x0:
					READMEMVAR();
					ORA();
					break;
				case 0x1:
					READMEMVAR();
					AND();
					break;
				case 0x2:
					READMEMVAR();
					EOR();
					break;
				case 0x3:
					READMEMVAR();
					ADC();
					break;
				case 0x4:
					clock_check++;
					if(column == 0x2){
						NOP();
						break;
					}
					else{
						memory_val=AC;
						WRITEMEMVAR();
					}
					break;
				case 0x5:
					READMEMVAR();
					LDA();
					break;
				case 0x6:
					READMEMVAR();
					CMP();
					break;
				case 0x7:
					READMEMVAR();
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
							NOP();
							break;
						case 0x5:
							IM();
							READMEMVAR();
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
					READMEMVAR();
					switch(row){
						case 0x0:
							ASL();
							break;
						case 0x1:
							ROL();
							break;
						case 0x2:
							LSR();
							break;
						case 0x3:
							ROR();
							break;
						case 0x4:
							STX();
							break;
						case 0x5:
							LDX();
							break;
						case 0x6:
							DEC();
							break;
						case 0x7:
							INC();
							break;
					}
					WRITEMEMVAR();
					break;
				case 0x2:
					switch(row){
						case 0x0:
							memory_val=AC;
							ASL();
							AC=memory_val;
							break;
						case 0x1:
							memory_val=AC;
							ROL();
							AC=memory_val;
							break;
						case 0x2:
							memory_val=AC;
							LSR();
							AC=memory_val;
							break;
						case 0x3:
							memory_val=AC;
							ROR();
							AC=memory_val;
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
					READMEMVAR();
					switch(row){
						case 0x0:
							ASL();
							break;
						case 0x1:
							ROL();
							break;
						case 0x2:
							LSR();
							break;
						case 0x3:
							ROR();
							break;
						case 0x4:
							STX();
							break;
						case 0x5:
							LDX();
							break;
						case 0x6:
							DEC();
							break;
						case 0x7:
							INC();
							break;
					}
					WRITEMEMVAR();
					break;

				case 0x4:
					//STP to be implemented
					break;

				case 0x5:
					switch(row){
						case 0x0:
							ZPX();
							READMEMVAR();
							ASL();
							break;
						case 0x1:
							ZPX();
							READMEMVAR();
							ROL();
							break;
						case 0x2:
							ZPX();
							READMEMVAR();
							LSR();
							break;
						case 0x3:
							ZPX();
							READMEMVAR();
							ROR();
							break;
						case 0x4:
							ZPY();
							READMEMVAR();
							STX();
							break;
						case 0x5:
							ZPY();
							READMEMVAR();
							LDX();
							break;
						case 0x6:
							ZPX();
							READMEMVAR();
							DEC();
							break;
						case 0x7:
							ZPX();
							READMEMVAR();
							INC();
							break;
					}
					WRITEMEMVAR();
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
					clock_check++;
					switch(row){
						case 0x0:
							ABX();
							READMEMVAR();
							ASL();
							clock_check++;
							break;
						case 0x1:
							ABX();
							READMEMVAR();
							ROL();
							clock_check++;
							break;
						case 0x2:
							ABX();
							READMEMVAR();
							LSR();
							clock_check++;
							break;
						case 0x3:
							ABX();
							READMEMVAR();
							ROR();
							clock_check++;
							break;
						case 0x4:
							ABY();
							clock_check++;
							READMEMVAR();
							memory_val = XR & (((PC & 0xff00)>>8)-1);
							break;
						case 0x5:
							ABY();
							READMEMVAR();
							LDX();
							break;
						case 0x6:
							ABX();
							READMEMVAR();
							DEC();
							clock_check++;
							break;
						case 0x7:
							ABX();
							READMEMVAR();
							INC();
							clock_check++;
							break;
					}
					WRITEMEMVAR();
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
					clock_check++;
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
					clock_check++;
					ABY();
					break;
				case 0x7:
					clock_check++;
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
					READMEMVAR();
					if(column==0x2){
						AND();
					}
					else{
						ASL();
						WRITEMEMVAR();
						ORA();
						clock_check++;
					}
					break;
				case 0x1:
					READMEMVAR();
					if(column == 0x2){
						AND();
					}
					else{
						ROL();
						WRITEMEMVAR();
						AND();
						clock_check++;
					}
					break;
				case 0x2:
					READMEMVAR();
					if(column==0x2){
						memory_val=memory_val & AC;
						LSR();
						AC=memory_val;
					}
					else{
						LSR();
						WRITEMEMVAR();
						EOR();
						clock_check++;
					}
					break;
				case 0x3:
					READMEMVAR();
					if(column == 0x2){
						memory_val=memory_val & AC;
						ROR();
						AC=memory_val;
					}
					else{
						ROR();
						ADC();
						WRITEMEMVAR();
						clock_check++;
					}
					break;
				case 0x4:
					READMEMVAR();
					if((column==0x0)||(column==0x1)||(column==0x3)||(column==0x5)){
						memory_val= XR & AC;
						//SETZN(byte2);
						WRITEMEMVAR();
					}
					else if((column==0x4)||(column==0x7)){
						clock_check++;
						memory_val=(XR & AC)& 0x7;
						WRITEMEMVAR();
					}
					else if(column==0x6){
						ST = XR & AC;
						memory_val=ST & (((PC & 0xff00)>>8)-1);
						WRITEMEMVAR();
					}
					else{
						printf(" to be implemented ");
					}
					break;
				case 0x5:
					READMEMVAR();
					if(column==0x2){
						XR= memory_val & AC;
						SETZN(XR);
					}
					else if(column==0x6){
						AC=ST & memory_val;
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
					READMEMVAR();
					if(column==0x2){
						XR=XR & AC;
						word1=XR - memory_val;
						TSTFLAG(word1 < 0x100, CFLAG);
						SETZN(word1);
						XR=word1;
					}
					else {
						memory_val=memory_val-1;
						CMP();
						WRITEMEMVAR();
						clock_check++;
					}
					break;
				case 0x7:
					READMEMVAR();
					if(column==0x2){
						SBC();
					}
					else{
						memory_val++;
						WRITEMEMVAR();
						SBC();
						clock_check++;
					}
					break;
			}
			break;
	}
	if(CPU->innmi){																					
		PUSH((PC>>0) & 0x00FF);								
		PUSH((PC>>8) & 0xFF);								
		PUSH(PF);											
		SETFLAG(B1FLAG);									
		PC=READWORD(NMI);
		CPU->innmi=0;
	}
	if(clock_check > 2) DECCLOCK
	printf(" Cycle: %d\n", CPU->clockcount);
}