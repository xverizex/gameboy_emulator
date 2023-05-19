#include "sm83.h"

#define IO              0xff00

enum REGS {
	A,
	B,
	C,
	D,
	E,
	H,
	L,
	AF,
	BC,
	DE,
	HL
};

static struct cpu_sm83 cpu;
static uint8_t *io;

static uint16_t*
get_16 (uint8_t *dt)
{
	return (uint16_t *) dt;
}

static void 
t_states (uint32_t cycles)
{
}

static int
handle_opcode (uint8_t* dt, uint32_t* _idx)
{
	uint32_t pc = *_idx;

	switch (dt[idx]) {
		case 0x00:                     /* NOP; T-states = 4 */
			t_states (4);
			break;
		case 0x01:                     /* LD BC, d16; T-states = 12 */
			pc++;
			cpu.regs16.BC = *(get_16 (&dt[pc]));
			pc += 2;
			t_states (12);
			break;
		case 0x02:                     /* LD (BC), A; T-states = 8 */
			pc++;
			io[cpu.regs16.BC] = cpu.regs8.A;
			t_states (8);
			break;
		case 0x03:                     /* INC BC; T-states = 8 */
			pc++;
			cpu.regs16.BC++;
			t_states (8);
			break;
		case 0x04:                    /* INC B; T-states = 4 */
			pc++;
			cpu.flags &= cpu.regs8.B < ++cpu.regs8.B? 0: H;
			cpu.flags &= cpu.regs8.B == 0? Z: ~Z;
			cpu.flags &= ~N;
			t_states (4);
			break;
		case 0x05:                    /* DEC B; T-states = 4 */
			pc++;
			cpu.regs8.B--;
			cpu.flags &= cpu.regs8.B == 0? Z: 0;
			cpu.flags &= N;
			cpu.flags &= cpu.regs8.B & H? H: ~H;
			t_states (4);
			break;
		case 0x06:                   /* LD B, d8; T-states = 8 */
			pc++;
			cpu.regs8.B = io[dt[pc++]];
			t_states (8);
			break;
		case 0x07:                  /* RLCA; T-states = 4; Rotate register A left. */
			pc++;
			cpu.flags &= ~(Z | N | H);
			cpu.flags &= cpu.regs8.A & (1 << 0x7)? C: ~C;
			cpu.regs8.A <<= 1;
			t_states (4);
			break;
		case 0x08:                  /* LD (a16), SP; T-states = 20 */
			pc++;
			*(get_16 (&dt[pc])) = cpu.regs_sp.SP;
			pc += 2;
			t_states (20);
			break;
		case 0x09:                  /* ADD HL, BC; T-states = 8 */
			pc++;
			cpu.regs &= ~N;
			cpu.regs &= ((cpu.regs16.HL <= 0x8ff) & ((cpu.regs16.HL + cpu.regs16.BC) > 0x8ff)) ? H: ~H;
			cpu.regs &= (cpu.regs16.HL < (cpu.regs16.HL + cpu.regs16.BC)) ? ~C: C;
			cpu.regs16.HL += cpu.regs16.BC;
			t_states (8);
			break;
		case 0x0a:                 /* LD A, (BC); T-states = 8 */
			pc++;
			cpu.regs8.A = io[cpu.regs16.BC];
			t_states (8);
			break;
		case 0x0b:                 /* DEC BC; T-states = 8 */
			pc++;
			cpu.regs16.BC--;
			t_states (8);
			break;
		case 0x0c:                 /* INC C; T-states = 4 */
			pc++;
			cpu.flags &= ~N;
			cpu.flags &= ((cpu.regs8.C >= 0x8) && ((cpu.regs8.C + 1) < cpu.regs8.C))? H: ~H;
			cpu.flags &= ++cpu.regs8.C = 0 ? Z: ~Z;
			cpu.regs8.C++;
			t_states (4);
			break;
		case 0x0d:                 /* DEC C; T-states = 4 */
			pc++;
			cpu.regs8.C--;
			cpu.flags &= N;
			cpu.flags &= (cpu.regs8.C == 0) ? Z: ~Z;
			cpu.flags &= (cpu.regs8.C & (1 << 4)) ? H: ~H;
			t_states (4);
			break;
		case 0x0e:                 /* LD C, d8; T-states = 8 */
			pc++;
			cpu.regs8.C = io[mem[pc++]];
			t_states (8);
			break;
		case 0x0f:                 /* RRCA; T-states = 4 */
			pc++;
			cpu.flags &= (cpu.regs8.A & 0x1) ? C: ~C;
			cpu.flags &= ~(N | H | Z);
			cpu.regs8.A >>= 1;
			t_states (4);
			break;
		case 0x10:                 /* STOP d8; T-states = 4 */
			pc++;
			/*
			 * TODO: implement stop. What is need for it? 
			 * Enter CPU very low power mode. Also used to switch between double and normal speed CPU modes in GBC.
			 */

			t_states (4);
			break;
		case 0x11:                 /* LD DE, d16; T-states = 12 */
			pc++;
			cpu.regs16.DE = *(get_16 (&dt[pc]));
			pc += 2;
			t_states (12);
			break;
		case 0x12:                 /* LD (DE), A; T-states = 8 */
			pc++;
			io[cpu.regs16.DE] = cpu.regs8.A;
			t_states (8);
			break;
		case 0x13:                 /* INC DE; T-states = 8 */
			pc++;
			cpu.regs16.DE++;
			s_states (8);
			break;
		case 0x14:                 /* INC D; T-states = 8 */
			pc++;
			cpu.flags &= cpu.regs8.D < ++cpu.regs8.D? 0: H;
			cpu.flags &= cpu.regs8.D == 0? Z: ~Z;
			cpu.flags &= ~N;
			t_states (4);
			break;
		case 0x15:                 /* DEC D; T-states = 4 */
			pc++;
			cpu.regs8.D--;
			cpu.flags &= cpu.regs8.D == 0? Z: 0;
			cpu.flags &= N;
			cpu.flags &= cpu.regs8.D & H? H: ~H;
			t_states (4);
			break;
		case 0x16:                 /* LD D, d8; T-states = 8 */
			pc++;
			cpu.regs8.D = io[dt[pc++]];
			t_states (8);
			break;
		case 0x17:                  /* RLA; T-states = 4; Rotate register A left through carry. */
			/*
			 * TODO: I don't understand what is difference between RLA and RLCA because it both shifting left.
			 * 	What I do, if the difference between these opcodes with carry.
			 */
			pc++;
			cpu.flags &= ~(Z | N | H);
			cpu.flags &= cpu.regs8.A & (1 << 0x7)? C: ~C;
			cpu.regs8.A <<= 1;
			t_states (4);
			break;
		case 0x18:                  /* JR r8; T-states = 12 */
			pc++;
			pc += (int8_t) mem[pc];
			break;
		case 0x19:                  /* ADD HL, DE; T-states = 8 */
			pc++;
			cpu.regs &= ~N;
			cpu.regs &= ((cpu.regs16.HL <= 0x8ff) & ((cpu.regs16.HL + cpu.regs16.DE) > 0x8ff)) ? H: ~H;
			cpu.regs &= (cpu.regs16.HL < (cpu.regs16.HL + cpu.regs16.DE)) ? ~C: C;
			cpu.regs16.HL += cpu.regs16.DE;
			t_states (8);
			break;
		case 0x1a:                 /* LD A, (DE); T-states = 8 */
			pc++;
			cpu.regs8.A = io[cpu.regs16.DE];
			t_states (8);
			break;
		case 0x1b:                 /* DEC DE; T-states = 8 */
			pc++;
			cpu.regs16.DE--;
			t_states (8);
			break;
		case 0x1c:                 /* INC E; T-states = 4 */
			pc++;
			cpu.flags &= ~N;
			cpu.flags &= ((cpu.regs8.E >= 0x8) && ((cpu.regs8.E + 1) < cpu.regs8.E))? H: ~H;
			cpu.flags &= ++cpu.regs8.E = 0 ? Z: ~Z;
			cpu.regs8.E++;
			t_states (4);
			break;
		case 0x1d:                 /* DEC E; T-states = 4 */
			pc++;
			cpu.regs8.E--;
			cpu.flags &= N;
			cpu.flags &= (cpu.regs8.E == 0) ? Z: ~Z;
			cpu.flags &= (cpu.regs8.E & (1 << 4)) ? H: ~H;
			t_states (4);
			break;
		case 0x1e:                 /* LD E, d8; T-states = 8 */
			pc++;
			cpu.regs8.E = io[mem[pc++]];
			t_states (8);
			break;
		case 0x1f:                 /* RRA; T-states = 4 */
			/*
			 * TODO: I don't understand what is difference between RRA and RRCA because it both shifting right.
			 * 	What I do, if the difference between these opcodes with carry.
			 */
			pc++;
			cpu.flags &= (cpu.regs8.A & 0x1) ? C: ~C;
			cpu.flags &= ~(N | H | Z);
			cpu.regs8.A >>= 1;
			t_states (4);
			break;


	}

	*_idx = pc;
}

void
cpu_init ()
{
	io = get_memory_io ();
}

void
cpu_loop (uint8_t *cartridge_data)
{
	uint32_t idx = 0;
	int play = 1;
	while (play) {

		play = handle_opcode (cartridge_data, &idx);
	}
}
