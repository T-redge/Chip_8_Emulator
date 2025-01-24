#include "chip8.h"

//Loading rom into memory
bool load_rom(BYTE *memory)
{
	const char *file_name = "ch8/corax.ch8";
	bool success = true;
	
	FILE *file_open = fopen(file_name, "rb");
	if (!file_open) {
		perror("File opening failed\n");
		success = false;
	}
	
	size_t file_size = get_file_size(file_open);
	
	
	BYTE *buffer = malloc(file_size * sizeof(char));
	while (fread(buffer, sizeof(buffer[0]), file_size, file_open) != 0)
		;
	
	for (size_t i = 0; i < file_size; ++i)
		memory[512 + i] = buffer[i];
	
	free(buffer);
	fclose(file_open);
	return success;
}
long long get_file_size(FILE *file)
{
	fseek(file, sizeof(char), SEEK_END);
#ifdef WIN32
	long long tmp = _ftelli64(file);
#else 
	long long tmp = ftell(file);
#endif
	printf("Size of file is: %lld bytes\n", tmp);
	rewind(file);
	
	return tmp;
}
//Stack for chip8
void push(Stack *st, DUPLET new_address)
{
	if(st->top == STACK_SIZE) {
		printf("Stack Overflow\n");
	}
	st->mem_address[st->top++] = new_address;
	printf("Top of stack: %X\n", st->mem_address[st->top - 1]);
}
DUPLET pop(Stack *st)
{
	if (st->top == 0) {
		printf("Stack Underflow\n");
	}
	DUPLET tmp = st->mem_address[st->top - 1];
	printf("Address: %X\n", tmp);
	return tmp;
}
//chip8 instructions
DUPLET get_opcode(BYTE *memory, DUPLET *p_c)
{
	DUPLET tmp;
	tmp = memory[*p_c] << 8 | memory[*p_c + 1];
	
	*p_c += 2;
	
	return tmp;
}
void opcodeEO(BYTE display[][SCREEN_HEIGHT])
{
	for(int y = 0; y < 32; ++y)
		for (int x = 0; x < 64; ++x)
			display[x][y] = 0;
}
void opcodeANNN(DUPLET opcode, DUPLET *i_reg)
{
	DUPLET tmp;
	tmp = opcode & 0x0FFF;
	*i_reg = tmp;
}
void opcode6XNN(DUPLET opcode, BYTE *var_reg)
{
	DUPLET vx;
	BYTE tmp;
	vx = (opcode & 0x0F00) >> 8;
	tmp = opcode & 0x00FF;
	
	var_reg[vx] = tmp;
}
void opcodeDXYN(DUPLET opcode, BYTE *var_reg, DUPLET i_reg, BYTE display[][SCREEN_HEIGHT], BYTE *memory)
{
	DUPLET vx, vy, rows, coord_x, coord_y, sprite_data;
	vx = (opcode & 0x0F00) >> 8;
	vy = (opcode & 0x00F0) >> 4;
	rows = opcode & 0x000F;
	
	coord_x = var_reg[vx & 63];
	coord_y = var_reg[vy & 31];
	
	var_reg[15] = 0;
	
	printf("vx: %X, vy: %X, rows: %X, coord_x: %X, coord_y: %X\n", vx, vy, rows, coord_x, coord_y);

	for (int i = 0; i < rows; ++i) {
		sprite_data = memory[i_reg + i];
		printf("Sprite_data: %X\t", sprite_data);
		
		int xpixelinv = 7;
		int xpixel = 0;
		
		printf("\n");
		for (xpixel = 0; xpixel < 8; ++xpixel, --xpixelinv) {
			int mask = 1 << xpixelinv;
			
			printf("%X\t", mask);
			if (sprite_data & mask) {
				int x = coord_x + xpixel;
				int y = coord_y + i;
				if (display[x][y] == 1)
					var_reg[15] = 1;
				display[x][y] ^= 1;
			}
		}
		printf("\n");
	}
	for (int y = 0; y < 32; ++y) {
		for(int x = 0; x < 64; ++x) {
			printf("%d", display[x][y]);
		}
		printf("\n");
	}
	printf("\n");
}
void opcode7XNN(DUPLET opcode, BYTE *var_reg)
{
	DUPLET vx, tmp;
	vx = (opcode & 0x0F00) >> 8;
	tmp = opcode & 0x00FF;
	
	var_reg[vx] += tmp;
	
	printf("vx: %X, tmp: %X, var_reg[%X]: %X\n", vx, tmp, vx, var_reg[vx]);
}
void opcode1NNN(DUPLET opcode, DUPLET *p_c)
{
	DUPLET tmp;
	tmp = opcode & 0x0FFF;
	
	*p_c = tmp;
}
void opcode3XNN(DUPLET opcode, DUPLET *p_c, BYTE *var_reg)
{
	DUPLET vx = (opcode & 0x0F00) >> 8;
	DUPLET tmp = opcode & 0x00FF;
	
	if (var_reg[vx] == tmp) {
		printf("Skipping\n");
		*p_c += 2;
		
	}
	
	printf("vx: %X\n", vx);
	printf("tmp: %X\n", tmp);
	printf("var_reg[%X]: %X\n", vx, var_reg[vx]);
}
void opcode4XNN(DUPLET opcode, DUPLET *p_c, BYTE *var_reg)
{
	DUPLET vx = (opcode & 0x0F00) >> 8;
	DUPLET tmp = opcode & 0x00FF;
	
	if (var_reg[vx] != tmp) {
		printf("Skipping\n");
		*p_c += 2;
		
	}
	
	printf("vx: %X\n", vx);
	printf("tmp: %X\n", tmp);
	printf("var_reg[%X]: %X\n", vx, var_reg[vx]);
}
void opcode5XY0(DUPLET opcode, DUPLET *p_c, BYTE *var_reg)
{
	DUPLET vx = (opcode & 0x0F00) >> 8;
	DUPLET vy = (opcode & 0x00F0) >> 4;
	
	if (var_reg[vx] == var_reg[vy]) {
		printf("Skipping\n");
		*p_c += 2;
		
	}
	
	printf("vx: %X\n", vx);
	printf("vy: %X\n", vy);
	printf("var_reg[%X]: %X\n", vx, var_reg[vx]);
	printf("var_reg[%X]: %X\n", vy, var_reg[vy]);
}
void opcode9XY0(DUPLET opcode, DUPLET *p_c, BYTE *var_reg)
{
	DUPLET vx = (opcode & 0x0F00) >> 8;
	DUPLET vy = (opcode & 0x00F0) >> 4;
	
	if (var_reg[vx] != var_reg[vy]) {
		printf("Skipping\n");
		*p_c += 2;
	}
	
	printf("vx: %X\n", vx);
	printf("vy: %X\n", vy);
	printf("var_reg[%X]: %X\n", vx, var_reg[vx]);
	printf("var_reg[%X]: %X\n", vy, var_reg[vy]);
}
void opcode2NNN(DUPLET opcode, DUPLET *p_c,Stack *stack)
{
	DUPLET tmp = opcode & 0x0FFF;
	
	push(stack, *p_c);
	
	*p_c = tmp;
	printf("Jumping to: %X\n", tmp);
}
void opcodeEE(DUPLET *p_c, Stack *stack)
{
	*p_c = pop(stack);
}
void opcode8XY0(DUPLET opcode, BYTE *var_reg)
{
	DUPLET vx = (opcode & 0x0F00) >> 8;
	DUPLET vy = (opcode & 0x00F0) >> 4;
	
	var_reg[vx] = var_reg[vy];
}
void opcode8XY1(DUPLET opcode, BYTE *var_reg)
{
	DUPLET vx = (opcode & 0x0F00) >> 8;
	DUPLET vy = (opcode & 0x00F0) >> 4;
	
	printf("vx: %X, vy: %X\n", vx, vy);
	
	var_reg[vx] |= var_reg[vy];
	
	printf("var_reg[%X]: %X\n", vx, var_reg[vx]);
}
void opcode8XY2(DUPLET opcode, BYTE *var_reg)
{
	DUPLET vx = (opcode & 0x0F00) >> 8;
	DUPLET vy = (opcode & 0x00F0) >> 4;
	
	printf("vx: %X, vy: %X\n", vx, vy);
	
	var_reg[vx] &= var_reg[vy];
	
	printf("var_reg[%X]: %X\n", vx, var_reg[vx]);
}
void opcode8XY3(DUPLET opcode, BYTE *var_reg)
{
	DUPLET vx = (opcode & 0x0F00) >> 8;
	DUPLET vy = (opcode & 0x00F0) >> 4;
	
	printf("vx: %X, vy: %X\n", vx, vy);
	
	var_reg[vx] ^= var_reg[vy];
	
	printf("var_reg[%X]: %X\n", vx, var_reg[vx]);
}
void opcode8XY4(DUPLET opcode, BYTE *var_reg)
{
	BYTE vx = (opcode & 0x0F00) >> 8;
	BYTE vy = (opcode & 0x00F0) >> 4;
	
	printf("vx: %X, vy: %X\n", vx, vy);
	
	DUPLET tmp = var_reg[vx] + var_reg[vy];
	if (tmp > 255)
		var_reg[16] = 1;
	else
		var_reg[16] = 0;
	
	var_reg[vx] += var_reg[vy];
	
	printf("var_reg[%X]: %X\n", vx, var_reg[vx]);
}
void opcode8XY5(DUPLET opcode, BYTE *var_reg)
{
	BYTE vx = (opcode & 0x0F00) >> 8;
	BYTE vy = (opcode & 0x00F0) >> 4;
	
	printf("vx: %X, vy: %X\n", vx, vy);
	
	if (var_reg[vx] >= var_reg[vy])
		var_reg[16] = 1;
	else
		var_reg[16] = 0;
	
	var_reg[vx] -= var_reg[vy];
	printf("var_reg[%X]: %X\n", vx, var_reg[vx]);
}
void opcode8XY6(DUPLET opcode, BYTE *var_reg)
{
	BYTE vx = (opcode & 0x0F00) >> 8;
	
	var_reg[0xF] = var_reg[vx] & 0x1;
	
	printf("%X\n",var_reg[0xF]);
	
	var_reg[vx] >>= 1;
	
	
	
}
void opcode8XY7(DUPLET opcode, BYTE *var_reg)
{
	BYTE vx = (opcode & 0x0F00) >> 8;
	BYTE vy = (opcode & 0x00F0) >> 4;
	
	printf("vx: %X, vy: %X\n", vx, vy);
	
	if (var_reg[vy] >= var_reg[vx])
		var_reg[16] = 1;
	else
		var_reg[16] = 0;
	
	var_reg[vx] = var_reg[vy] - var_reg[vx];
	printf("var_reg[%X]: %X\n", vx, var_reg[vx]);
}
void opcode8XYE(DUPLET opcode, BYTE *var_reg)
{
	BYTE vx = (opcode & 0x0F00) >> 8;
	
	var_reg[0xF] = var_reg[vx] & 0x1;
	
	printf("%X\n",var_reg[0xF]);
	
	var_reg[vx] <<= 1;
}
void opcodeFX65(DUPLET opcode, BYTE *var_reg, DUPLET i_reg, BYTE *memory)
{
	DUPLET vx = (opcode & 0x0F00) >> 8;
	
	DUPLET tmp = i_reg;
	
	for (DUPLET x = 0; x <= vx; ++x) {
		var_reg[x] = memory[tmp];
		printf("var_reg[%d]: %X\t", x, tmp);
		++tmp;
		printf("i_reg: %X\n", tmp);
	}
	printf("\n");
}
void opcodeFX55(DUPLET opcode, BYTE *var_reg, DUPLET i_reg, BYTE *memory)
{
	DUPLET vx = (opcode & 0x0F00) >> 8;
	
	DUPLET tmp = i_reg;
	
	for (DUPLET x = 0; x <= vx; ++x) {
		memory[tmp] = var_reg[x];
		printf("var_reg[%d]: %X\t", x, tmp);
		++tmp;
		printf("i_reg: %X\n", tmp);
	}
	printf("\n");
}
void opcodeFX33(DUPLET opcode, BYTE *memory, DUPLET i_reg, BYTE *var_reg)
{
	BYTE vx = (opcode & 0x0F00) >> 8;
	printf("var_reg[%X]: %d\n", vx, var_reg[vx]);
	
	
	BYTE tmp_one = var_reg[vx] / (10 * 10);
	BYTE tmp_two = (var_reg[vx] % (10 * 10)) / 10;
	BYTE tmp_three = (var_reg[vx] % 10) / 1;
	printf("Tmp_one: %d\n", tmp_one);
	printf("Tmp_two: %d\n", tmp_two);
	printf("Tmp_three: %d\n", tmp_three);
	
	memory[i_reg]	= tmp_one;
	memory[i_reg+1] = tmp_two;
	memory[i_reg+2] = tmp_three;
}
void opcodeFX1E(DUPLET opcode, BYTE *var_reg, DUPLET *i_reg)
{
	BYTE vx = (opcode & 0x0F00) >> 8;
	printf("var_reg[%X]: %X\n", vx, var_reg[vx]);
	printf("I_reg: %X\n", *i_reg);
	
	*i_reg += var_reg[vx];
	
}