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
#define COND_CODE_POS (28)

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
#include <string.h>     // for strcat
#include <stdbool.h>	// requires C99 and is considered the "standard way" to do booleans
#include <getopt.h>		// convenient argument parsing. Works on Linux and Mac OSX
//#include <unistd.h>	// can either use unistd.h or getopt.h for handling args.

#include "armpit.h"     // contains some defines and my function prototypes.



// registers:
// REGISTER registers[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
// // uint32_t *R0 = &registers[0];
// // int R1 = *registers[1];
// // int &REGISTER_2 = *registers[2];
// // int &REGISTER_3 = *registers[3];
// // int &REGISTER_4 = *registers[4];
// // int &REGISTER_5 = *registers[5];
// // int &REGISTER_6 = *registers[6];
// // int &REGISTER_7 = *registers[7];
// // int &REGISTER_8 = *registers[8];
// // int &REGISTER_9 = *registers[9];
// // int &REGISTER_10 = *registers[10];
// // int &REGISTER_11 = *registers[11];
// // int &REGISTER_12 = *registers[12];       // General Purpose (but often used as the stack pointer).
// REGISTER *STACK_POINTER = &registers[13];     // R13 is the SP (stack pointer)
// REGISTER *LINK_REGISTER = &registers[14];     // R14 is general purpose or the LR (link register)
// REGISTER *PROGRAM_COUNTER = &registers[15];   // R15 is used as program counter

REGISTER R1 = 0;
REGISTER R2 = 0;
REGISTER R3 = 0;
REGISTER R4 = 0;
REGISTER R5 = 0;
REGISTER R6 = 0;
REGISTER R7 = 0;
REGISTER R8 = 0;
REGISTER R9 = 0;
REGISTER R10 = 0;
REGISTER R11 = 0;
REGISTER R12 = 0;
// int &REGISTER_12 = *registers[12];       // General Purpose (but often used as the stack pointer).
REGISTER STACK_POINTER;     // R13 is the SP (stack pointer)
REGISTER LINK_REGISTER;     // R14 is general purpose or the LR (link register)
REGISTER PROGRAM_COUNTER;   // R15 is used as program counter

// (note: the bottom 2 bits of the PC are always 0 0 when memory is accessed.
//                  i.e. memory is implicitly accessed on a word aligned boundary.)


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
BYTE STATUS_REGISTER;

#define STATUS_N	(1<<7) // negative
#define STATUS_Z	(1<<6) // zero flag
#define STATUS_C	(1<<5) // carry flag
#define STATUS_V	(1<<4) // overflow flag
#define STATUS_I	(1<<3) // interrupt disable flag
#define STATUS_I	(1<<3) // Interrupt disable flag
#define STATUS_F	(1<<2) // Fast interrupt disable flag
#define STATUS_S1	(1<<1) // processor mode
#define STATUS_S0	(1<<0) // processor mode


/*
 * Global Variables:
 */
char USAGE[] = "\nARMpit - ARM processor imitation technology (2013 - Samael Bate)\n\nUsage: armpit [arguments] [file ..]\n\nArguments:\n -f <filename>\tspecify a file of ARM assembly to be processed.\n -v\t\tverbose output\n -h\t\thelp";

static bool VERBOSE = false;	// -v option
const char *fileName;			// -f option
FILE *file;
static const char *optString = "f:vh"; // requires <getopt.h> or <unistd.h>

INSTRUCTION memory[1024]; // 4 kilobytes of RAM in this example (since 1 instruction = 4 bytes)



/*	------------------------------------------------------------------------
		Function Prototypes now in armpit.h
	----------------------------------------------------------------------*/



// function from: stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
const char *byte_to_binary(int x) {
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1) {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    x,z=0;
    return b;
}

/*
 * should potentially take some arguments. I want to use '-f' to point to a text file of 
 * ARM assembly which can the be parsed into binary and loaded onto my virtual stack.
 * -v should activate verbose logging output and -h should show the usage.
 */
int main(int argc, char *argv[]) {
    printf("\nStarting ARMpit...\n\n");

    int opt = getopt( argc, argv, optString );
    while( opt != -1 ) {
        switch( opt ) {                
            case 'f':
                fileName = optarg;
                file = fopen(fileName, "r"); // r indicates opening for Read Only
                if(file == NULL) {
                    printf("Error opening %s\n", fileName);
                    exit(EXIT_FAILURE);
                }
                printf("\n\tAT THIS TIME INPUT FILES ARE UNSUPPORTED\n\n");
                exit(EXIT_FAILURE);
                break;
            case 'v':
                VERBOSE = true;
                break;
            case 'h':
                displayUsage();
                exit(EXIT_SUCCESS);
                break;
            case '?':
                displayUsage();
                exit(EXIT_FAILURE);
                break;
            default:
                /* You won't actually get here. */
                break;
        }
        opt = getopt( argc, argv, optString );
    }
    
    printf("Verbose Logging: %s\n", VERBOSE? "true" : "false");
    printf("Input File: %s\n\n", fileName? fileName : "none");

    init(); // load instructions onto stack

    // fetch
    INSTRUCTION instruction;
    int amountOfInstructions = STACK_POINTER;
    
    for(int counter = 0; counter < amountOfInstructions; counter++) {
        instruction = fetchInstruction();

        // check 1st 4 bits (if 1110 execute)
        if(shouldExecute(instruction)) {

            InstructionType type = getInstructionType(instruction);

            switch(type) {
                case ALU:
                    doDataProcessing();
                    break;
                case BRANCH:
                    doBranch();
                    break;
                case DATA_TRANSFER:
                    doDataTransfer();
                    break;
                case INTERUPT:
                    doInterupt();
                    break;
            }

        } else {
            if(VERBOSE) {
                printf("\tInstruction %i skipped!\n", counter);
            }
        }
    }   

    printf("\nExiting program\n\n");
    exit(EXIT_SUCCESS);
}

