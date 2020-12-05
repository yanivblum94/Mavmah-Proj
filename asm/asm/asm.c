#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define CMDBITS 20; //number of bits in a command
#define REGRBITS 32; //number of bits in a register
#define LINELEN 500; //length of max length
#define MAXLABEL 50; //length of max label


const char hex_vals[16][1] = { "0","1","2","3","4","5","6","7" ,"8","9","A","B","C","D","E","F" };

int string_length(char s[])//returns the length of a char array
{
	int res = 0;

	while (s[res] != '\0')
		res++;

	return res;
}

int binary_to_decimal(char n[])//converts a binary numberin char to decimal int
{
	int i, res, mul = 0;

	for (res = 0, i = string_length(n) - 1; i >= 0; --i, ++mul)
		res += (n[i] - 48)*(1 << mul);

	return res;
}




void  main() {
	char* bin = "11111110";
	printf("%d", binary_to_decimal(bin));


}
