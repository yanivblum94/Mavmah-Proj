#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define CMDBITS 20 //number of bits in a command
#define REGRBITS 32 //number of bits in a register
#define LINELEN 500 //length of max length
#define MAXLABEL 50 //length of max label


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

char* check_label(char * ptr) {//check if word ends with :
	char* label;
	label = strrchr(ptr, ':');
	return label;
}



int main(int argc, char** argv) {
	char line[LINELEN];
	int labels_num, pc = 0;
	char delim[] = " ,\t\n:";//the characters that break the linw
	char label_table[1024][2][50];//max 1024 labels, 2 options for the name(pc and name), max len of label
	char pc_as_str[20];//the address of the pc as treing
	FILE *input;
	input = fopen(argv[1], "r");
	if (input == NULL) {
		fprintf(stderr, "Can't open input file \n");
		exit(1);
	}
	char* ptr;
	//first run over the code for the labels
	while (fgets(line, sizeof line, input) != NULL) {
		ptr = strtok(line, delim);
		bool is_first = true;
		while (ptr != NULL) {
			if (is_first) {
				if (check_label(ptr) != NULL) {//if we have a label
					sprintf(pc_as_str, "%d", pc);
					strcpy(label_table[labels_num][0], pc_as_str);
					strcpy(label_table[labels_num][1], ptr);
					labels_num++;
					ptr = strtok(NULL, delim);
				}
				is_first = false;
			}

		}
	}
}
