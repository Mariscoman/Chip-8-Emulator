#ifndef CPU_H
#define CPU_H

#include <stdbool.h>

#define MEM_BYTES 4096
#define STACK_SIZE 16
#define NUM_REGISTERS 16
#define VF 0xf
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define SCREEN_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT)
#define NUM_KEYS 16

#define KEY_DOWN 1
#define KEY_UP 0

typedef uint8_t byte;
typedef uint16_t word;

struct CPU {
	word opcode;

	byte MEMORY[MEM_BYTES];
	word stack[STACK_SIZE];
	byte sp;

	byte V[NUM_REGISTERS];
	
	word I;
	word PC;

	byte display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
	
	byte delayTimer;
	byte soundTimer;

	byte key[NUM_KEYS];

	bool drawFlag;
};

void initCPU(struct CPU *cpu);
int loadProgram(struct CPU *cpu, char *program);
void emulateCycle(struct CPU *cpu);
void updateTimers(struct CPU *cpu);

#endif