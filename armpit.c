/*
 * Compile using:
 * 			gcc armpit.c -std=c99 -o armpit
 */

/*
 * Condition Codes:
 *
	AL – Always				NV – Never
	EQ – Equal				NE – Not Equal
	VS – Overflow Set		VC – Overflow Clear
	MI – Minus				PL – Plus
	CS – Carry Set			CC – Carry Clear
	HI – Higher				LS – Lower or Same
	GE – Greater or equal	LT – Less Than
	GT – Greater than		LE – Less or Equal
 *
 */
#define CONDITION_EQUAL	(0x0) // 0000
#define CONDITION_NOT_EQUAL	(0x1) // 0001
#define CONDITION_CARRY_SET	(0x2) // 0010
#define CONDITION_CARRY_CLEAR	(0x3) // 0011
#define CONDITION_MINUS	(0x4) // 0100
#define CONDITION_PLUS	(0x5) // 0101
#define CONDITION_OVERFLOW_SET	(0x6) // 0110
#define CONDITION_OVERFLOW_CLEAR	(0x7) // 0111
#define CONDITION_HIGHER	(0x8) // 1000
#define CONDITION_LOWER_OR_SAME	(0x9) // 1001
#define CONDITION_GREATER_OR_EQUAL	(0xa) // 1010
#define CONDITION_LESS_THAN	(0xb) // 1011
#define CONDITION_GREATER_THAN	(0xc) // 1100
#define CONDITION_LESS_OR_EQUAL	(0xd) // 1101
#define CONDITION_ALWAYS	(0xe) // 1110
#define CONDITION_NEVER	(0xf) // 1111


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef uint8_t BYTE;

typedef union myInstructionType {
    uint32_t data32;
    BYTE byte[4];
} INSTRUCTION;

typedef uint32_t REGISTER;


// now define some bitmasks:
#define BITMASK_1_BIT 0X1
#define BITMASK_2_BIT 0X3
#define BITMASK_3_BIT 0X7
#define BITMASK_4_BIT 0Xf
#define BITMASK_5_BIT 0X1f
#define BITMASK_6_BIT 0X3f
#define BITMASK_7_BIT 0X7f
#define BITMASK_8_BIT 0Xff


/* -------------------------------------------------------------------------------
 * 	 OpCode (21..24) - Identifies the specific Data Processing Instruction
 * -------------------------------------------------------------------------------
 */
#define OP_AND	(0x0) // 0000
#define OP_EOR	(0x1) // 0001
#define OP_SUB	(0x2) // 0010
#define OP_RSB	(0x3) // 0011
#define OP_ADD	(0x4) // 0100
#define OP_ADC	(0x5) // 0101
#define OP_SBC	(0x6) // 0110
#define OP_RSC	(0x7) // 0111
#define OP_TST	(0x8) // 1000
#define OP_TEQ	(0x9) // 1001
#define OP_CMP	(0xa) // 1010
#define OP_CMN	(0xb) // 1011
#define OP_ORR	(0xc) // 1100
#define OP_MOV	(0xd) // 1101
#define OP_BIC	(0xe) // 1110
#define OP_MVN	(0xf) // 1111


// STATUS_REGISTER

/*
 * Status Register - 8 bits
 *
 * -----------------------------------
 * | 7 | 6 | 5 | 4 | 3 | 2 | 1  | 0  |
 * -----------------------------------
 * | N | Z | C | V | I | F | S1 | S0 |
 * -----------------------------------
 *
 * Bits 7-4 Status flag that identify the result of the last operation.
 *
 * Bit 7  Negative flag
 * Bit 6  Zero flag
 * Bit 5  Carry flag
 * Bit 4  Overflow flag
 *
 * Bits 3-2 Interrupt disable flags, when set prevent the specified type of interrupt from being serviced
 *
 * Bit 3  Interrupt disable flag
 * bit 2  Fast interrupt disable flag
 *
 * Bits 1-0 Identify the processor operation mode (User mode is 0 0)
 *
 * bit 1   Processor mode bit
 * bit 0   Processor mode bit
*/
#define STATUS_N	(1<<7) // negative
#define STATUS_Z	(1<<6) // zero flag
#define STATUS_C	(1<<5) // carry flag
#define STATUS_V	(1<<4) // overflow flag
#define STATUS_I	(1<<3) // interrupt disable flag
#define STATUS_I	(1<<3) // Interrupt disable flag
#define STATUS_F	(1<<2) // Fast interrupt disable flag
#define STATUS_S1	(1<<1) // processor mode
#define STATUS_S0	(1<<0) // processor mode


/*	------------------------------------------------------------------------
			Function Prototypes
	----------------------------------------------------------------------*/

void init();

void loadInstruction(INSTRUCTION);

INSTRUCTION fetchInstruction();



int main(int argc, char *argv[]) {
    init();

    exit(0);
}

void init() {
    printf("starting ARMpit");
}
