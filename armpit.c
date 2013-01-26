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





// function prototypes:

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
