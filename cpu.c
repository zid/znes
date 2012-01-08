#include <stdio.h>
#include <stdlib.h>

#define get_byte() readb(c.pc+1)
#define get_byte_at(x) readb(x)
#define get_short() (readb(c.pc+1) | (readb(c.pc+2)<<8))
#define get_short_at(x) (readb(x) | ( readb( ((x)+1))<<8 ) )
#define get_zeropage_short_at(x) (get_byte_at(((x)+1)&0xFF) << 8 | get_byte_at((x)&0xFF))

struct CPU {
	unsigned int pc, sp, a, x, y;
	unsigned int z, n, v, c;
	unsigned int i, d;
	unsigned int cycles;
};

static struct CPU c;

static void (*writeb)(unsigned int, unsigned char);
static unsigned char (*readb)(unsigned int);

void cpu_set_writeb(void (*wbfp)(unsigned int, unsigned char))
{
	writeb = wbfp;
}

void cpu_set_readb(unsigned char (*rbfp)(unsigned int))
{
	readb = rbfp;
}


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

unsigned int cpu_getcycles(void)
{
	return c.cycles;
}

static void push_flags(void)
{
	unsigned int flags;

	flags = (c.n<<7) | (c.v<<6)| (1<<5) | (c.d<<3) | (c.i<<2) | (c.z<<1) | (c.c);
	writeb(c.sp, flags);
	c.sp--;
}
static void pull_flags(void)
{
	unsigned int t;
	t = get_byte_at(c.sp+1);

	c.n = !!(t & 0x80);
	c.v = !!(t & 0x40);
	c.d = !!(t & 0x08);
	c.i = !!(t & 0x04);
	c.z = !!(t & 0x02);
	c.c = !!(t & 0x01);
	c.sp += 1;
}

void cpu_nmi(unsigned int addr)
{
	writeb(c.sp, (c.pc & 0xFF00)>>8);
	writeb(c.sp-1, c.pc & 0xFF);
	c.sp -= 2;
	push_flags();
	c.pc = get_short_at(addr);
}