void init() {

    STATUS_REGISTER = 0;
    LINK_REGISTER = 0;
    STACK_POINTER = 0;
    PROGRAM_COUNTER = 0;

    INSTRUCTION instruction;
    instruction.data32 = 0xE3A00001;	// MOV R0, #1 	; 11100011 10100000 00000000 00000001
    loadInstruction(instruction);

    instruction.data32 = 0xF0000000; // never execute;
    loadInstruction(instruction);

    instruction.data32 = 0xE4000000; // dummy branch - 01
    loadInstruction(instruction);

    instruction.data32 = 0xE8000000; // dummy transfer - 10
    loadInstruction(instruction);

    instruction.data32 = 0xEF000000; // dummy interupt - 11
    loadInstruction(instruction);

    instruction.data32 = 0xE3f3fa02;    // MOV R1, #2       ; 1110 0011 1010 0000 0001 0000 0000 0010
    loadInstruction(instruction);

    instruction.data32 = 0xE0802001;    // ADD R2, R0, R1   ; 1110 0000 1000 0000 0010 0000 0000 0001
    loadInstruction(instruction);

    instruction.data32 = 0xE2822001;    // ADD R2, R2, #5   ; 1110 0010 1000 0010 0010 0000 0000 0101
    loadInstruction(instruction);
}

void displayUsage() {
	puts(USAGE);
}

void loadInstruction(INSTRUCTION i) {
	if(VERBOSE)
		printf("\tLoading Instruction: %04X onto stack at position %i\n", i.data32, STACK_POINTER);

	// now to place it onto my virtual stack
    memory[STACK_POINTER] = i;
    STACK_POINTER++;
}

INSTRUCTION fetchInstruction() {
    if(STACK_POINTER > 0)
        STACK_POINTER--; // as the position is incremented after loading each instruction, it needs to be decreased 
    INSTRUCTION instruction = memory[STACK_POINTER];
    printf("\n\tFetching instruction: %04X from stack position %i\n", instruction.data32, STACK_POINTER);
    return instruction;
}

InstructionType getInstructionType(INSTRUCTION instruction) {
    InstructionType type;

    type = (instruction.data32 >> 26) & BITMASK_3_BIT;

    if(VERBOSE) {
        printf("\tInstruction is type: %d\n", type);
    }
    return type; // returns 0, 1, 2, or 3 for ALU, BRANCH, DATA_TRANSFER, or INTERUPT
}


/*
 * All instructions are 32 bits in length.  Bit 31-28 of all instructions contain a condition code which is used to decide 
 * whether the instruction will execute.
 * This condition code identifies the required state of the status register (SR) in order for execution to take place.
 *
 * The table below shows each condition code value (stored in top 4 bits of the instruction) and the status flags which are
 * tested to see whether the condition is true.  If a status flag is blank then that means its value doesn't matter for that
 * condition to be true.
 *
 * -------------------------------------------------------------------------------
 *               Condition   Status Reg Flags
 *                  Code    | N | Z | C | V |
 * -------------------------------------------------------------------------------
 * Equal            0000    |   | 1 |   |   | Execute the instruction if the Z flag is set.
 * Not Equal        0001    |   | 0 |   |   | Execute the instruction is the Z flag is clear.
 * Carry Set        0010    |   |   | 1 |   | Execute the instruction if the C flag is set. Provides >= test (unsigned).
 * Carry Clear      0011    |   |   | 0 |   | Execute if the C flag is clear. Provides < test for unsigned comparison.
 * Minus            0100    | 1 |   |   |   | Execute if the N flag is set.
 * Plus             0101    | 0 |   |   |   | Execute if the N flag is clear.
 * Overflow Set     0110    |   |   |   | 1 | Execute if the V flag is set.
 * Overflow Clear   0111    |   |   |   | 0 | Execute if the V flag is clear.
 * Higher           1000    |   | 0 | 1 |   | Execute if the C flag is set and Z flag is clear. Provides > test (unsigned).
 * Lower or Same    1001    |   | 1 | 0 |   | Execute if the C flag is clear and Z flag is set. Provides <= test (unsigned).
 * Greater or Equal 1010    | 0 |   |   | 0 | Execute if the N flag is clear and V flag is clear, 
 *                          | 1 |   |   | 1 |  or N flag is set and V flag is set. Provides >= test for signed comparison.
 *              
 * Less Than        1011    | 0 |   |   | 1 | Execute if the N flag is clear and V flag is set, 
 *                          | 1 |   |   | 0 |  or N flag is set and V flag is clear. Provides a < test for signed comparison.
 *                          
 * Greater Than     1100    | 0 | 0 |   | 0 | Same as GE but Z flag must be clear. Provides > test for signed comparison.
 *                          | 1 | 0 |   | 1 |
 * Less or Equal    1101    | 0 |   |   | 1 | Same as LT but also true if Z flag is set.
 *                          | 1 |   |   | 0 |
 *                          |   | 1 |   |   |
 * Always           1110    |   |   |   |   | Instruction always executes. Flags irrelevant.
 * Never            1111    |   |   |   |   | Instruction never executes. Flags irrelevant.
 *
 */ 
