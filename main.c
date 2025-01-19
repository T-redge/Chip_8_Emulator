/*
 *Create an CHIP_8 interpretor which prints IBM logo to console.
 *Next step would be to transfer that to GRAPHICS instead of console.
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL.h>

typedef unsigned char  BYTE;
typedef unsigned short DUPLET;

const int SCREEN_WIDTH  = 64;
const int SCREEN_HEIGHT = 32;

SDL_Window *window	= NULL;
SDL_Renderer *renderer	= NULL;
SDL_Texture *texture 	= NULL;

bool load_rom(BYTE *memory);									//loads rom into memory
long get_file_size(FILE* file);									//Aquires file size for memory allocation
DUPLET get_opcode(BYTE *memory, DUPLET *p_c);							//Gets operator code from memory

void opcodeEO(BYTE display[][SCREEN_HEIGHT]);							//Sets Display to 0's
void opcode1NNN(DUPLET opcode, DUPLET *p_c);							//Sets p_c to NNN
void opcode6XNN(DUPLET opcode, BYTE *var_reg);  						//Sets register vx to NN
void opcode7XNN(DUPLET opcode, BYTE *var_reg);							//Adds NN to register vx
void opcodeANNN(DUPLET opcode, DUPLET *i_reg);							//Sets I_reg to NNN
void opcodeDXYN(DUPLET opcode, BYTE *var_reg, DUPLET i_reg, BYTE display[][32], BYTE *memory); 	//Display

bool init(void);										//Initializes SDL2
void quit(void);										//Quits and cleans SDL2

int main(int argc, char *argv[]) {
	
	BYTE memory[4096];				//Load font/rom into
	BYTE display[SCREEN_WIDTH][SCREEN_HEIGHT];	//Sets to 1(on) or 0(off)
	BYTE var_reg[16];				//Holds different variables
	
	DUPLET p_c;					//Program counter points to current instruction in memory
	DUPLET opcode;					//Holds current operating code p_c is pointing to
	DUPLET i_reg;					//Points at locations in memory
	
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
			for (int i = 512; i < 645; ++i)
				printf("%X\t", memory[i]);
			printf("\n");
		}
		
		for (int x = 0; x < 16; ++x)
			var_reg[x] = 0;
		i_reg = 0;
		p_c = 512;
		
		while (running) {
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT) {
					running = false;
				}
			}
			opcode = get_opcode(memory, &p_c);
			
			printf("\n");
			printf("Opcode: %X\n", opcode);
			printf("P_C: %X\n", p_c);
			printf("I_Reg: %X\n", i_reg);
			for (int x = 0; x < 16; ++x) {
				printf("var_reg[%d]: %X\t", x, var_reg[x]);
				if ((x % 5 == 0))
					printf("\n");
			}
			printf("\n");
			
			switch (opcode & 0xF000) {
			case 0x0000:
				printf("0x0000, decoding further\n");
				switch (opcode & 0x00FF) {
				case 0xE0:
					printf("0x00E0, clear screen\n");
					opcodeEO(display);
					for (int y = 0; y < 32; ++y) {
						for(int x = 0; x < 64; ++x) {
							printf("%d", display[x][y]);
						}
						printf("\n");
					}
					break;
				default:
					printf("Opcode not recognised\n");
					running = false;
					break;
				}
				break;
			case 0x1000:
				printf("0x1000, Setting p_c to NNN\n");
				opcode1NNN(opcode, &p_c);
				break;
			case 0x6000:
				printf("0x6000, Setting var_reg\n");
				opcode6XNN(opcode, var_reg);
				break;
			case 0x7000:
				printf("0x7000, Adding to vx\n");
				opcode7XNN(opcode, var_reg);
				break;
			case 0xA000:
				printf("0xA000, Setting index register\n");
				opcodeANNN(opcode, &i_reg);
				break;
			case 0xD000:
				printf("0xD000, Drawing\n");
				opcodeDXYN(opcode, var_reg, i_reg, display, memory);
				break;
			default:
				printf("Opcode not recognised\n");
				running = false;
				break;
			}
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
	const char *file_name = "IBMLogo.ch8";
	bool success = true;
	
	FILE *file_open = fopen(file_name, "rb");
	if (!file_open) {
		perror("File opening failed\n");
		success = false;
	}
	
	size_t file_size = get_file_size(file_open);
	
	
	BYTE *buffer = malloc(file_size * sizeof(char));
	while (fread(buffer, sizeof(buffer[0]), file_size, file_open) != 0) {
		for (size_t i = 0; i < file_size; ++i) 
			printf("%X\t", buffer[i]);
		printf("\n");
	}
	
	for (size_t i = 0; i < file_size; ++i)
		memory[512 + i] = buffer[i];
	
	free(buffer);
	fclose(file_open);
	return success;
}
long get_file_size(FILE *file)
{
	fseek(file, sizeof(char), SEEK_END);
	long int tmp = ftell(file);
	printf("Size of file is: %ld bytes\n", tmp);
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
void opcodeEO(BYTE display[][32])
{
	for (int x = 0; x < 64; ++x)
		for(int y = 0; y < 32; ++y)
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
	DUPLET vx, tmp;
	vx = (opcode & 0x0F00) >> 8;
	tmp = opcode & 0x00FF;
	
	var_reg[vx] = tmp;
}
void opcodeDXYN(DUPLET opcode, BYTE *var_reg, DUPLET i_reg, BYTE display[][32], BYTE *memory)
{
	DUPLET vx, vy, rows, coord_x, coord_y, sprite_data;
	vx = opcode & 0x0F00;
	vy = (opcode & 0x00F0) >> 4;
	rows = opcode & 0x000F;
	
	coord_x = var_reg[vx & 63];
	coord_y = var_reg[vy & 31];
	
	var_reg[15] = 0;
	
	printf("vx: %X, vy: %X, rows: %X, coord_x: %X, coord_y: %X\n", vx, vy, rows, coord_x, coord_y);

	for (int i = 0; i < rows; ++i) {
		sprite_data = memory[i_reg + i];
		printf("Sprite_data: %X\n", sprite_data);
		int xpixelinv = 7;
		int xpixel = 0;
		
		
		for (xpixel = 0; xpixel < 8; ++xpixel, --xpixelinv) {
			int mask = 1 << xpixelinv;
			printf("%X\n", mask);
			if (sprite_data & mask) {
				int x = coord_x + xpixel;
				int y = coord_y + i;
				if (display[x][y] == 1)
					var_reg[15] = 1;
				display[x][y] ^= 1;
			}
		}
	}
	for (int y = 0; y < 32; ++y) {
		for(int x = 0; x < 64; ++x) {
			printf("%d", display[x][y]);
		}
		printf("\n");
	}
}
void opcode7XNN(DUPLET opcode, BYTE *var_reg)
{
	DUPLET vx, tmp;
	vx = opcode & 0x0F00;
	tmp = opcode & 0x00FF;
	
	var_reg[vx] = var_reg[vx] + tmp;
	
	printf("vx: %X, tmp: %X, var_reg[%X]: %X\n", vx, tmp, vx, var_reg[vx]);
}
void opcode1NNN(DUPLET opcode, DUPLET *p_c)
{
	DUPLET tmp;
	tmp = opcode & 0x0FFF;
	
	*p_c = tmp;
}
bool init(void)
{
	bool success = true;
	
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
		success = false;
	} else {
		window = SDL_CreateWindow("CHIP8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * 10,SCREEN_HEIGHT * 10, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			printf("Window could not be created!\n");
			success = false;
		} else {
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == NULL) {
				printf("Renderer could not be created!\n");
				success = false;
			} else {
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
				texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
				if (texture == NULL) {
					printf("Texture could not be created!\n");
					success = false;
				}
			}
		}
	}
	
	return success;
}
void quit(void)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	printf("SDL has closed succesfully!\n");
}