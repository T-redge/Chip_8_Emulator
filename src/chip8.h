#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef CHIP_8_INSTRUCTIONS_H
#define CHIP_8_INSTRUCTIONS_H

typedef unsigned char BYTE;
typedef unsigned short DUPLET;

extern int MEMORY_SIZE;
extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

#define STACK_SIZE 16

typedef struct Stack
{
	int top;
	DUPLET mem_address[STACK_SIZE];
	
	
}Stack;

void push(Stack *st, DUPLET new_address);									//Pushes address to top of stack
DUPLET pop(Stack *st);												//Returns address from subroutine

bool load_rom(BYTE *memory);											//loads rom into memory
long long get_file_size(FILE* file);										//Aquires file size for memory allocation

DUPLET get_opcode(BYTE *memory, DUPLET *p_c);									//Gets operator code from memory

void opcodeEO(BYTE display[][SCREEN_HEIGHT]);									//Sets Display to 0's
void opcodeEE(DUPLET *p_c, Stack *stack);									//Returns from address at top of stack
void opcode1NNN(DUPLET opcode, DUPLET *p_c);									//Sets p_c to NNN
void opcode2NNN(DUPLET opcode, DUPLET *p_c, Stack *stack);							//Pushes current p_c to stack and jumps to NNN
void opcode3XNN(DUPLET opcode, DUPLET *p_c, BYTE *var_reg);							//skips one instruction if vx == NN
void opcode4XNN(DUPLET opcode, DUPLET *p_c, BYTE *var_reg);							//skips one instruction if vx != NN
void opcode5XY0(DUPLET opcode, DUPLET *p_c, BYTE *var_reg);							//skips one instruction if vx == vy
void opcode6XNN(DUPLET opcode, BYTE *var_reg);  								//Sets register vx to NN
void opcode7XNN(DUPLET opcode, BYTE *var_reg);									//Adds NN to register vx
void opcode8XY0(DUPLET opcode, BYTE *var_reg);									//Sets vx to vy
void opcode8XY1(DUPLET opcode, BYTE *var_reg);									//Sets vx to vx OR vy
void opcode8XY2(DUPLET opcode, BYTE *var_reg);									//Sets vx to vx AND vy
void opcode8XY3(DUPLET opcode, BYTE *var_reg);									//Sets vx to vx ZOR vy
void opcode8XY4(DUPLET opcode, BYTE *var_reg);									//Sets vx to vx += vy if overflow var_reg set to 1 else 0
void opcode8XY5(DUPLET opcode, BYTE *var_reg);									//Sets vx to vx -= vy if underflow var_reg set to 0 else 1
void opcode8XY6(DUPLET opcode, BYTE *var_reg);									//Sets vf to vx >> 1
void opcode8XYE(DUPLET opcode, BYTE *var_reg);									//Sets vf to vx << 1
void opcode8XY7(DUPLET opcode, BYTE *var_reg);									//Sets vx to vy - vx var_reg16 set if vy >= vx
void opcode9XY0(DUPLET opcode, DUPLET *p_c, BYTE *var_reg);							//skips one instruction if vx != vy
void opcodeANNN(DUPLET opcode, DUPLET *i_reg);									//Sets I_reg to NNN
void opcodeDXYN(DUPLET opcode, BYTE *var_reg, DUPLET i_reg, BYTE display[][SCREEN_HEIGHT], BYTE *memory); 	//Display
void opcodeFX33(DUPLET opcode, BYTE *memory, DUPLET i_reg, BYTE *var_reg);					//Binary coded decimal conversion
void opcodeFX55(DUPLET opcode, BYTE *var_reg, DUPLET i_reg, BYTE *memory);					//Loads var_reg into memory
void opcodeFX65(DUPLET opcode, BYTE *var_reg, DUPLET i_reg, BYTE *memory);					//Loads memory into v[0] to v[X]
void opcodeFX1E(DUPLET opcode, BYTE *var_reg, DUPLET *i_reg);							//Adds var_reg[vx] to i_reg

#endif//CHIP_8_INSTRUCTIONS_H