void cpu_cycle(void)
{
	unsigned int opcode = get_byte_at(c.pc);
	unsigned int t, t2;
	signed char s;
//	printf("PC: %04X, SP: %04X\n", c.pc, c.sp);
/*	printf("PC:%04X A:%02X X:%02X Y:%02X S:%02X P:%c%cub%c%c%c ", c.pc, c.a, c.x, c.y, c.sp&0xFF,
		c.n ? 'N' : 'n',
		c.v ? 'V' : 'v',
		c.i ? 'I' : 'i',
		c.z ? 'Z' : 'z',
		c.c ? 'C' : 'c'
	);
	printf("S: %02X %02X %02X\n", get_byte_at(c.sp+1), get_byte_at(c.sp+2), get_byte_at(c.sp+3));
*/

	switch(opcode)
	{
		case 0x01:  /* ORA (mem8, X) */
			t = get_byte();
			t2 = get_zeropage_short_at(t + c.x);
			c.a |= get_byte_at(t2);
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0x05:  /* ORA mem8 */
			t = get_byte();
			c.a |= get_byte_at(t);
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 2;
			c.cycles += 2;
		break;
		case 0x06:  /* ASL mem8 */
			t = get_byte();
			t2 = get_byte_at(t);
			c.c = !!(t2 & 0x80);
			t2 = (t2 << 1) & 0xFF;
			writeb(t, t2);
			c.z = !t2;
			c.n = !!(t2 & 0x80);
			c.pc += 2;
			c.cycles += 5;
		break;
		case 0x08:  /* PHP */
			push_flags();
			c.pc += 1;
			c.cycles += 3;
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
		case 0x0D:  /* ORA mem16 */
			t = get_short();
			t2 = get_byte_at(t);
			c.a |= t2;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0x0E:  /* ASL mem16 */
			t = get_short();
			t2 = get_byte_at(t);
			c.c = !!(t2 & 0x80);
			t2 = (t2 << 1) & 0xFF;
			writeb(t, t2);
			c.z = !t2;
			c.n = !!(t2 & 0x80);
			c.pc += 3;
			c.cycles += 6;
		break;
		case 0x10:  /* BPL imm16 */
			if(c.n == 0) {
				s = get_byte();
				c.pc += s + 2;
//				printf("Branched to %04X\n", c.pc);
				c.cycles += 3;
			} else {
				c.cycles += 2;
				c.pc += 2;
			}
		break;
		case 0x11:  /* ORA (mem8), Y */
			t = get_byte();
			t2 = get_zeropage_short_at(t);
			t2 = get_byte_at((t2 + c.y) & 0xFFFF);
			c.a |= t2;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 2;
			c.cycles += 5;
		break;
		case 0x15:  /* ORA mem8, X */
			t = get_byte();
			t2 = get_byte_at((t + c.x) & 0xFF);
			c.a |= t2;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 2;
			c.cycles += 4;
		break;
		case 0x16:  /* LSR mem8, X */
			t = get_byte();
			t2 = get_byte_at((t + c.x) & 0xFF);
			c.c = !!(t2 & 0x80);
			t2 = t2 << 1;
			writeb((t + c.x) & 0xFF, t2);
			c.n = 0;
			c.z = !t2;
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0x18:  /* CLC */
			c.c = 0;
			c.pc++;
			c.cycles += 2;
		break;
		case 0x19:  /* ORA mem16, y */
			t = get_short();
			t2 = get_byte_at((t + c.y) & 0xFFFF);
			c.a |= t2;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.cycles += 4;
			c.pc += 3;
		break;
		case 0x1D:  /* ORA mem16, x */
			t = get_short();
			c.a |= get_byte_at(t + c.x);
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0x1E:  /* ASL mem16, X */
			t = get_short();
			t2 = get_byte_at((t + c.x) & 0xFFFF);
			c.c = !!(t2 & 0x80);
			t2 = (t2 << 1) & 0xFF;
			writeb((t + c.x) & 0xFFFF, t2);
			c.z = !t2;
			c.n = !!(t2 & 0x80);
			c.pc += 3;
			c.cycles += 6;
		break;
		case 0x20:  /* JSR */
			t = c.pc;
			writeb(c.sp, ((t+2) & 0xFF00) >> 8);
			writeb(c.sp-1, ((t+2) & 0xFF));
			c.pc = get_short();
			c.sp -= 2;
			c.cycles += 6;
		break;
		case 0x21:  /* AND (mem8, X) */
			t = get_byte();
			t2 = get_zeropage_short_at(t + c.x);
			c.a &= get_byte_at(t2);
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0x24:  /* BIT mem8 */
			t = get_byte();
			t = get_byte_at(t);
			c.z = !(t & c.a);
			c.n = !!(t&0x80);
			c.v = !!(t&0x40);
			c.pc += 2;
			c.cycles += 3;
		break;
		case 0x25:  /* AND mem8 */
			t = get_byte();
			c.a &= get_byte_at(t);
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.pc += 2;
			c.cycles += 3;
		break;
		case 0x26:  /* ROL mem8 */
			t = get_byte();
			t2 = get_byte_at(t);
			c.z = c.c; /* Temporary, c.z will be recalculated */
			c.c = !!(t2 & 0x80);
			t2 = (t2 << 1) | c.z;
			writeb(t, t2);
			c.n = !!(t2 & 0x80);
			c.z = !t2;
			c.pc += 2;
			c.cycles += 5;
		break;
		case 0x28:  /* PLP */
			pull_flags();
			c.pc += 1;
			c.cycles += 3;
		break;
		case 0x29:  /* AND imm8 */
			t = get_byte();
			c.a &= t;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 2;
			c.cycles += 2;
		break;
		case 0x2A:  /* ROL */
			t = c.c;
			c.c = !!(c.a & 0x80);
			c.a = ((c.a << 1) | t) & 0xFF;
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 1;
			c.cycles += 2;
		break;
		case 0x2C:  /* BIT m16 */
			t = get_short();
			t = get_byte_at(t);
			c.z = !(t & c.a);
			c.n = !!(t&0x80);
			c.v = !!(t&0x40);
			c.cycles += 4;
			c.pc += 3;
		break;
		case 0x2D:  /* AND mem16 */
			t = get_short();
			t2 = get_byte_at(t);
			c.a &= t2;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0x2E:  /* ROL mem16 */
			t = get_short();
			t2 = get_byte_at(t);
			c.z = c.c; /* Temporary, c.z will be recalculated */
			c.c = !!(t2 & 0x80);
			t2 = (t2 << 1) | c.z;
			writeb(t, t2);
			c.n = !!(t2 & 0x80);
			c.z = !t2;
			c.pc += 3;
			c.cycles += 6;
		break;
		case 0x30:  /* BMI rel8 */
			if(c.n) {
				s = get_byte();
				c.pc += s + 2;
//				printf("Branched to %04X\n", c.pc);
				c.cycles += 3;
			} else {
				c.cycles += 2;
				c.pc += 2;
			}
		break;
		case 0x31:  /* AND (mem8), Y */
			t = get_byte();
			t2 = get_zeropage_short_at(t);
			t2 = get_byte_at((t2 + c.y) & 0xFFFF);
			c.a &= t2;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 2;
			c.cycles += 5;
		break;
		case 0x35:  /* AND mem8, X */
			t = get_byte();
			t2 = get_byte_at((t + c.x) & 0xFF);
			c.a &= t2;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 2;
			c.cycles += 4;
		break;
		case 0x36:  /* ROL mem8, X */
			t = get_byte();
			t2 = get_byte_at((t + c.x) & 0xFF);
			c.z = c.c; /* Temporary, c.z will be recalculated */
			c.c = !!(t2 & 0x80);
			t2 = (t2 << 1) | c.z;
			writeb((t + c.x) & 0xFF, t2);
			c.n = !!(t2 & 0x80);
			c.z = !t2;
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0x38:  /* SEC */
			c.c = 1;
			c.pc++;
			c.cycles += 2;
		break;
		case 0x39:  /* AND mem16, y */
			t = get_short();
			t2 = get_byte_at((t + c.y) & 0xFFFF);
			c.a &= t2;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.cycles += 4;
			c.pc += 3;
		break;
		case 0x3D:  /* AND mem16, x */
			t = get_short();
			c.a &= get_byte_at(t + c.x);
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0x3E:  /* ROL mem16, X */
			t = get_short();
			t2 = get_byte_at((t + c.x) & 0xFFFF);
			c.z = c.c; /* Temporary, c.z will be recalculated */
			c.c = !!(t2 & 0x80);
			t2 = (t2 << 1) | c.z;
			writeb((t + c.x) & 0xFFFF, t2);
			c.n = !!(t2 & 0x80);
			c.z = !t2;
			c.pc += 3;
			c.cycles += 6;
		break;
		case 0x40:  /* RTI */
			pull_flags();
			c.pc = get_short_at(c.sp+1);
			c.sp += 2;
			c.cycles += 6;
		break;
		case 0x41:  /* EOR (mem8, X */
            t = get_byte();
			t2 = get_zeropage_short_at(t + c.x);
			c.a ^= get_byte_at(t2);
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0x45:  /* EOR mem8 */
			t = get_byte();
			c.a ^= get_byte_at(t);
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.pc += 2;
			c.cycles += 3;
		break;
		case 0x46:  /* LSR mem8 */
			t = get_byte();
			t2 = get_byte_at(t);
			c.c = t2 & 1;
			t2 = t2 >> 1;
			writeb(t, t2);
			c.n = 0;
			c.z = !t2;
			c.pc += 2;
			c.cycles += 5;
		break;
		case 0x48:  /* PHA */
			writeb(c.sp, c.a);
			c.sp--;
			c.pc += 1;
			c.cycles += 3;
		break;
		case 0x49:  /* EOR imm8 */
			t = get_byte();
			c.a ^= t;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 2;
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
		case 0x4D:  /* EOR mem16 */
			t = get_short();
			t2 = get_short_at(t);
			c.a ^= t2;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0x4E:  /* LSR mem16 */
			t = get_short();
			t2 = get_byte_at(t);
			c.c = t2 & 1;
			t2 = t2 >> 1;
			writeb(t, t2);
			c.n = 0;
			c.z = !t2;
			c.pc += 3;
			c.cycles += 6;
		break;
		case 0x50:  /* BVC s8 */
			if(!c.v) {
				s = get_byte();
				c.pc += s + 2;
				c.cycles += 3;
			} else {
				c.cycles += 2;
				c.pc += 2;
			}
		break;
		case 0x51:  /* EOR (mem8), Y */
			t = get_byte();
			t2 = get_zeropage_short_at(t);
			t2 = get_byte_at((t2 + c.y) & 0xFFFF);
			c.a ^= t2;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 2;
			c.cycles += 5;
		break;
		case 0x55:  /* EOR mem8, X */
			t = get_byte();
			t2 = get_byte_at((t + c.x) & 0xFF);
			c.a ^= t2;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 2;
			c.cycles += 4;
		break;
		case 0x56:  /* LSR mem8, X */
			t = get_byte();
			t2 = get_byte_at((t + c.x) & 0xFF);
			c.c = t2 & 1;
			t2 = t2 >> 1;
			writeb((t + c.x) & 0xFF, t2);
			c.n = 0;
			c.z = !t2;
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0x58:  /* CLI */
			c.i = 0;
			c.cycles += 2;
			c.pc++;
		break;
		case 0x59:  /* EOR mem16, y */
			t = get_short();
			t2 = get_byte_at((t + c.y) & 0xFFFF);
			c.a ^= t2;
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.cycles += 4;
			c.pc += 3;
		break;
		case 0x5D:  /* EOR mem16, x */
			t = get_short();
			c.a ^= get_byte_at(t + c.x);
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0x5E:  /* LSR mem16, X */
			t = get_short();
			t2 = get_byte_at((t + c.x) & 0xFFFF);
			c.c = t2 & 1;
			t2 = t2 >> 1;
			writeb((t + c.x) & 0xFFFF, t2);
			c.n = 0;
			c.z = !t2;
			c.pc += 3;
			c.cycles += 6;
		break;
		case 0x60:  /* RTS */
			c.pc = get_short_at(c.sp+1);
			c.pc += 1;
			c.sp += 2;
			c.cycles += 6;
		break;
		case 0x61:  /* ADC (mem8, X) */
			t = get_byte();
			t = get_zeropage_short_at(t + c.x);
			t = get_byte_at(t);
			t2 = c.a;
			c.a += t + c.c;
			c.c = !!(c.a & 0x100);
			c.a &= 0xFF;
			c.v = !!(~(t2 ^ t) & (t2 ^ c.a) & 0x80);
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 2;
			c.cycles += 2;
		break;
		case 0x65:  /* ADC mem8 */
			t = get_byte();
			s = get_byte_at(t);
			t2 = get_byte_at(t);
			t = c.a;
			c.a += t2 + c.c;
			c.c = !!(c.a & 0x100);
			c.a &= 0xFF;
			c.v = !!(~(t ^ t2) & (t ^ c.a) & 0x80);
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 2;
			c.cycles += 3;
		break;
		case 0x66:  /* ROR mem8 */
			t = get_byte();
			t2 = get_byte_at(t);
			c.n = c.c;  /* Temporary variable */
			c.c = t2 & 1;
			t2 = (t2 >> 1) | (c.n<<7);
			writeb(t, t2);
			c.n = !!(t2 & 0x80);
			c.z = !t2;
			c.pc += 2;
			c.cycles += 5;
		break;
		case 0x68:  /* PLA */
			c.sp++;
			c.a = get_byte_at(c.sp);
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.cycles += 4;
			c.pc++;
		break;
		case 0x69:  /* ADC imm8 */
			t = get_byte();
			t2 = c.a;
			c.a += t + c.c;
			c.c = !!(c.a & 0x100);
			c.a &= 0xFF;
			c.v = !!(~(t2 ^ t) & (t2 ^ c.a) & 0x80);
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 2;
			c.cycles += 2;
		break;
		case 0x6A:  /* ROR */
			t = c.c;
			c.c = c.a & 1;
			c.a = (c.a >> 1) | (t<<7);
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 1;
			c.cycles += 2;
		break;
		case 0x6C:  /* JMP mem16 */
			t = get_short();
			if((t & 0xFF) == 0xFF){
				t2 = get_byte_at(t & 0xFF00);
			} else {
				t2 = get_byte_at(t+1);
			}
			t2 = (t2 << 8) | get_byte_at(t);
			c.pc = t2;
			c.cycles += 5;
		break;
		case 0x6D:  /* ADC mem16 */
			t = get_short();
			s = get_byte_at(t);
			t2 = get_byte_at(t);
			t = c.a;
			c.a += t2 + c.c;
			c.c = !!(c.a & 0x100);
			c.a &= 0xFF;
			c.v = !!(~(t ^ t2) & (t ^ c.a) & 0x80);
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 3;
			c.cycles += 3;
		break;
		case 0x6E:  /* ROR mem16 */
			t = get_short();
			t2 = get_byte_at(t);
			c.n = c.c;  /* Temporary variable */
			c.c = t2 & 1;
			t2 = (t2 >> 1) | (c.n<<7);
			writeb(t, t2);
			c.n = !!(t2 & 0x80);
			c.z = !t2;
			c.pc += 3;
			c.cycles += 6;
		break;
		case 0x70:  /* BVS s8 */
			if(c.v) {
				s = get_byte();
				c.pc += s + 2;
				c.cycles += 3;
			} else {
				c.cycles += 2;
				c.pc += 2;
			}
		break;
		case 0x71:  /* ADC (mem8), Y */
			t = get_byte();
			t2 = get_zeropage_short_at(t);
			t = get_byte_at((t2 + c.y) & 0xFFFF);
			t2 = c.a;
			c.a += t + c.c;
			c.c = !!(c.a & 0x100);
			c.a &= 0xFF;
			c.v = !!(~(t2 ^ t) & (t2 ^ c.a) & 0x80);
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 2;
			c.cycles += 5;
		break;
		case 0x75:  /* ADC mem8, X */
			t = get_byte();
			t = get_byte_at((t + c.x) & 0xFF);
			t2 = c.a;
			c.a += t + c.c;
			c.c = !!(c.a & 0x100);
			c.a &= 0xFF;
			c.v = !!(~(t2 ^ t) & (t2 ^ c.a) & 0x80);
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 2;
			c.cycles += 2;
		break;
		case 0x76:  /* ROR mem8, X */
			t = get_byte();
			t2 = get_byte_at((t + c.x) & 0xFF);
			c.n = c.c;  /* Temporary variable */
			c.c = t2 & 1;
			t2 = (t2 >> 1) | (c.n<<7);
			writeb((t + c.x) & 0xFF, t2);
			c.n = !!(t2 & 0x80);
			c.z = !t2;
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0x78:  /* SEI - Disable interrupts */
			c.i = 1;
			c.cycles += 2;
			c.pc++;
		break;
		case 0x79:  /* ADC mem16, Y */
			t = get_short();
			s = get_byte_at((t + c.y) & 0xFFFF);
			t2 = get_byte_at((t + c.y ) & 0xFFFF);
			t = c.a;
			c.a += t2 + c.c;
			c.c = !!(c.a & 0x100);
			c.a &= 0xFF;
			c.v = !!(~(t ^ t2) & (t ^ c.a) & 0x80);
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 3;
			c.cycles += 3;
		break;
		case 0x7D:  /* ADC mem16, X */
			t = get_short();
			s = get_byte_at((t + c.x) & 0xFFFF);
			t2 = get_byte_at((t + c.x) & 0xFFFF);
			t = c.a;
			c.a += t2 + c.c;
			c.c = !!(c.a & 0x100);
			c.a &= 0xFF;
			c.v = !!(~(t ^ t2) & (t ^ c.a) & 0x80);
			c.n = !!(c.a & 0x80);
			c.z = !(c.a);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0x7E:  /* ROR mem16, X */
			t = get_short();
			t2 = get_byte_at((t + c.x) & 0xFFFF);
			c.n = c.c;  /* Temporary variable */
			c.c = t2 & 1;
			t2 = (t2 >> 1) | (c.n<<7);
			writeb((t + c.x) & 0xFFFF, t2);
			c.n = !!(t2 & 0x80);
			c.z = !t2;
			c.pc += 3;
			c.cycles += 6;
		break;
		case 0x81:  /* STA (mem8, X) */
			t = get_byte();
			t2 = get_zeropage_short_at(t + c.x);
			writeb(t2, c.a);
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0x84:  /* STY mem8 */
			t = get_byte();
			writeb(t, c.y);
			c.pc += 2;
			c.cycles += 3;
		break;
		case 0x85:  /* STA mem8 */
			t = get_byte();
			writeb(t, c.a);
			c.pc += 2;
			c.cycles += 3;
		break;
		case 0x86:  /* STX mem8 */
			t = get_byte();
			writeb(t, c.x);
			c.pc += 2;
			c.cycles += 3;
		break;
		case 0x88:  /* DEY */
			c.y--;
			c.y &= 0xFF;
			c.z = !c.y;
			c.n = !!(c.y & 0x80);
			c.cycles += 2;
			c.pc++;
		break;
		case 0x8A:  /* TXA */
			c.a = c.x;
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.pc += 1;
			c.cycles += 2;
		break;
		case 0x8C:  /* STY m16 */
			t = get_short();
			writeb(t, c.y);
			c.cycles += 4;
			c.pc += 3;
		break;
		case 0x8D:  /* STA m16 */
			t = get_short();
			writeb(t, c.a);
			c.cycles += 4;
			c.pc += 3;
		break;
		case 0x8E:  /* STX imm16 */
			t = get_short();
			writeb(t, c.x);
			c.cycles += 4;
			c.pc += 3;
		break;
		case 0x90:  /* BCC rel8 */
			if(!c.c) {
				s = get_byte();
				c.pc += s + 2;
				c.cycles += 3;
			} else {
				c.cycles += 2;
				c.pc += 2;
			}
		break;
		case 0x91:  /* STA (mem8), Y */
			t = get_byte();
			t2 = get_zeropage_short_at(t);
			writeb((t2 + c.y)&0xFFFF, c.a);
			c.cycles += 6;
			c.pc += 2;
		break;
		case 0x94:  /* STY imm8, X */
			t = get_byte();
			writeb((t+c.x)&0xFF, c.y);
			c.cycles += 4;
			c.pc += 2;
		break;
		case 0x95:  /* STA imm8, X */
			t = get_byte();
			writeb((t+c.x)&0xFF, c.a);
			c.cycles += 4;
			c.pc += 2;
		break;
		case 0x96:  /* STX mem8, Y */
			t = get_byte();
			writeb((t + c.y) & 0xFF, c.x);
			c.pc += 2;
			c.cycles += 4;
		break;
		case 0x98:  /* TYA */
			c.a = c.y;
			c.n = !!(c.y & 0x80);
			c.z = !(c.y);
			c.pc++;
			c.cycles += 2;
		break;
		case 0x99:  /* STA mem16, y */
			t = get_short();
			writeb((t + c.y) & 0xFFFF, c.a);
			c.pc += 3;
			c.cycles += 5;
		break;
		case 0x9A:  /* TXS */
			c.sp = 0x100 + c.x;
			c.pc += 1;
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
		case 0xA1:  /* LDA (mem8, X) */
			t = get_byte();
			t2 = get_zeropage_short_at(t + c.x);
			c.a = get_byte_at(t2);
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0xA2:  /* LDX imm8 */
			c.x = get_byte();
			c.z = !c.x;
			c.n = !!(c.x & 0x80);
			c.cycles += 2;
			c.pc += 2;
		break;
		case 0xA4:  /* LDY mem8 */
			t = get_byte();
			c.y = get_byte_at(t);
			c.n = !!(c.y & 0x80);
			c.z = !(c.y);
			c.pc += 2;
			c.cycles += 3;
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
		case 0xAE:  /* LDX imm8 */
			t = get_short();
			c.x = get_byte_at(t);
			c.z = !c.x;
			c.n = !!(c.x & 0x80);
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
				c.cycles += 3;
			} else {
				c.cycles += 2;
				c.pc += 2;
			}
		break;
		case 0xB1:  /* LDA (mem8), y */
			t = get_byte();
			t2 = get_zeropage_short_at(t);
			c.a = get_byte_at((t2 + c.y)&0xFFFF);
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.pc += 2;
			c.cycles += 5; /* TODO: Page crossing cycle */
		break;
		case 0xB4:  /* LDY mem8, x */
			t = get_byte();
			c.y = get_byte_at((t + c.x)&0xFF);
			c.z = !c.y;
			c.n = !!(c.y & 0x80);
			c.cycles += 4;
			c.pc += 2;

		break;
		case 0xB5:  /* LDA mem8, x */
			t = get_byte();
			c.a = get_byte_at((t + c.x)&0xFF);
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.cycles += 4;
			c.pc += 2;
		break;
		case 0xB6:  /* LDX mem8, Y */
			t = get_byte();
			c.x = get_byte_at((t + c.y) & 0xFF);
			c.z = !c.x;
			c.n = !!(c.x & 0x80);
			c.pc += 2;
			c.cycles += 4;
		break;
		case 0xB8:  /* CLV */
			c.v = 0;
			c.pc += 1;
			c.cycles += 2;
		break;
		case 0xB9:  /* LDA mem16, y */
			t = get_short();
			c.a = get_byte_at((t + c.y) & 0xFFFF);
			c.n = !!(c.a & 0x80);
			c.z = !c.a;
			c.cycles += 4;
			c.pc += 3;
		break;
		case 0xBA:  /* TSX */
			c.x = c.sp & 0xFF;
			c.n = !!(c.x & 0x80);
			c.z = !c.x;
			c.cycles += 2;
			c.pc += 1;
		break;
		case 0xBC:  /* LDY mem16, y */
			t = get_short();
			c.y = get_byte_at((t + c.x) & 0xFFFF);
			c.n = !!(c.y & 0x80);
			c.z = !c.y;
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0xBD:  /* LDA mem16, X */
			t = get_short();
			c.a = get_byte_at((t + c.x) & 0xFFFF);
			c.z = !c.a;
			c.n = !!(c.a & 0x80);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0xBE:  /* LDX mem16, Y */
			t = get_short();
			c.x = get_byte_at((t + c.y) & 0xFFFF);
			c.z = !c.x;
			c.n = !!(c.x & 0x80);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0xC0:  /* CPY imm8 */
			t = get_byte();
			c.c = !!((c.y-t) < 0x100);
			c.z = !(c.y - t);
			c.n = !!((c.y - t)&0x80);
			c.cycles += 2;
			c.pc += 2;
		break;
		case 0xC1:  /* CMP (mem8, X) */
			t = get_byte();
			t = get_zeropage_short_at(t + c.x);
			t = get_byte_at(t);
			c.c = !!((c.a-t) < 0x100);
			c.z = c.a == t;
			c.n = !!((c.a - t)&0x80);
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0xC4:  /* CPY mem8 */
			t = get_byte();
			t = get_byte_at(t);
			c.c = !!((c.y-t) < 0x100);
			c.z = !(c.y - t);
			c.n = !!((c.y - t)&0x80);
			c.pc += 2;
			c.cycles += 3;
		break;
		case 0xC5:  /* CMP mem8 */
			t = get_byte();
			t = get_byte_at(t);
			c.c = !!((c.a-t) < 0x100);
			c.z = c.a == t;
			c.n = !!((c.a - t)&0x80);
			c.cycles += 3;
			c.pc += 2;
		break;
		case 0xC6:  /* DEC mem 8*/
			t = get_byte();
			t2 = get_byte_at(t);
			t2--;
			t2 &= 0xFF;
			writeb(t, t2);
			c.z = !t2;
			c.n = !!(t2 & 0x80);
			c.cycles += 5;
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
			c.c = !!((c.a-t) < 0x100);
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
		case 0xCC:  /* CPY mem16 */
			t = get_short();
			t = get_byte_at(t);
			c.c = !!((c.y-t) < 0x100);
			c.z = !(c.y - t);
			c.n = !!((c.y - t)&0x80);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0xCD:  /* CMP mem16 */
			t = get_short();
			t = get_byte_at(t);
			c.c = !!((c.a-t) < 0x100);
			c.z = c.a == t;
			c.n = !!((c.a - t)&0x80);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0xCE:  /* DEC mem16 */
			t = get_short();
			t2 = get_byte_at(t);
			t2--;
			t2 &= 0xFF;
			writeb(t, t2);
			c.z = !t2;
			c.n = !!(t2 & 0x80);
			c.pc += 3;
			c.cycles += 6;
		break;
		case 0xD0:  /* BNE */
			if(!c.z){
				s = get_byte();
				c.pc += s + 2;
				c.cycles += 3;
			} else {
				c.cycles += 2;
				c.pc += 2;
			}
		break;
		case 0xD1:  /* CMP (mem8), Y */
			t = get_byte();
			t2 = get_zeropage_short_at(t);
			t = get_byte_at((t2 + c.y) & 0xFFFF);
			c.c = !!((c.a-t) < 0x100);
			c.z = c.a == t;
			c.n = !!((c.a - t)&0x80);
			c.pc += 2;
			c.cycles += 5;
		break;
		case 0xD5:  /* CMP mem8, X */
			t = get_byte();
			t = get_byte_at((t + c.x) & 0xFF);
			c.c = !!((c.a-t) < 0x100);
			c.z = c.a == t;
			c.n = !!((c.a - t)&0x80);
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0xD6:  /* DEC mem8, x */
			t = get_byte();
			t2 = get_byte_at((t + c.x) & 0xFF) - 1;
			writeb((t + c.x) & 0xFF, t2);
			c.z = !t2;
			c.n = !!(t2 & 0x80);
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0xD8:  /* CLD */
			c.d = 0;
			c.cycles += 2;
			c.pc += 1;
		break;
		case 0xD9:  /* CMP mem16, Y */
			t = get_short();
			t = get_byte_at((t + c.y) & 0xFFFF);
			c.c = !!((c.a-t) < 0x100);
			c.z = c.a == t;
			c.n = !!((c.a - t)&0x80);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0xDD:  /* CMP mem16, X */
			t = get_short();
			t = get_byte_at((t + c.x) & 0xFFFF);
			c.c = !!((c.a-t) < 0x100);
			c.z = c.a == t;
			c.n = !!((c.a - t)&0x80);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0xDE:  /* DEC mem16, X */
			t = get_short();
			t2 = get_byte_at((t + c.x) & 0xFFFF);
			t2--;
			t2 &= 0xFF;
			writeb((t + c.x) & 0xFFFF, t2);
			c.z = !t2;
			c.n = !!(t2 & 0x80);
			c.pc += 3;
			c.cycles += 6;
		break;
		case 0xE0:  /* CPX imm8 */
			t = get_byte();
			c.c = !!((c.x-t) < 0x100);
			c.z = c.x == t;
			c.n = !!((c.x - t)&0x80);
			c.cycles += 2;
			c.pc += 2;
		break;
		case 0xE1:  /* SBC (mem8, X) */
			t = get_byte();
			t2 = get_zeropage_short_at(t + c.x);
			t = get_byte_at(t2);
			s = get_byte_at(t2);
			t2 = c.a;
			c.a -= s + !c.c;
			c.c = !(c.a > 0xFF);
			c.a &= 0xFF;
			c.z = !c.a;
			c.v = !!((t2 ^ t) & (t2 ^ c.a) & 0x80);
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0xE4:  /* CPX mem8 */
			t = get_byte();
			t = get_byte_at(t);
			c.c = !!((c.x-t) < 0x100);
			c.z = c.x == t;
			c.n = !!((c.x - t)&0x80);
			c.pc += 2;
			c.cycles += 3;
		break;
		case 0xE5:  /* SBC mem8 */
			t = get_byte();
			s = get_byte_at(t);
			t = get_byte_at(t);
			t2 = c.a;
			c.a -= s + !c.c;
			c.c = !(c.a > 0xFF);
			c.a &= 0xFF;
			c.z = !c.a;
			c.v = !!((t2 ^ t) & (t2 ^ c.a) & 0x80);
			c.pc += 2;
			c.cycles += 3;
		break;
		case 0xE6:  /* INC mem8 */
			t = get_byte();
			t2 = get_byte_at(t);
			t2 = (t2+1)&0xFF;
			writeb(t, t2);
			c.n = !!(t2 & 0x80);
			c.z = !t2;
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
			t = get_byte();
			t2 = c.a;
			c.a -= s + !c.c;
			c.c = !(c.a > 0xFF);
			c.a &= 0xFF;
			c.z = !c.a;
			c.v = !!((t2 ^ t) & (t2 ^ c.a) & 0x80);
			c.n = !!(c.a & 0x80);
			c.pc += 2;
			c.cycles += 2;
		break;
		case 0xEA:  /* NOP */
			c.pc += 1;
			c.cycles += 2;
		break;
		case 0xEC:  /* CPX mem16 */
			t = get_short();
			t = get_byte_at(t);
			c.c = !!((c.x-t) < 0x100);
			c.z = c.x == t;
			c.n = !!((c.x - t)&0x80);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0xED:  /* SBC mem16 */
			t = get_short();
			s = get_byte_at(t);
			t = get_byte_at(t);
			t2 = c.a;
			c.a -= s + !c.c;
			c.c = !(c.a > 0xFF);
			c.a &= 0xFF;
			c.z = !c.a;
			c.v = !!((t2 ^ t) & (t2 ^ c.a) & 0x80);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0xEE:  /* INC mem16 */
			t = get_short();
			t2 = get_byte_at(t);
			t2 = (t2+1)&0xFF;
			writeb(t, t2);
			c.n = !!(t2 & 0x80);
			c.z = !t2;
			c.pc += 3;
			c.cycles += 6;
		break;
		case 0xF0:  /* BEQ */
			if(c.z){
				s = get_byte();
				c.pc += s + 2;
				break;
			}
			c.pc += 2;
			c.cycles += 2;
		break;
		case 0xF1:  /* SBC (mem8), Y */
			t = get_byte();
			t2 = get_zeropage_short_at(t);
			t = get_byte_at((t2 + c.y) & 0xFFFF);
			s = get_byte_at((t2 + c.y) & 0xFFFF);
			t2 = c.a;
			c.a -= s + !c.c;
			c.c = !(c.a > 0xFF);
			c.a &= 0xFF;
			c.z = !c.a;
			c.v = !!((t2 ^ t) & (t2 ^ c.a) & 0x80);
			c.n = !!(c.a & 0x80);
			c.pc += 2;
			c.cycles += 5;
		break;
		case 0xF5:  /* SBC mem8, X */
			t2 = get_byte();
			t = get_byte_at((t2 + c.x) & 0xFF);
			s = get_byte_at((t2 + c.x) & 0xFF);
			t2 = c.a;
			c.a -= s + !c.c;
			c.c = !(c.a > 0xFF);
			c.a &= 0xFF;
			c.z = !c.a;
			c.v = !!((t2 ^ t) & (t2 ^ c.a) & 0x80);
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0xF6:  /* INC mem8, X */
			t = get_byte();
			t2 = get_byte_at((t + c.x) & 0xFF);
			t2 = (t2+1)&0xFF;
			writeb((t + c.x) & 0xFF, t2);
			c.n = !!(t2 & 0x80);
			c.z = !t2;
			c.pc += 2;
			c.cycles += 6;
		break;
		case 0xF8:  /* SED */
			c.d = 1;
			c.pc += 1;
			c.cycles += 2;
		break;
		case 0xF9:  /* SBC mem16, Y */
			t2 = get_short();
			s = get_byte_at((t2 + c.y) & 0xFFFF);
			t = get_byte_at((t2 + c.y) & 0xFFFF);
			t2 = c.a;
			c.a -= s + !c.c;
			c.c = !(c.a > 0xFF);
			c.a &= 0xFF;
			c.z = !c.a;
			c.v = !!((t2 ^ t) & (t2 ^ c.a) & 0x80);
			c.n = !!(c.a & 0x80);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0xFD:  /* SBC mem16, X */
			t = get_short();
			s = get_byte_at((t + c.x) & 0xFFFF);
			t = get_byte_at((t + c.x) & 0xFFFF);
			t2 = c.a;
			c.a -= s + !c.c;
			c.c = !(c.a > 0xFF);
			c.a &= 0xFF;
			c.z = !c.a;
			c.v = !!((t2 ^ t) & (t2 ^ c.a) & 0x80);
			c.pc += 3;
			c.cycles += 4;
		break;
		case 0xFE:  /* INC mem16, X */
			t = get_short();
			t2 = get_byte_at((t + c.x) & 0xFFFF);
			t2 = (t2+1)&0xFF;
			writeb((t + c.x) & 0xFFFF, t2);
			c.n = !!(t2 & 0x80);
			c.z = !t2;
			c.pc += 3;
			c.cycles += 6;
		break;
		default:
//			defaul:
			printf("A: %02X, X: %02X, Y: %02X\n", c.a, c.x, c.y);
			printf("Z: %1u, N: %1u, V: %1u, C: %1u\n", c.z, c.n, c.v, c.c);
			printf("I: %1u, D: %1u, SP: %04X\n", c.i, c.d, c.sp);
			printf("Unhandled opcode: %02X at %04X\n", opcode, c.pc);
			for(t = c.sp-4; t < c.sp+8; t++)
				printf("%02X ", get_byte_at(t));
			printf("\n");
			exit(0);
		break;
	}
}
