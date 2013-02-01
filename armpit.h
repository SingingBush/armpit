#ifndef __ARMPIT_H__
#define __ARMPIT_H__


typedef uint8_t BYTE;

typedef union INSTRUCTION {
    uint32_t data32;
    BYTE byte[4]; // endian issue - when getting an individual byte you need to reverse the order
} INSTRUCTION;

typedef enum InstructionType {
    ALU = 0,
    BRANCH,
    DATA_TRANSFER,
    INTERUPT
} InstructionType;

typedef uint32_t REGISTER;



/*  ------------------------------------------------------------------------
 *		Function Prototypes
 *  ----------------------------------------------------------------------*/

void init();
void displayUsage();

void loadInstruction(INSTRUCTION);

INSTRUCTION fetchInstruction();

bool shouldExecute();
InstructionType getInstructionType(INSTRUCTION); // returns enum value

void doDataProcessing();
void doBranch();
void doDataTransfer();
void doInterupt();

int isSet(int);
int isClear(int);
void setFlag(int);
void clearFlag(int);

#endif
