/*
 * Compile using:
 * 			gcc armpit.c -std=c99 -o armpit
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>     // for strcat
#include <stdbool.h>	// requires C99 and is considered the "standard way" to do booleans
#include <getopt.h>		// convenient argument parsing. Works on Linux and Mac OSX
//#include <unistd.h>	// can either use unistd.h or getopt.h for handling args.

#include "armpit.h"     // contains some defines and my function prototypes.


// registers:
REGISTER registers[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

REGISTER* R0 = &registers[0];
REGISTER* R1 = &registers[1];
REGISTER* R2 = &registers[2];
REGISTER* R3 = &registers[3];
REGISTER* R4 = &registers[4];
REGISTER* R5 = &registers[5];
REGISTER* R6 = &registers[6];
REGISTER* R7 = &registers[7];
REGISTER* R8 = &registers[8];
REGISTER* R9 = &registers[9];
REGISTER* R10 = &registers[10];
REGISTER* R11 = &registers[11];
REGISTER* R12 = &registers[12]; // General Purpose (but often used as the stack pointer).   
REGISTER* STACK_POINTER = &registers[12];     // R12 is the SP (stack pointer)
REGISTER* LINK_REGISTER = &registers[14];     // R14 is general purpose or the LR (link register)
REGISTER* PROGRAM_COUNTER = &registers[15];   // R15 is used as program counter

// (note: the bottom 2 bits of the PC are always 0 0 when memory is accessed.
//                  i.e. memory is implicitly accessed on a word aligned boundary.)



/*
 * Global Variables:
 */
char USAGE[] = "\nARMpit - ARM processor imitation technology (2013 - Samael Bate)\n\nUsage: armpit [arguments] [file ..]\n\nArguments:\n -f <filename>\tspecify a file of ARM assembly to be processed.\n -v\t\tverbose output\n -h\t\thelp";

static bool VERBOSE = false;	// -v option
const char *fileName;			// -f option
FILE *file;
static const char *optString = "f:vh"; // requires <getopt.h> or <unistd.h>

INSTRUCTION memory[1024]; // 4 kilobytes of RAM in this example (since 1 instruction = 4 bytes)



// function from: stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
// const char *byte_to_binary(int x, int max, const int bits) {
//     static int size = bits+1;
//     static char b[size];
//     b[0] = '\0';
    
//     if(!(max > 1)) {
//         max = 128;
//     }

//     for (max > 0; max >>= 1) {
//         strcat(b, ((x & max) == max) ? "1" : "0");
//     }

