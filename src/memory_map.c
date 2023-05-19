#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum MEMORY_MAP {
	ROM,
	VRAM,
	RAM,
	WRAM,
	ECHO,
	OAM,
	IOAM,
	IO,
	HRAM,
	IE,
	N_MEMORY_MAP
};

enum MEMORY_IO {
	JOYP,
	SB,
	DC,
	DIV = 0x4,
	TIMA,
	TMA,
	TAC,
	IF = 0x0f,
	NR10,
	NR11,
	NR12,
	NR13,
	NR14,
	NR21 = 0x16,
	NR22,
	NR23,
	NR24,
	NR30,
	NR31,
	NR32,
	NR33,
	NR34,
	NR41 = 0x20,
	NR42,
	NR43,
	NR44,
	NR50,
	NR51,
	NR52,
	WAVE_PATTERN = 0X30,
	LCDC = 0x40,
	STAT,
	SCY,
	SCX,
	LY,
	LYC,
	DMA,
	BGP,
	OBP0,
	OBP1,
	WY,
	WX,
	BOOT_CONTROL = 0x50,
	IE = 0xff,
	N_MEMORY_IO
};

static uint8_t *mem;
static uint8_t *io;

struct map {
	uint8_t *ptr;
	uint16_t size;
} map[] = {
	{    &mem[0x0], 0x7fff },
	{ &mem[0x8000], 0x1fff },
	{ &mem[0xa000], 0x1fff },
	{ &mem[0xc000], 0x1fff },
	{ &mem[0xe000], 0x1dff },
	{ &mem[0xfe00],   0x9f },
	{ &mem[0xfea0],   0x5f },
	{ &mem[0xff00],   0x7f },
	{ &mem[0xff80],   0x7e },
	{ &mem[0xffff],   0x01 }
};

void
memory_map_init()
{
	mem = malloc(0x10000);
	memset (mem, 0, 0x10000);

	io = &mem[IO];
}
