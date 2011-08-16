#include <stdio.h>
#include "ppu.h"
#include "mem.h"

#define get_byte() get_byte_at(c.pc+1)
#define get_short() get_short_at(c.pc+1)

struct CPU {
	unsigned int pc, sp, a, x, y;
	unsigned int z, n, v, c;
	unsigned int i, d;
	unsigned int cycles;
};

static struct CPU c;

void init_cpu(void)
{
	c.pc = get_short_at(0xFFFC);
	c.sp = 0x1FD;
	c.x = 0;
	c.a = 0;
	c.y = 0;
	c.z = 0;
	c.n = 0;
	c.v = 0;
	c.c = 0;
	c.i = 0;
	c.d = 0;
}

void cpu_nmi(unsigned int addr)
{
	c.pc = get_short_at(addr);
}

void cpu_cycle(void)
{
	unsigned int opcode = get_byte_at(c.pc);
	unsigned int t, t2;
	signed char s;
	printf("PC: %04X, SP: %04X\n", c.pc, c.sp);

	switch(opcode)
	{
		case 0x05:  /* ORA mem8 */
			t = get_byte();
			c.a |= get_byte_at(t);
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 2;
			c.cycles += 2;
		break;
		case 0x09:  /* ORA imm8 */
			t = get_byte();
			c.a |= t;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 2;
			c.cycles += 2;
		break;
		case 0x0A:  /* ASL A */
			c.c = !!(c.a & 0x80);
			c.a = (c.a << 1) & 0xFF;
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.pc++;
			c.cycles += 2;
		break;
		case 0x10:  /* BPL imm16 */
			if(c.n == 0) {
				s = get_byte();
				c.pc += s + 2;
				printf("Branched to %04X\n", c.pc);
				c.cycles += 3;
			} else {
				c.cycles += 2;
				c.pc += 2;
			}
		break;
		case 0x18:  /* CLC */
			c.c = 0;
			c.pc++;
			c.cycles += 2;
		break;
		case 0x1D:  /* ORA mem16, x*/
			t = get_short();
			c.a ^= get_byte_at(t + c.x);
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.cycles += 4;
			c.pc += 3;
			/* TODO: Page crossing cycle */
		break;
		case 0x20:  /* JSR */
			t = c.pc;
			writeb(c.sp, ((t+2) & 0xFF00) >> 8);
			writeb(c.sp-1, ((t+2) & 0xFF));
			c.pc = get_short();
			c.sp -= 2;
			c.cycles += 6;
		break;
		case 0x26:  /* ROL mem8 */
			t = get_byte();
			t = get_byte_at(t);
			t2 = t;
			t = (t<<7) | c.c;
			c.c = !!(t2 & 0x80);
			c.pc += 2;
			c.cycles += 5;
		break;
		case 0x29:  /* AND imm8 */
			t = get_byte();
			c.a &= t;
			c.pc += 2;
			c.cycles += 2;
		break;
		case 0x2C:  /* BIT m16 */
			t = get_short();
			t = get_byte_at(t);
			c.z = !!(t & c.a);
			c.n = !!(t&0x80);
			c.v = !!(t&0x40);
			c.cycles += 4;
			c.pc += 3;
		break;
		case 0x38:  /* SEC */
			c.c = 1;
			c.pc++;
			c.cycles += 2;
		break;
		case 0x4A:  /* LSR A */
			c.c = c.a & 1;
			c.a = c.a >> 1;
			c.n = 0;
			c.z = !c.a;
			c.cycles += 2;
			c.pc++;
		break;
		case 0x4C:  /* JMP */
			c.pc = get_short();
			c.cycles += 3;
		break;
		case 0x60:  /* RTS */
			printf("SP: %04X, %02X %02X %02X %02X\n", c.sp, get_byte_at(c.sp-1),
                get_byte_at(c.sp),get_byte_at(c.sp+1),get_byte_at(c.sp+2));
			printf("Read %04X for rts\n", get_short_at(c.sp+1));
			c.pc = get_short_at(c.sp+1);
			c.pc += 1;
			printf("RTS to %04X\n", c.pc);
			c.sp += 2;
			c.cycles += 6;
		break;
		case 0x68:  /* PLA */
			c.a = get_byte_at(c.sp+1);
//			c.sp++;
			c.cycles += 4;
			c.pc++;
		break;
		case 0x69:  /* ADC imm8 */
			s = get_byte();
			c.a += s + c.c;
			/* TODO: Fix overflow flag */
//			c.v = a > 128 ? 1 : 0;
			c.c = !!(c.a & 0x100);
			c.a &= 0xFF;
			c.n = !(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 2;
			c.cycles += 2;
		break;
		case 0x75:  /* ADC mem8, x */
			t = get_byte();
			s = get_byte_at((t + c.x) & 0xFF);
			c.a += s + c.c;
			/* TODO: Fix overflow flag */
//			c.v = a > 128 ? 1 : 0;
			c.c = !!(c.a & 0x100);
			c.a &= 0xFF;
			c.n = !(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 2;
			c.cycles += 2;
		break;
		case 0x78:  /* SEI - Disable interrupts */
			printf("Interrupts disabled, great, less work.\n");
			c.i = 1;
			c.cycles += 2;
			c.pc++;
		break;
		case 0x85:  /* STA m8 */
			t = get_byte();
			writeb(t, c.a);
			c.pc += 2;
			c.cycles += 3;
		break;
		case 0x8D:  /* STA m16 */
			t = get_short();
			writeb(t, c.a);
			c.cycles += 4;
			c.pc += 3;
		break;
		case 0x8E:  /* STX imm16 */
			t = get_short();
			writeb(t, c.a);
			c.cycles += 4;
			c.pc += 3;
		break;
		case 0x90:  /* BCC rel8 */
			if(!c.c) {
				s = get_byte();
				c.pc += s + 2;
				printf("Branched to %04X\n", c.pc);
				c.cycles += 3;
			} else {
				c.cycles += 2;
				c.pc += 2;
			}
		break;
		case 0x95:  /* STA imm8, X */
			t = get_byte();
			writeb((t+c.x)&0xFF, c.a);
			c.cycles += 4;
			c.pc += 2;
		break;
		case 0x98:  /* TYA */
			c.y = c.a;
			c.n = !!(c.y & 0x80);
			c.z = !(c.y);
			c.pc++;
			c.cycles += 2;
		break;
		case 0x9A:  /* TXS */
			c.sp = 0x100 + c.x;
			c.pc++;
			c.cycles += 2;
		break;
		case 0x9D:  /* STA mem16, x */
			t = get_short() + c.x;
			writeb(t, c.a);
			c.cycles += 5;
			c.pc += 3;
		break;
		case 0xA0:  /* LDY imm8 */
			c.y = get_byte();
			c.z = !c.y;
			c.n = !!(c.y & 0x80);
			c.cycles += 2;
			c.pc += 2;
		break;
		case 0xA2:  /* LDX imm8 */
			c.x = get_byte();
			c.z = !c.x;
			c.n = !!(c.x & 0x80);
			c.cycles += 2;
			c.pc += 2;
		break;
		case 0xA5:  /* LDA mem8 */
			t = get_byte();
			c.a = get_byte_at(t);
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.cycles += 2;
			c.pc += 2;
		break;
		case 0xA6:  /* LDX mem8 */
			t = get_byte();
			c.x = get_byte_at(t);
			c.z = !c.x;
			c.n = !!(c.x & 0x80);
			c.pc += 2;
			c.cycles += 3;
		break;
		case 0xA8:  /* TAY */
			c.y = c.a;
			c.n = !!(c.y & 0x80);
			c.z = !(c.y);
			c.pc++;
			c.cycles += 2;
		break;
		case 0xA9:  /* LDA imm8 */
			c.a = get_byte();
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.cycles += 2;
			c.pc += 2;
		break;
		case 0xAC:  /* LDY mem16 */
			t = get_short();
			c.y = get_byte_at(t);
			c.z = !c.y;
			c.n = !!(c.y & 0x80);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0xAD:  /* LDA mem16 */
			t = get_short();
			c.a = get_byte_at(t);
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0xAA:  /* TAX */
			c.x = c.a;
			c.n = !!(c.x & 0x80);
			c.z = !(c.x);
			c.pc++;
			c.cycles += 2;
		break;
		case 0xB0:  /* BCS */
			if(c.c) {
				s = get_byte();
				c.pc += s + 2;
				printf("Branched to %04X\n", c.pc);
				c.cycles += 3;
			} else {
				c.cycles += 2;
				c.pc += 2;
			}
		break;
		case 0xB1:  /* LDA (imm8), y */
			t = get_byte();
			t = get_short_at(t);
			t += c.y;
			c.a = get_byte_at(t);
			c.pc += 2;
			c.cycles += 5; /* TODO: Page crossing cycle */
		break;
		case 0xB5:  /* LDA mem8, x */
			t = get_byte();
			c.a = get_byte_at((t + c.x)&0xFF);
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.cycles += 4;
			c.pc += 2;
		break;
		case 0xBD:  /* LDA mem16, x */
			t = get_short() + c.x;
			c.a = get_byte_at(t);
			c.cycles += 3;
			if(c.pc & 0xFF00 != t & 0xFF00)
				c.cycles++;
			c.pc += 3;
		break;
		case 0xC0:  /* CPY imm8 */
			t = get_byte();
			c.c = c.y > t ? 1 : 0;
			c.z = c.y == t;
			c.n = !!((c.y - t)&0x80);
			c.cycles += 2;
			c.pc += 2;
		break;
		case 0xC8:  /* INY */
			c.y++;
			c.y = c.y & 0xFF;
			c.z = !c.y;
			c.n = !!(c.y & 0x80);
			c.pc++;
			c.cycles += 2;
		break;
		case 0xC9:  /* CMP imm8 */
			t = get_byte();
			c.c = c.a > t ? 1 : 0;
			c.z = c.a == t;
			c.n = !!((c.a - t)&0x80);
			c.cycles += 2;
			c.pc += 2;
		break;
		case 0xCA:  /* DEX */
			c.x--;
			c.x &= 0xFF;
			c.z = !c.x;
			c.n = !!(c.x & 0x80);
			c.cycles += 2;
			c.pc++;
		break;
		case 0xD0: /* BNE */
			if(c.z == 0) {
				s = get_byte();
				c.pc += s + 2;
				printf("Branched to %04X\n", c.pc);
				c.cycles += 3;
			} else {
				c.cycles += 2;
				c.pc += 2;
			}
		break;
		case 0xD8:
			printf("Clear decimal mode.\n");
			c.d = 0;
			c.cycles += 2;
			c.pc += 1;
		break;
		case 0xE0:  /* CPX imm8 */
			t = get_byte();
			c.c = c.x > t ? 1 : 0;
			c.z = c.x == t;
			c.n = !!((c.x - t)&0x80);
			c.cycles += 2;
			c.pc += 2;
		break;
		case 0xE6:  /* INC mem8 */
			t = get_byte();
			writeb(t, get_byte_at(t));
			c.pc += 2;
			c.cycles += 5;
		break;
		case 0xE8:  /* INX */
			c.x += 1;
			if(c.x > 0xFF)
				c.x = 0x0;
			c.z = !c.x;
			c.n = !!(c.x & 0x80);
			c.pc += 1;
			c.cycles += 2;
		break;
		case 0xE9:  /* SBC imm8 */
			s = get_byte();
			c.a -= s + !c.c;
			c.a &= 0xFF;
			c.z = !c.a;
			c.pc += 2;
			c.cycles += 2;
		break;
		case 0xF0:  /* BEQ */
			if(c.z){
				s = get_byte();
				c.pc += s + 2;
				c.cycles += (t & 0xFF00) == (c.pc & 0xFF00) ? 1 : 2;
				printf("BEQ to %04X\n", c.pc);
				break;
			}
			c.pc += 2;
			c.cycles += 2;
		break;
		default:
			defaul:
			printf("A: %02X, X: %02X, Y: %02X\n", c.a, c.x, c.y);
			printf("Z: %1u, N: %1u, V: %1u, C: %1u\n", c.z, c.n, c.v, c.c);
			printf("I: %1u, D: %1u, SP: %04X\n", c.i, c.d, c.sp);
           	printf("Unhandled opcode: %02X at %04X\n", opcode, c.pc);
			for(t = c.pc; t < c.pc+16; t++)
				printf("%02X ", get_byte_at(t));
			printf("\n");
			exit(0);
		break;
	}
}

unsigned int cpu_getcycles(void)
{
	return c.cycles;
}
