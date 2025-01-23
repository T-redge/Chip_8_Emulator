#include "stack.h"

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