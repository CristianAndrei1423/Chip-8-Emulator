// emulator structure and function prototypes
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    uint8_t memory[4096];
    uint8_t V[16]; // data registers
    uint16_t I; // index register
    uint16_t pc; // program counter

    uint16_t stack[16]; // subroutine stack
    uint8_t sp; // stack pointer

    uint8_t delay_timer; // timer for delays to 60 Hz
    uint8_t sound_timer; // time for sound to 60 Hz

    uint8_t keypad[16]; // state of each key on the keyboard
    uint32_t video[64 * 32]; // monitor

    uint16_t opcode; // current instruction
} Chip8;

// initialize hardware
void init_chip8(Chip8* chip8);

/*
Read chip-8 game file.
0 - file does not exist
-1 - error
else -> bytes read
*/
uint8_t load_chip8(Chip8* chip8, char* path);

/* loads the next operation into opcode variable
and increases op by 2 */
void fetch_chip8(Chip8* chip8);

/* decode the operation in opcode and
compute the instruction */
void decode_execute_chip8(Chip8* chip8);