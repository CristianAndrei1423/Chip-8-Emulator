// processor logic
#include "chip8.h"

static uint8_t chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10,
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    0xF0, 0x80, 0xF0, 0x80, 0x80 
};

void init_chip8(Chip8* chip8) {

    // zero out whole structure
    memset(chip8, 0, sizeof(Chip8));

    // program counter initialized to 0x200
    chip8->pc = 0x200;

    // load fonts into stack
	memcpy(chip8->memory + 0x50, chip8_fontset, sizeof(chip8_fontset));
}

uint8_t load_chip8(Chip8* chip8, char* path){

    FILE *ptr;

    ptr = fopen(path, "rb");

	if(!ptr) return 0;

	// make sure to load the game in the unreserved part of the memory
	size_t bytesRead = fread((chip8->memory + 0x200), 1, sizeof(chip8->memory) - 0x200, ptr);

	if(ferror(ptr)){
		fclose(ptr);
		return -1;
	}

	fclose(ptr);

	return (int)bytesRead;
}

void fetch_chip8(Chip8* chip8){

	chip8->opcode = (uint16_t) chip8->memory[chip8->pc];

	chip8->opcode <<= 8;

	chip8->opcode |= (uint16_t) chip8->memory[chip8->pc + 1];

	chip8->pc += 2;
}

void decode_execute_chip8(Chip8* chip8) {
	uint16_t operation = (chip8->opcode & 0xF000) >> 12;

	uint16_t X, Y, N, NN, NNN;
	X = (uint8_t)((chip8->opcode & 0x0F00) >> 8);
	Y = (uint8_t)((chip8->opcode & 0x00F0) >> 4);
	N = (uint8_t)chip8->opcode & 0x000F;
	NN = (uint8_t)chip8->opcode & 0x00FF;
	NNN = chip8->opcode & 0x0FFF;
	uint8_t x = chip8->V[X], y = chip8->V[Y];

	switch (operation) {
		case 1:
			// Jump
			chip8->pc = NNN;
			break;
		case 2:
			// Call
			chip8->stack[chip8->sp++] = chip8->pc;
			chip8->pc = NNN;
			break;
		case 3:
			// Skip
			if(chip8->V[X] == NN)
				chip8->pc += 2;
			break;
		case 4:
			// Skip
			if(chip8->V[X] != NN)
				chip8->pc += 2;
			break;
		case 5:
			// Skip
			if(chip8->V[X] == chip8->V[Y])
				chip8->pc += 2;
			break;
		case 6:
			// Set
			chip8->V[X] = NN;
			break;
		case 7:
			// Add
			chip8->V[X] += NN;
			break;
		case 9:
			// Skip
			if(chip8->V[X] != chip8->V[Y])
				chip8->pc += 2;
			break;
		case 0xA:
			// Set Index
			chip8->I = NNN;
			break;
		case 0xB:
			// Jump with offset
			chip8->pc = NNN + chip8->V[0];
			break;
		case 0xC:
			// Random
			chip8->V[X] = ((uint8_t)(rand() % 256)) & NN;
			break;
		case 0xD:
			// Display
			
			// first initialize start coordonates
			uint8_t Xc = chip8->V[X] % 64;
			uint8_t Yc = chip8->V[Y] % 32;

			// Draw
			chip8->V[0xF] = 0;
			for(int i = 0; i < N; i++){
				uint8_t mask = 0x80;
				for(int j = 0; j < 8; j++){
					if(chip8->memory[chip8->I + i] & mask){
						if(Xc + j >= 64 || Yc + i >= 32)
							continue;

						uint16_t memC = (Xc + j) + (Yc + i) * 64;
						chip8->video[memC] ^= 0xFFFFFFFF;
						if(chip8->video[memC] == 0)
							chip8->V[0xF] = 1;
					}
					mask >>= 1;
				}
			}

			break;
		case 0:
			switch(chip8->opcode) {
				case 0x00E0:
					// Clear
					memset(chip8->video, 0, sizeof(chip8->video));
					break;
				case 0x00EE:
					// Return
					chip8->pc = chip8->stack[--chip8->sp];
					break;
			}
			break;
		case 8:
			switch(N){
				case 0:
					chip8->V[X] = chip8->V[Y];
					break;
				case 1:
					chip8->V[0xF] = 0;
					if(X != 0xF)
						chip8->V[X] = x | y;
					break;
				case 2:
					chip8->V[0xF] = 0;
					if(X != 0xF)
						chip8->V[X] = x & y;
					break;
				case 3:
					chip8->V[0xF] = 0;
					if(X != 0xF)
						chip8->V[X] = x ^ y;
					break;
				case 4:
					if(x + y >= 256)
						chip8->V[0xF] = 1;
					else chip8->V[0xF] = 0;
					if(X != 0xF)
						chip8->V[X] += y;
					break;
				case 5:
					if(x < y)
						chip8->V[0xF] = 0;
					else chip8->V[0xF] = 1;
					if(X != 0xF)
						chip8->V[X] -= y;
					break;
				case 6:
					chip8->V[0xF] = (y & 0x001) == 1;
					if(X != 0xF)
						chip8->V[X] = y >> 1;
					break;
				case 7:
					if(y < x)
						chip8->V[0xF] = 0;
					else chip8->V[0xF] = 1;
					if(X != 0xF)
						chip8->V[X] = y - x;
					break;
				case 0xE:
					chip8->V[0xF] = (y & 0x80) >> 7;
					if(X != 0xF)
						chip8->V[X] = y << 1;
					break;
			}
			break;
		case 0xE:
			if(NN == 0x9E && chip8->keypad[chip8->V[X]] == 1)
				chip8->pc += 2;
			else if(NN == 0xA1 && chip8->keypad[chip8->V[X]] == 0)
				chip8->pc += 2;
			break;
		case 0xF:
			switch(NN) {
				case 0x07:
					chip8->V[X] = chip8->delay_timer;
					break;
				case 0x15:
					chip8->delay_timer = chip8->V[X];
					break;
				case 0x18:
					chip8->sound_timer = chip8->V[X];
					break;
				case 0x1E:
					chip8->I += chip8->V[X];
					break;
				case 0x29:
					// Load Font
					chip8->I = chip8->V[X] * 5 + 0x50;
					break;
				case 0x33:
					// Binary Coded Decimal
					chip8->memory[chip8->I] = chip8->V[X] / 100;
					chip8->memory[chip8->I + 1] = (chip8->V[X] / 10) % 10;
					chip8->memory[chip8->I + 2] = chip8->V[X] % 10;
					break;
				case 0x55:
					for(int i = chip8->I; i <= chip8->I + X; i++){
						chip8->memory[i] = chip8->V[i - chip8->I];
					}
					chip8->I = chip8->I + X + 1;
					break;
				case 0x65:
					for(int i = chip8->I; i <= chip8->I + X; i++){
						chip8->V[i - chip8->I] = chip8->memory[i];
					}
					chip8->I = chip8->I + X + 1;
					break;
			}
		break;
	}
}