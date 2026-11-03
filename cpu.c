#define CPU_C

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "cpu.h"

#define FONT_START 0x050
#define USABLE_MEMORY_START 0x200

#define OPCODE_MASK 0xf000
#define HALF_OPERATION_MASK 0x00ff
#define SUB_OPERATION_MASK 0x000f
#define DIR_MASK 0x0fff
#define FIRST_REGISTER(opcode) ((opcode & 0x0f00) >> 8)
#define SECOND_REGISTER(opcode) ((opcode & 0x00f0) >> 4)
#define BYTE_CONSTANT_MASK 0x00ff
#define HALF_BYTE_CONSTANT_MASK 0x000f

const byte FONTSET[80] = { 
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

static void fullOperation(struct CPU *cpu);
static void halfOperation(struct CPU *cpu);
static void logicOperation(struct CPU *cpu);
static void typeFOperation(struct CPU *cpu);

void initCPU(struct CPU *cpu) {
	cpu->PC = USABLE_MEMORY_START;
	cpu->I = 0;
	cpu->opcode = 0;
	cpu->sp = 0;

	memset(cpu->MEMORY, 0, MEM_BYTES * sizeof(cpu->MEMORY[0]));
	memset(cpu->stack, 	0, STACK_SIZE * sizeof(cpu->stack[0]));
	memset(cpu->V, 		0, NUM_REGISTERS * sizeof(cpu->V[0]));
	memset(cpu->display, 0, SCREEN_SIZE * sizeof(cpu->display[0]));

	cpu->delayTimer = 0;
	cpu->soundTimer = 0;
	
	cpu->drawFlag = false;

	for(int i = 0; i < 80; i++) {
		cpu->MEMORY[FONT_START + i] = FONTSET[i];
	}

	srand(time(NULL));
}

int loadProgram(struct CPU *cpu, char *program) {
	FILE *file;
	if((file = fopen(program, "rb")) == NULL) {
		fprintf(stderr, "Error al abrir el programa\n");
		return (EXIT_FAILURE);
	}

	/* Measure file */
	if(fseek(file, 0, SEEK_END) != 0) {
		perror("Error al medir el programa\n");
		fclose(file);
		return (EXIT_FAILURE);
	}
	long fileSize = ftell(file);
	if(fileSize < 0) {
		fprintf(stderr, "Error: al leer el tamaño del programa\n");
		fclose(file);
		return (EXIT_FAILURE);
	}
	rewind(file);
	const size_t maxMemoryAvailable = MEM_BYTES - USABLE_MEMORY_START;
	if((size_t)fileSize > maxMemoryAvailable) {
		fprintf(stderr, "Error: El programa supera la memoria disponible\n");
		fclose(file);
		return (EXIT_FAILURE);
	}

	/* Copy file to memory */
	size_t read = fread(&cpu->MEMORY[USABLE_MEMORY_START], sizeof(cpu->MEMORY[0]), (size_t)fileSize, file);
	fclose(file);
	if(read != (size_t)fileSize) {
		fprintf(stderr, "Error: No se pudo leer el programa completo\n");
		return (EXIT_FAILURE);
	}
	
	return EXIT_SUCCESS;
}

void emulateCycle(struct CPU *cpu) {
	/* Fetch opcode */
	cpu->opcode = cpu->MEMORY[cpu->PC] << 8 | cpu->MEMORY[cpu->PC + 1];

	cpu->PC += 2;
	cpu->drawFlag = false;

	/* Execute opcode */
	if((cpu->opcode & OPCODE_MASK) != 0x0000) {
		fullOperation(cpu);
	} else {
		halfOperation(cpu);
	}
}

void updateTimers(struct CPU *cpu) {	
	if(cpu->delayTimer > 0) cpu->delayTimer--;
	if(cpu->soundTimer > 0) cpu->soundTimer--;
}

static void logicOperation(struct CPU *cpu) {
	byte r1 = FIRST_REGISTER(cpu->opcode),
		 r2 = SECOND_REGISTER(cpu->opcode);
	switch(cpu->opcode & SUB_OPERATION_MASK) {
		case 0x0000: { // 8xy0 - LD Vx, Vy
			cpu->V[r1] = cpu->V[r2];
			break;
		}
		case 0x0001: { // 8xy1 - OR Vx, Vy
			cpu->V[r1] |= cpu->V[r2];
			break;
		}
		case 0x0002: { // 8xy2 - AND Vx, Vy
			cpu->V[r1] &= cpu->V[r2];
			break;
		}
		case 0x0003: { // 8xy3 - XOR Vx, Vy
			cpu->V[r1] ^= cpu->V[r2];
			break;
		}
		case 0x0004: { // 8xy4 - ADD Vx, Vy
			byte flag = (cpu->V[r2] > (0xff - cpu->V[r1])) ? 1 : 0;
			cpu->V[r1] += cpu->V[r2];
			cpu->V[VF] = flag;
			break;
		}
		case 0x0005: { // 8xy5 - SUB Vx, Vy
			byte flag = (cpu->V[r1] >= cpu->V[r2]) ? 1 : 0;
			cpu->V[r1] -= cpu->V[r2];
			cpu->V[VF] = flag;
			break;
		}
		case 0x0006: { // 8xy6 - SHR Vx {, Vy}
			byte flag = (cpu->V[r1] & 0x1);
			cpu->V[r1] >>= 1;
			cpu->V[VF] = flag;
			break;
		}
		case 0x0007: { // 8xy7 - SUBN Vx, Vy
			byte flag = (cpu->V[r1] <= cpu->V[r2]) ? 1 : 0;
			cpu->V[r1] = cpu->V[r2] - cpu->V[r1];
			cpu->V[VF] = flag;
			break;
		}
		case 0x000e: { // 8xyE - SHL Vx {, Vy}
			byte flag = ((cpu->V[r1] & 0x80) >> 7);
			cpu->V[r1] <<= 1;
			cpu->V[VF] = flag;
			break;
		}
		default: {
			fprintf(stderr, "Error: Código desconocido: 0x%X\n", cpu->opcode);
			break;
		}
	}
}

static void typeFOperation(struct CPU *cpu) {
	switch(cpu->opcode & HALF_OPERATION_MASK) {
		case 0x0007: { // Fx07 - LD Vx, DT
			byte r = FIRST_REGISTER(cpu->opcode);
			cpu->V[r] = cpu->delayTimer;
			break;
		}
		case 0x000a: { // Fx0A - LD Vx, K
			byte r = FIRST_REGISTER(cpu->opcode);
			bool pressed = true;
			while(pressed) {
				for(int i = 0; i < NUM_KEYS; i++) {
					if(cpu->key[i] == KEY_DOWN) {
						cpu->V[r] = i;
						pressed = false;
						break;
					}
				}
			}
			break;
		}
		case 0x0015: { // Fx15 - LD DT, Vx
			cpu->delayTimer = cpu->V[FIRST_REGISTER(cpu->opcode)];
			break;
		}
		case 0x0018: { // Fx18 - LD ST, Vx
			cpu->soundTimer = cpu->V[FIRST_REGISTER(cpu->opcode)];
			break;
		}
		case 0x001e: { // Fx1E - ADD I, Vx
			cpu->I += cpu->V[FIRST_REGISTER(cpu->opcode)];
			break;
		}
		case 0x0029: { // Fx29 - LD F, Vx
			byte sprite = cpu->V[FIRST_REGISTER(cpu->opcode)];
			cpu->I = FONT_START + (sprite * 5);
			break;
		}
		case 0x0033: { // Fx33 - LD B, Vx
			byte val = cpu->V[FIRST_REGISTER(cpu->opcode)];
			cpu->MEMORY[cpu->I] = (byte) (val / 100);
			cpu->MEMORY[cpu->I + 1] = (byte) (val / 10) % 10;
			cpu->MEMORY[cpu->I + 2] = (byte) (val % 100) % 10;
			break;
		}
		case 0x0055: { // Fx55 - LD [I], Vx
			byte r = FIRST_REGISTER(cpu->opcode);
			if(r >= NUM_REGISTERS) break;
			for(int i = 0; i <= r; i++) {
				cpu->MEMORY[cpu->I + i] = cpu->V[i];
			}
			break;
		}
		case 0x0065: { // Fx65 - LD Vx, [I]
			byte r = FIRST_REGISTER(cpu->opcode);
			if(r >= NUM_REGISTERS) break;
			for(int i = 0; i <= r; i++) {
				cpu->V[i] = cpu->MEMORY[cpu->I + i];
			}
			break;
		}
	}
}

static void fullOperation(struct CPU *cpu) {
	switch(cpu->opcode & OPCODE_MASK) {
		case 0x1000: { // 1NNN: JP addr
			cpu->PC = (cpu->opcode & DIR_MASK);
			break;
		}
		case 0x2000: { // 2NNN: CALL addr
			if(cpu->sp >= 16) break; // Prevent stack overflow
			cpu->stack[cpu->sp] = cpu->PC;
			cpu->sp++;
			cpu->PC = cpu->opcode & DIR_MASK;
			break;
		}
		case 0x3000: { // 3xkk: SE Vx, byte
			byte r = FIRST_REGISTER(cpu->opcode);
			word val = cpu->opcode & BYTE_CONSTANT_MASK;
			if(cpu->V[r] == val) cpu->PC += 2;
			break;
		}
		case 0x4000: { // 4xkk: SNE Vx, byte
			byte r = FIRST_REGISTER(cpu->opcode);
			byte val = cpu->opcode & BYTE_CONSTANT_MASK;
			if(cpu->V[r] != val) cpu->PC += 2;
			break;
		}
		case 0x5000: { // 5xy0 - SE Vx, Vy
			byte r1 = FIRST_REGISTER(cpu->opcode),
				 r2 = SECOND_REGISTER(cpu->opcode);
			if(cpu->V[r1] == cpu->V[r2]) cpu->PC += 2;
			break;
		}
		case 0x6000: { // 6xkk - LD Vx, byte
			byte r = FIRST_REGISTER(cpu->opcode);
			cpu->V[r] = cpu->opcode & BYTE_CONSTANT_MASK;
			break;
		}
		case 0x7000: { // 7xkk - ADD Vx, byte
			byte r = FIRST_REGISTER(cpu->opcode);
			cpu->V[r] += cpu->opcode & BYTE_CONSTANT_MASK;
			break;
		}
		case 0x8000: { // 8000
			logicOperation(cpu);
			break;
		}
		case 0x9000: { // 9xy0 - SNE Vx, Vy
			byte r1 = FIRST_REGISTER(cpu->opcode),
				 r2 = SECOND_REGISTER(cpu->opcode);
			cpu->PC += (cpu->V[r1] != cpu->V[r2]) ? 2 : 0;
			break;
		}
		case 0xa000: { // Annn - LD I, addr
			cpu->I = cpu->opcode & DIR_MASK;
			break;
		}
		case 0xb000: { // Bnnn - JP V0, addr
			cpu->PC = ((cpu->opcode & DIR_MASK) + cpu->V[0]);
			break;
		}
		case 0xc000: { // Cxkk - RND Vx, byte
			byte r = FIRST_REGISTER(cpu->opcode);
			cpu->V[r] = ((byte) rand() & (cpu->opcode & BYTE_CONSTANT_MASK));
			break;
		}
		case 0xd000: { // Dxyn - DRW Vx, Vy, nibble
			byte n = cpu->opcode & HALF_BYTE_CONSTANT_MASK;
			word x = cpu->V[FIRST_REGISTER(cpu->opcode)] % 64,
				 y = cpu->V[SECOND_REGISTER(cpu->opcode)] % 32;
			byte pixel;
			cpu->V[VF] = 0;
			for(int row = 0; row < n; row++) { // Get sprite row
				if(y + row >= 32) break; // Display's bottom edge
				pixel = cpu->MEMORY[cpu->I + row];

				for(int col = 0; col < 8; col++) { // Get sprite columns (row bits)
					if(x + col >= 64) break; // Display's right edge
				
					if((pixel & (0x80 >> col)) != 0) {
						int index = x + col + ((y + row) * 64);
						if(cpu->display[index] == 1) cpu->V[VF] = 1;
						cpu->display[index] ^= 1;
					}	
				}
			}
			cpu->drawFlag = true;
			break;
		}
		case 0xe000: { // E000
			switch(cpu->opcode & HALF_OPERATION_MASK) {
				case 0x009e: { // Ex9E - SKP Vx
					if(cpu->key[cpu->V[FIRST_REGISTER(cpu->opcode)]] == KEY_DOWN)
						cpu->PC += 2;
					break;
				}
				case 0x00a1: { // ExA1 - SKNP Vx
					if(cpu->key[cpu->V[FIRST_REGISTER(cpu->opcode)]] == KEY_UP)
						cpu->PC += 2;
					break;
				}
				default: {
					fprintf(stderr, "Error: Código desconocido: 0x%X\n", cpu->opcode);
					break;
				}
			}
			break;
		}
		case 0xf000: { // F000
			typeFOperation(cpu);
			break;
		}
		default: {
			fprintf(stderr, "Error: Código desconocido: 0x%X\n", cpu->opcode);
			break;
		}
	}
}

static void halfOperation(struct CPU *cpu) {
	switch(cpu->opcode & HALF_OPERATION_MASK) {
		case 0x00e0: // 00E0 - CLS
			memset(cpu->display, 0, SCREEN_SIZE * sizeof(cpu->display[0]));
			break;
		case 0x00ee: // 00EE - RET
			if(cpu->sp == 0) break;
			cpu->sp--;
			cpu->PC = cpu->stack[cpu->sp];
			break;
		default:
			fprintf(stderr, "Error: Código desconocido [0x0000]: 0x%X\n", cpu->opcode);
			break;
	}
}