//     x,max=0;
//     return b;
// }

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
        }
        opt = getopt( argc, argv, optString );
    }
    
    printf("Verbose Logging: %s\n", VERBOSE? "true" : "false");
    printf("Input File: %s\n\n", fileName? fileName : "none");

    init(); // load instructions onto stack

    // fetch
    INSTRUCTION instruction;
    int amountOfInstructions = *PROGRAM_COUNTER;
    
    for(int counter = 0; counter < amountOfInstructions; counter++) {
        instruction = fetchInstruction();

        // check 1st 4 bits (if 1110 execute)
        if(shouldExecute(instruction)) {

            InstructionType type = getInstructionType(instruction);

            switch(type) {
                case ALU:
                    doDataProcessing(instruction);
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
        printf("\nPress Enter key to continue\n");
        getchar();
    }   

    printf("\nExiting program\n\n");
    exit(EXIT_SUCCESS);
}

void init() {

    STATUS_REGISTER = 0;
    *LINK_REGISTER = 0;
    *STACK_POINTER = 0;
    *PROGRAM_COUNTER = 0;

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
		printf("\tLoading Instruction: %04X onto stack at position %i\n", i.data32, *PROGRAM_COUNTER);

	// now to place it onto my virtual stack
    memory[*PROGRAM_COUNTER] = i;
    (*PROGRAM_COUNTER)++;
}

INSTRUCTION fetchInstruction() {
    if(PROGRAM_COUNTER > 0)
        (*PROGRAM_COUNTER)--; // as the position is incremented after loading each instruction, it needs to be decreased 
    INSTRUCTION instruction = memory[*PROGRAM_COUNTER];
    printf("\n\tFetching instruction: %04X from position %i\n", instruction.data32, *PROGRAM_COUNTER);
    return instruction;
}

InstructionType getInstructionType(INSTRUCTION instruction) {
    InstructionType type;

    type = (instruction.data32 >> 26) & BITMASK_2_BIT;

    if(VERBOSE) {
        char *typeName;

        switch(type) {
            case ALU: typeName = "ALU"; break;
            case BRANCH: typeName = "Branch"; break;
            case DATA_TRANSFER: typeName = "Data Transfer"; break;
            case INTERUPT: typeName = "Interupt"; break;
        }
    
        printf("\tInstruction is type %d:\t%s\n", type, typeName);
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

bool isImmediateValue(INSTRUCTION instruction) {
    bool immediate = (instruction.data32 >> 25) & BITMASK_1_BIT;
    printf("\tImmediate Bit:\t%s \n", immediate? "set" : "not set");
    return immediate;
}

bool statusBitSet(INSTRUCTION instruction) {
    bool updateStatus = (instruction.data32 >> 20) & BITMASK_1_BIT;
    printf("\tStatus Bit:\t%s \n", updateStatus? "set" : "not set");
    return updateStatus;
}

REGISTER getRegister(INSTRUCTION instruction, int shiftAmount) {
    int regNo = (instruction.data32 >> shiftAmount) & BITMASK_4_BIT;
    printf("\tRegister:\t%i \n", regNo);

    return registers[regNo];
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

void doDataProcessing(INSTRUCTION instruction) {
    printf("\tData Processing...\n");
    
    bool immediate = isImmediateValue(instruction);
    bool updateStatus = statusBitSet(instruction);

    REGISTER regN = getRegister(instruction, 16); // bits 16 to 19
    REGISTER regDest = getRegister(instruction, 12); // bits 12 to 15

    int opCode = (instruction.data32 >> OP_CODE_POS) & BITMASK_4_BIT;

    printf("\tOp Code:\t%i\n", opCode);

    switch (opCode) {
        case OP_AND: // 0000
            // doAND();
            printf("\tProcessing AND function\n");
            break;
        case OP_EOR: // 0001
            printf("\tProcessing Exclusive-OR function\n");
            break;
        case OP_SUB: // 0010
            printf("\tProcessing SUB function\n");
            break;
        case OP_RSB: // 0011
            printf("\tProcessing RSB function\n");
            break;
        case OP_ADD: // 0100
            printf("\tProcessing ADD function\n");

            // todo, something like:
            // if(immediate) { doAdd(updateStatus, regDest, regN, shiftAmount, value); } else {doAdd(updateStatus, regDest, regN, shiftAmount, register);}
            break;
        case OP_ADC: // 0101
            printf("\tProcessing ADC function\n");
            break;
        case OP_SBC: // 0110
            printf("\tProcessing SBC function\n");
            break;
        case OP_RSC: // 0111
            printf("\tProcessing RSC function\n");
            break;
        case OP_TST: // 1000
            printf("\tProcessing TST function\n");
            break;
        case OP_TEQ: // 1001
            printf("\tProcessing TEQ function\n");
            break;
        case OP_CMP: // 1010
            printf("\tProcessing Compare function\n");
            break;
        case OP_CMN: // 1011
            printf("\tProcessing CMN function\n");
            break;
        case OP_ORR: // 1100
            printf("\tProcessing ORR function\n");
            break;
        case OP_MOV: // 1101
            printf("\tProcessing MOV function\n");
            break;
        case OP_BIC: // 1110
            printf("\tProcessing BIC function\n");
            break;
        case OP_MVN: // 1111
            printf("\tProcessing MVN function\n");
            break;
    }
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




/*
 * Adds op1Value to op2Value and stores in the specified register
 *
 * enter:   regNumber - the number of the destination register (0-15)
 *          op1Value  - the first value to be added
 *          op2Value  -  the second value to be added
 *          setSR     - flag, if non-zero then the status register should be updated, else it shouldn't
 */
// void doAdd(int regNumber, REGISTER op1Value, REGISTER op2Value, bool setSR) {
//     REGISTER result = op1Value + op2Value;

//     registers[ regNumber ] = result;

//     if ( setSR ) {
//         // set the SR flags by examining the result. This can be difficult for certain flags!

//         // some are simple, e.g. to set or clear the Zero flag, could use -
//         if ( result == 0 ) {
//             setFlag(STAT_Z);
//         } else {
//             clearFlag(STAT_Z);
//         }
//     }
// }