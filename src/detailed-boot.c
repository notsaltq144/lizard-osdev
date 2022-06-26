//#include "stdc/stdbool.h"
//#include "stdc/stdio.h"
asm(".code32");
int a(int *d);
int main() {
	int c = 0;
	c = a(&c);
	asm("cli");
	asm("hlt");
	c = a(&c);
	c = a(&c);
	while(1);
}
int a(int *d) {
	asm("cli");
	asm("hlt");
	int b = 1;
	b *= 5;
	b *= 7;
	b -= 6;
	b *= 9;
	*d = b;
	return b;
}
//#include "stdc/stdio.c"

