#ifndef CPU_H
#define CPU_H

#include <stdint.h>

/*
 * DMG: clocked at 4.194304 MHz
 * CGB: optionally clocked at 8.388608 MHz
 * T states and M cycles
 * M machine 1:4 clock base unit of CPU intrtuctions
 * T states or T cycles transistor states 1:1 clock
 */

struct regs_8bit {
	uint8_t A;
	uint8_t B;
	uint8_t C;
	uint8_t D;
	uint8_t E;
	uint8_t H;
	uint8_t L;
};

struct regs_16bit {
	uint16_t AF;
	uint16_t BC;
	uint16_t DE;
	uint16_t HL;
};

struct regs_sp16bit {
	uint16_t PC;
	uint16_t SP;
};

#define C               (1 << 4)  /* CARRY FLAG */
#define H               (1 << 5)  /* HALF-CARRY FLAG */
#define N               (1 << 6)  /* NEGATIVE FLAG */
#define Z               (1 << 7)  /* ZERO FLAG */

struct cpu {
	struct regs_8bit regs8;
	struct regs_16bit regs16;
	struct regs_sp16bit regs_sp;
	uint8_t flags;
};

#endif
