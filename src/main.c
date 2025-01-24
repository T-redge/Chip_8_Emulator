#include "chip8.h"

#include <SDL.h>

typedef struct Stack Stack;

const int VIDEO_SCALE	= 20;

int MEMORY_SIZE		= 4096;
int SCREEN_WIDTH	= 64;
int SCREEN_HEIGHT	= 32;


SDL_Window *window	= NULL;
SDL_Renderer *renderer	= NULL;
SDL_Texture *texture 	= NULL;

bool init(void);					//Initializes SDL2
void quit(void);					//Quits and cleans SDL2

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
		case 0xF000:
			printf("Opcode: %X\n", opcode);
			printf("0xF000, Decoding further\n");
			switch(opcode & 0x00FF) {
			case 0x33:
				printf("0x33, Converting hex to decimal\n");
				opcodeFX33(opcode, memory, i_reg, var_reg);
				printf("Memory[%X]: %X\t", i_reg, memory[i_reg]);
				printf("Memory[%X]: %X\t", i_reg+1, memory[i_reg+1]);
				printf("Memory[%X]: %X\t", i_reg+2, memory[i_reg+2]);
				break;
			case 0x55:
				printf("0x55, Loading to memory\n");
				opcodeFX55(opcode,var_reg,i_reg,memory);
				break;
			case 0x65:
				printf("0x65, Loading from memory\n");
				opcodeFX65(opcode, var_reg, i_reg, memory);
				break;
			case 0x1E:
				printf("0x1E, Adding vx to i_reg\n");
				opcodeFX1E(opcode, var_reg, &i_reg);
				printf("I_reg: %X\n", i_reg);
				//running = false;
				break;
			default:
				printf("Opcode not recognised\n");
				running = false;
				break;
			}
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