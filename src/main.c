#include "stack.h"

#include <stdbool.h>
#include <stdlib.h>
#include <SDL.h>

typedef unsigned char  BYTE;
typedef unsigned short DUPLET;

typedef struct Stack Stack;

const int SCREEN_WIDTH  = 64;
const int SCREEN_HEIGHT = 32;
const int VIDEO_SCALE	= 20;

const int MEMORY_SIZE	= 4096;

SDL_Window *window	= NULL;
SDL_Renderer *renderer	= NULL;
SDL_Texture *texture 	= NULL;

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

bool init(void);												//Initializes SDL2
void quit(void);												//Quits and cleans SDL2

int main(int argc, char *argv[]) {
	
	BYTE memory[MEMORY_SIZE];			//Load font/rom into
	BYTE display[SCREEN_WIDTH][SCREEN_HEIGHT];	//Sets to 1(on) or 0(off)
	BYTE var_reg[16];				//Holds different variables
	
	DUPLET p_c;					//Program counter points to current instruction in memory
	DUPLET opcode;					//Holds current operating code p_c is pointing to
	DUPLET i_reg;					//Points at locations in memory
	
	Stack stack;
	stack.top = 0;
	
	Uint32 *pixels;
	int pitch;
	
	if (!init()) {
		printf("Failed to initialize SDL2\n");
	} else {
		printf("SDL2 initialized\n");
		bool running = true;
		SDL_Event e;
		
		
		
		if (!load_rom(memory)) {
			printf("Rom not loaded\n");
			return 1;
		} else {
			printf("Rom loaded\n");
		}
		
		for (int x = 0; x < 16; ++x)
			var_reg[x] = 0;
		i_reg = 0;
		p_c = 512;
		
		printf("\n");
		while (running) {
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT) {
					running = false;
				}
			}
			printf("Getting opcode!\n");
			opcode = get_opcode(memory, &p_c);
			
			
		printf("\n");	
		switch (opcode & 0xF000) {
		case 0x0000:
			printf("Opcode: %X\n", opcode);
			printf("0x0000, decoding further\n");
			switch (opcode & 0x00FF) {
			case 0xE0:
				printf("0x00E0, clear screen\n");
				opcodeEO(display);
				break;
			case 0xEE:
				printf("0x00EE, returning subroutine\n");
				opcodeEE(&p_c, &stack);
				break;
			default:
				printf("Opcode not recognised\n");
				running = false;
				break;
			}
			break;
		case 0x1000:
			printf("Opcode: %X\n", opcode);
			printf("0x1000, Setting p_c to NNN\n");
			opcode1NNN(opcode, &p_c);
			printf("P_C: %X\n", p_c);
			break;
		case 0x2000:
			printf("Opcode: %X\n", opcode);
			printf("0x2000, pushing current p_c to stack and jumping to NNN\n");
			opcode2NNN(opcode, &p_c, &stack);
			break;
		case 0x3000:
			printf("Opcode: %X\n", opcode);
			printf("0x3000, Skipping on p_c instruction if vx equal NN\n");
			opcode3XNN(opcode, &p_c, var_reg);
			break;
		case 0x4000:
			printf("Opcode: %X\n", opcode);
			printf("0x4000, Skipping on p_c instruction if vx not equal NN\n");
			opcode4XNN(opcode, &p_c, var_reg);
			break;
		case 0x5000:
			printf("Opcode: %X\n", opcode);
			printf("0x5000, Skipping on p_c instruction if vx equals vy\n");
			opcode5XY0(opcode, &p_c, var_reg);
			break;
		case 0x6000:
			printf("Opcode: %X\n", opcode);
			printf("0x6000, Setting var_reg\n");
			opcode6XNN(opcode, var_reg);
			printf("I_Reg: %X\n", i_reg);
			for (int x = 0; x < 16; ++x) {
				printf("var_reg[%X]: %X\t", x, var_reg[x]);
				if ((x % 5 == 0))
					printf("\n");
			}
			printf("\n");
			break;
		case 0x7000:
			printf("Opcode: %X\n", opcode);
			printf("0x7000, Adding to vx\n");
			opcode7XNN(opcode, var_reg);
			break;
		case 0x8000:
			printf("Opcode: %X\n", opcode);
			printf("0x8000, decoding further\n");
			switch(opcode & 0x000F) {
			case 0x0:
				printf("0x8XY0, sets value of vx to vy\n");
				opcode8XY0(opcode, var_reg);
				break;
			case 0x1:
				printf("0x8XY1, Sets vx to vx or vy\n");
				opcode8XY1(opcode, var_reg);
				break;
			case 0x2:
				printf("0x8XY2, Sets vx to vx and vy\n");
				opcode8XY2(opcode, var_reg);
				break;
			case 0x3:
				printf("0x8XY3, Sets vx to vx zor vy\n");
				opcode8XY3(opcode, var_reg);
				break;
			case 0x4:
				printf("0x8XY4, Sets vx to vx += vy\n");
				opcode8XY4(opcode, var_reg);
				break;
			case 0x5:
				printf("0x8XY5, Sets vx to vx -= vy var_reg[16] set if underflow\n");
				opcode8XY5(opcode, var_reg);
				break;
			case 0x6:
				printf("0x8XY6, sets vf to vx >> 1\n");
				opcode8XY6(opcode, var_reg);
				break;
			case 0x7:
				printf("0x8XY7, Sets vx to vy - vx var_reg[16] sets flag if vy >= vx\n");
				opcode8XY7(opcode, var_reg);
				break;
			case 0xE:
				printf("0x8XY6, sets vf to vx << 1\n");
				opcode8XYE(opcode, var_reg);
				break;
			default:
				printf("Opcode not recognised\n");
				running = false;
				break;
			}
			for (int x = 0; x < 16; ++x) {
				printf("var_reg[%X]: %X\t", x, var_reg[x]);
				if ((x % 5 == 0))
					printf("\n");
				}
				printf("\n");
			break;
		case 0x9000:
			printf("Opcode: %X\n", opcode);
			printf("0x9000, Skipping on p_c instruction if vx doesnt equal vy\n");
			opcode9XY0(opcode, &p_c, var_reg);
			break;
		case 0xA000:
			printf("Opcode: %X\n", opcode);
			printf("0xA000, Setting index register\n");
			opcodeANNN(opcode, &i_reg);
			printf("Index_Reg set: %X\n", i_reg);
			break;
		case 0xD000:
			printf("Opcode: %X\n", opcode);
			printf("0xD000, Drawing\n");
			opcodeDXYN(opcode, var_reg, i_reg, display, memory);
			break;
		default:
			printf("Opcode: %X\n", opcode);
			printf("Opcode not recognised\n");
			running = false;
			break;
		}
		printf("\n");
		printf("p_c: %X\n", p_c);
		
		
		SDL_LockTexture(texture,NULL,(void**)&pixels, &pitch);
		SDL_memset(pixels, 255, pitch * 32);
		
		for (int y = 0; y < 32; ++y) {
			for (int x = 0; x < 64; ++x) {
				if (display[x][y] != 0) {
					pixels[y * 64 + x] = 0;
				}
			}
		}
		
		SDL_UnlockTexture(texture);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		}
	}
	quit();
	return 0;
}

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




bool init(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		return false;
	}
	window = SDL_CreateWindow("CHIP8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * VIDEO_SCALE,SCREEN_HEIGHT * VIDEO_SCALE, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Window could not be created!\n");
		return false;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("Renderer could not be created!\n");
		return false;
	}
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (texture == NULL) {
		printf("Texture could not be created!\n");
		return false;
	}
	return true;
}
void quit(void)
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	printf("SDL has closed succesfully!\n");
}