bool shouldExecute(INSTRUCTION instruction) {
    bool execute = false;
    int conditionCode = instruction.data32 >> COND_CODE_POS;

    switch (conditionCode) {
        case CONDITION_EQUAL: // 0000
            // Execute the instruction is the Z flag is set.
            execute = isSet(STATUS_Z);
            break;
        case CONDITION_NOT_EQUAL: // 0001
            // Execute the instruction is the Z flag is clear.
            execute = isClear(STATUS_Z);
            break;
        case CONDITION_CARRY_SET: // 0010
            // Execute the instruction if the C flag is set. Provides >= test (unsigned).
            execute = isSet(STATUS_C);
            break;
        case CONDITION_CARRY_CLEAR: // 0011
            // Execute if the C flag is clear. Provides < test for unsigned comparison.
            execute = isClear(STATUS_C);
            break;
        case CONDITION_MINUS: // 0100
            // Execute if the N flag is set.
            execute = isSet(STATUS_N);
            break;
        case CONDITION_PLUS: // 0101
            // Execute if the N flag is clear.
            execute = isClear(STATUS_N);
            break;
        case CONDITION_OVERFLOW_SET: // 0110
            // Execute if the V flag is set.
            execute = isSet(STATUS_V);
            break;
        case CONDITION_OVERFLOW_CLEAR: // 0111
            // Execute if the V flag is clear.
            execute = isClear(STATUS_V);
            break;
        case CONDITION_HIGHER: // 1000
            // Execute if the C flag is set and Z flag is clear. Provides > test (unsigned).
            execute = ( isSet(STATUS_C) && isClear(STATUS_Z) );
            break;
        case CONDITION_LOWER_OR_SAME: // 1001
            // Execute if the C flag is clear and Z flag is set. Provides <= test (unsigned).
            execute = ( isClear(STATUS_C) && isSet(STATUS_Z) );
            break;
        case CONDITION_GREATER_OR_EQUAL: // 1010
            // Execute if the N flag is clear and V flag is clear, or N flag is set and V flag is set. 
            // Provides >= test for signed comparison.
            execute = ( isClear(STATUS_N) && isClear(STATUS_V) ) || ( isSet(STATUS_N) && isSet(STATUS_V) );
            break;
        case CONDITION_LESS_THAN: // 1011
            // Execute if the N flag is clear and V flag is set, or N flag is set and V flag is clear. 
            // Provides a < test for signed comparison.
            execute = ( isSet(STATUS_N) && isClear(STATUS_V) ) || ( isClear(STATUS_N) && isSet(STATUS_V) );
            break;
        case CONDITION_GREATER_THAN: // 1100
            // Same as GE but Z flag must be clear. Provides > test for signed comparison.
            execute = ( isClear(STATUS_N) && isClear(STATUS_V) && isClear(STATUS_Z) ) || ( isSet(STATUS_N) && isSet(STATUS_V) && isClear(STATUS_Z));
            break;
        case CONDITION_LESS_OR_EQUAL: // 1101
            // Same as LT but also true if Z flag is set.
            execute = ( isSet(STATUS_N) && isClear(STATUS_V) ) || ( isClear(STATUS_N) && isSet(STATUS_V) ) || isSet(STATUS_Z);
            break;
        case CONDITION_ALWAYS: // 1110
            execute = true;
            break;
        case CONDITION_NEVER: // 1111
            execute = false;
            break;
    }

    if(VERBOSE) {
        printf("\tshould execute?: %s - %04X\n", execute? "true" : "false", instruction.data32);
    }
    return execute; 
}

int isSet(int flag) {
    return (STATUS_REGISTER & flag);
}

int isClear(int flag) {
    return ((~STATUS_REGISTER) & flag);
}

void setFlag(int flag) {
    STATUS_REGISTER = STATUS_REGISTER | flag;
}

void clearFlag(int flag) {
    STATUS_REGISTER = STATUS_REGISTER & (~flag);
}

void doDataProcessing() {
    printf("\tData Processing\n");
    // todo
}

void doBranch() {
    printf("\tBranch\n");
    // todo
}

void doDataTransfer() {
    printf("\tData Transfer\n");
    // todo
}

void doInterupt() {
    printf("\tInterupt\n");
    // todo
}