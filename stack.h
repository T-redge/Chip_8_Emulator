#include <stdio.h>

#ifndef STRUCTS_H
#define STRUCTS_H

typedef unsigned short DUPLET;

#define STACK_SIZE 16

typedef struct Stack
{
	int top;
	DUPLET mem_address[STACK_SIZE];
	
	
}Stack;

void push(Stack *st, DUPLET new_address);

#endif//STRUCTS_H