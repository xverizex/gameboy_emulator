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

static struct cpu cpu;
static uint8_t *io;

static uint16_t*
get_16 (uint8_t *mem)
{
	return (uint16_t *) mem;
}

static void 
t_states (uint32_t cycles)
{
}

static int
handle_opcode (uint8_t* mem, uint32_t* _idx)
{
	uint32_t pc = *_idx;

	switch (mem[idx]) {
		case 0x00:                     /* NOP; T-states = 4 */
			t_states (4);
			break;
		case 0x01:                     /* LD BC, d16; T-states = 12 */
			pc++;
			cpu.regs16.BC = *(io + *(get_16 (&mem[pc])));
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
			cpu.flags |= ((cpu.regs8.B <= 0xf) && ((cpu.regs8.B + 0x01) > 0xf)? H: 0;
			cpu.flags |= cpu.regs8.B == 0? Z: 0;
			cpu.flags &= ~N;
			cpu.regs8.B++;
			t_states (4);
			break;
		case 0x05:                    /* DEC B; T-states = 4 */
			pc++;
			cpu.flags |= ((cpu.regs8.B - 1) == 0) ? Z: 0;
			cpu.flags &= N;
			cpu.flags |= (cpu.regs8.B & (1 << 4))? H: 0;
			cpu.regs8.B--;
			t_states (4);
			break;
		case 0x06:                   /* LD B, d8; T-states = 8 */
			pc++;
			cpu.regs8.B = io[mem[pc++]];
			t_states (8);
			break;
		case 0x07:                  /* RLCA; T-states = 4; Rotate register A left. */
			pc++;
			cpu.flags &= ~(Z | N | H);
			cpu.flags |= cpu.regs8.A & (1 << 7)? C: 0;
			cpu.regs8.A <<= 1;
			t_states (4);
			break;
		case 0x08:                  /* LD (a16), SP; T-states = 20 */
			pc++;
			*(io + *(get_16 (&mem[pc]))) = cpu.regs_sp.SP;
			pc += 2;
			t_states (20);
			break;
		case 0x09:                  /* ADD HL, BC; T-states = 8 */
			pc++;
			cpu.flags &= ~N;
			cpu.flags |= ((cpu.regs16.HL <= 0x8ff) & ((cpu.regs16.HL + cpu.regs16.BC) > 0x8ff)) ? H: 0;
			cpu.flags |= (cpu.regs16.HL < (cpu.regs16.HL + cpu.regs16.BC)) ? 0: C;
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
			cpu.flags |= ((cpu.regs8.C <= 0xf) && ((cpu.regs8.C + 1) > 0xf))? H: 0;
			cpu.flags |= ++cpu.regs8.C = 0 ? Z: 0;
			cpu.regs8.C++;
			t_states (4);
			break;
		case 0x0d:                 /* DEC C; T-states = 4 */
			pc++;
			cpu.flags &= N;
			cpu.flags |= ((cpu.regs8.C - 1) == 0) ? Z: 0;
			cpu.flags |= (cpu.regs8.C & (1 << 4)) ? H: 0;
			cpu.regs8.C--;
			t_states (4);
			break;
		case 0x0e:                 /* LD C, d8; T-states = 8 */
			pc++;
			cpu.regs8.C = io[mem[pc++]];
			t_states (8);
			break;
		case 0x0f:                 /* RRCA; T-states = 4 */
			pc++;
			cpu.flags |= (cpu.regs8.A & 0x1) ? C: 0;
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
			cpu.regs16.DE = *(io + *(get_16 (&mem[pc])));
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
			cpu.flags |= cpu.regs8.D & (1 << 4) ? H: 0;
			cpu.flags |= ((cpu.regs8.D + 1) == 0) ? Z: 0;
			cpu.flags &= ~N;
			cpu.regs8.D++;
			t_states (4);
			break;
		case 0x15:                 /* DEC D; T-states = 4 */
			pc++;
			cpu.flags &= N;
			cpu.flags |= ((cpu.regs8.D - 1) == 0) ? Z: 0;
			cpu.flags |= ((cpu.regs8.D & (1 << 4)) ? H: 0;
			cpu.regs8.D--;
			t_states (4);
			break;
		case 0x16:                 /* LD D, d8; T-states = 8 */
			pc++;
			cpu.regs8.D = io[mem[pc++]];
			t_states (8);
			break;
		case 0x17:                  /* RLA; T-states = 4; Rotate register A left through carry. */
			/*
			 * TODO: I don't understand what is difference between RLA and RLCA because it both shifting left.
			 * 	What I do, if the difference between these opcodes with carry.
			 */
			pc++;
			cpu.flags &= ~(Z | N | H);
			cpu.flags |= cpu.regs8.A & (1 << 7)? C: 0;
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
			cpu.regs |= ((cpu.regs16.HL <= 0xfff) & ((cpu.regs16.HL + cpu.regs16.DE) > 0xfff)) ? H: 0;
			cpu.regs |= (cpu.regs16.HL < (cpu.regs16.HL + cpu.regs16.DE)) ? 0: C;
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
			cpu.flags |= ((cpu.regs8.E <= 0xf) && ((cpu.regs8.E + 1) > 0xf))? H: 0;
			cpu.flags |= ((cpu.regs8.E + 1) == 0) ? Z: 0;
			cpu.regs8.E++;
			t_states (4);
			break;
		case 0x1d:                 /* DEC E; T-states = 4 */
			pc++;
			cpu.flags &= N;
			cpu.flags |= ((cpu.regs8.E  - 1) == 0) ? Z: 0;
			cpu.flags |= (cpu.regs8.E & (1 << 4)) ? H: 0;
			cpu.regs8.E--;
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
			cpu.flags |= (cpu.regs8.A & 0x1) ? C: 0;
			cpu.flags &= ~(N | H | Z);
			cpu.regs8.A >>= 1;
			t_states (4);
			break;
		case 0x20:                /* JR NZ, e8; T-states: 12 / 8 */
			pc++;
			if ((cpu.flags & N) && (cpu.flags & Z))
				pc += (int8_t) io[mem[pc]];
			t_states (1);
			break;
		case 0x21:                /* LD HL, n16; T-states: 12 */
			pc++;
			cpu.regs16.HL = *(io + *(get_16 (&mem[pc])));
			pc += 2;
			t_states (12);
			break;
		case 0x22:                /* LD HL+, A; T-states: 8 */
			pc++;
			*(io + cpu.regs16.HL) = cpu.regs8.A;
			cpu.regs16.HL++;
			t_states (8);
			break;
		case 0x23:                /* INC HL; T-states: 8 */
			pc++;
			cpu.regs16.HL++;
			t_states (8);
			break;
		case 0x24:                /* INC H; T-states: 4 */
			pc++;
			cpu.flags &= ~N;
			cpu.flags |= ((cpu.regs8.H <= 0xf) && (cpu.regs8.H + 1 > 0xf)) ? H: 0;
			cpu.flags |= ((cpu.regs8.H + 1) == 0)? Z: 0;
			cpu.regs8.H++;
			t_states (4);
			break;
		case 0x25:                /* DEC H; T-states: 4 */
			pc++;
			cpu.flags &= N;
			cpu.flags |= ((cpu.regs8.D - 1) == 0) ? Z: 0;
			cpu.flags |= ((cpu.regs8.D & (1 << 4)) ? H: 0;
			t_states (4);
			break;
		case 0x26:               /* LD H, n8; T-states: 8 */
			pc++;
			cpu.regs8.H = io[mem[pc++]];
			t_states (8);
			break;
		case 0x27:               /* DAA; T-states: 4 */
			/*
			 * TODO: Decimal Adjust Accumulator to get a correct BCD representation after an arithmetic instruction.
			 */
			pc++;
			cpu.flags &= ~H;
			t_states (4);
			break;
		case 0x28:               /* JR Z, e8; T-states: 12/8 */
			pc++;
			if (cpu.flags & Z)
				pc += (int8_t) io[mem[pc]];
			t_states (1);
			break;
		case 0x29:               /* ADD HL, HL; T-states: 8 */
			pc++;
			cpu.regs &= ~N;
			cpu.regs |= ((cpu.regs16.HL <= 0xfff) & ((cpu.regs16.HL + cpu.regs16.HL) > 0xfff)) ? H: 0;
			cpu.regs |= (cpu.regs16.HL < (cpu.regs16.HL + cpu.regs16.HL)) ? 0: C;
			cpu.regs16.HL += cpu.regs16.HL;
			t_states (8);
			break;
		case 0x2a:               /* LD A, [HL+]; T-states: 8 */
			pc++;
			cpu.regs8.A = io[cpu.regs16.HL];
			cpu.regs16.HL++;
			break;
		case 0x2b:               /* DEC HL; T-states: 8 */
			pc++;
			cpu.regs16.HL--;
			break;
		case 0x2c:               /* INC E; T-states: 4 */
			pc++;
			cpu.flags &= ~N;
			cpu.flags |= ((cpu.regs8.L <= 0xf) && ((cpu.regs8.L + 1) > 0xf))? H: 0;
			cpu.flags |= ((cpu.regs8.L + 1) == 0) ? Z: 0;
			cpu.regs8.L++;
			t_states (4);
			break;
		case 0x2d:               /* DEC L; T-states: 4 */
			pc++;
			cpu.flags &= N;
			cpu.flags |= ((cpu.regs8.L  - 1) == 0) ? Z: 0;
			cpu.flags |= (cpu.regs8.L & (1 << 4)) ? H: 0;
			cpu.regs8.L--;
			t_states (4);
			break;
		case 0x2e:               /* LD L, n8; T-states: 8 */
			pc++;
			cpu.regs8.L = io[mem[pc++]];
			t_states (8);
			break;
		case 0x2f:               /* CPL; T-states: 4 */
			pc++;
			cpu.regs8.A = ~cpu.regs8.A;
			cpu.flags |= (N | H);
			t_states (4);
			break;
		case 0x30:               /* JR NC, e8; T-states: 12/8 */
			pc++;
			if ((cpu.flags & N) && (cpu.flags & C))
				pc += (int8_t) io[mem[pc]];
			t_states (1);
			break;
		case 0x31:               /* LD SP, n16; T-states: 12 */
			pc++;
			cpu.regs16.SP = *(io + *(get_16 (&mem[pc])));
			pc += 2;
			t_states (12);
			break;
		case 0x32:                /* LD HL-, A; T-states: 8 */
			pc++;
			*(io + cpu.regs16.HL) = cpu.regs8.A;
			cpu.regs16.HL--;
			t_states (8);
			break;
		case 0x33:                /* INC SP; T-states: 8 */
			pc++;
			cpu.regs16.SP++;
			t_states (8);
			break;
		case 0x34:                /* INC HL; T-states: 12 */
			pc++;
			cpu.flags &= ~N;
			cpu.flags |= ((cpu.regs8.HL <= 0xf) && (cpu.regs8.HL + 1 > 0xf)) ? H: 0;
			cpu.flags |= ((cpu.regs8.HL + 1) == 0)? Z: 0;
			cpu.regs8.HL++;
			t_states (12);
			break;
		case 0x35:                /* DEC [HL]; T-states: 12 */
			{
				pc++;
				uint8_t *e8 = &io[cpu.regs16.HL];

				cpu.flags &= N;
				cpu.flags |= ((*e8 - 1) == 0) ? Z: 0;
				cpu.flags |= ((*e8 & (1 << 4)) ? H: 0;

				(*e8)--;
			}
			break;
		case 0x36:                /* LD [HL], n8; T-states: 12 */
			pc++;
			*(io + cpu.regs16.HL) = io[mem[pc++]];
			break;
		case 0x37:                /* SCF; T-states: 4 */
			pc++;
			cpu.flags &= ~N;
			cpu.flags &= ~H;
			cpu.flags |= C;
			break;
		case 0x38:               /* JR C, e8; T-states: 12/8 */
			pc++;
			if (cpu.flags & C)
				pc += (int8_t) io[mem[pc]];
			t_states (1);
			break;
		case 0x39:               /* ADD HL, SP; T-states: 8 */
			pc++;
			cpu.regs &= ~N;
			cpu.regs |= ((cpu.regs16.HL <= 0xfff) & ((cpu.regs16.HL + cpu.regs16.SP) > 0xfff)) ? H: 0;
			cpu.regs |= (cpu.regs16.HL < (cpu.regs16.HL + cpu.regs16.SP)) ? 0: C;
			cpu.regs16.HL += cpu.regs16.SP;
			t_states (8);
			break;
		case 0x3a:               /* LD A, [HL-]; T-states: 8 */
			pc++;
			cpu.regs8.A = io[cpu.regs16.HL];
			cpu.regs16.HL--;
			break;
		case 0x3b:               /* DEC SP; T-states: 8 */
			pc++;
			cpu.regs16.SP--;
			break;
		case 0x3c:               /* INC A; T-states: 4 */
			pc++;
			cpu.flags &= ~N;
			cpu.flags |= ((cpu.regs8.A <= 0xf) && ((cpu.regs8.A + 1) > 0xf))? H: 0;
			cpu.flags |= ((cpu.regs8.A + 1) == 0) ? Z: 0;
			cpu.regs8.A++;
			t_states (4);
			break;
		case 0x3d:               /* DEC A; T-states: 4 */
			pc++;
			cpu.flags &= N;
			cpu.flags |= ((cpu.regs8.A  - 1) == 0) ? Z: 0;
			cpu.flags |= (cpu.regs8.A & (1 << 4)) ? H: 0;
			cpu.regs8.A--;
			t_states (4);
			break;
		case 0x3e:               /* LD A, n8; T-states: 8 */
			pc++;
			cpu.regs8.A = io[mem[pc++]];
			t_states (8);
			break;
		case 0x3f:               /* CCF; T-states: 4 */
			/*
			 * TODO: if this is not correct then it need to correct.
			 * Complement Carry Flag.
			 */
			pc++;
			cpu.flags &= ~N;
			cpu.flags &= ~H;
			if (cpu.flags & C)
				cpu.flags &= C;
			else
				cpu.flags |= C;
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
