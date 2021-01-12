
#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define CMDBITS 20 //number of bits in a command
#define REGRBITS 32 //number of bits in a register
#define LINELEN 500 //length of max line
#define MAXLABEL 50 //length of max label
#define REGSNUM 16 // number if registers
#define OPCODESNUM 22// number of opcodes
#define MEMSIZE 4096
/*
Register Number Register Name Purpose
0 $zero Constant zero
1 $imm Sign extended immediate
2 $v0 Result value
3 $a0 Argument register
4 $a1 Argument register
5 $t0 Temporary register
6 $t1 Temporary register
7 $t2 Temporary register
8 $t3 Temporary register
9 $s0 Saved register
10 $s1 Saved register
11 $s2 Saved register
12 $gp Global pointer(static data)
13 $sp Stack pointer
14 $fp Frame Pointer
15 $ra Return address


Opcode Number Name Meaning
0 add R[rd] = R[rs] + R[rt]
1 sub R[rd] = R[rs] � R[rt]
2 and R[rd] = R[rs] & R[rt]
3 or R[rd] = R[rs] | R[rt]
4 xor R[rd] = R[rs] ^ R[rt]
5 mul R[rd] = R[rs] * R[rt]
6 sll R[rd] = R[rs] << R[rt]
7 sra R[rd] = R[rs] >> R[rt], arithmetic shift with sign extension
8 srl R[rd] = R[rs] >> R[rt], logical shift
9 beq if (R[rs] == R[rt]) pc = R[rd][low bits 9:0]
10 bne if (R[rs] != R[rt]) pc = R[rd] [low bits 9:0]
11 blt if (R[rs] < R[rt]) pc = R[rd] [low bits 9:0]
12 bgt if (R[rs] > R[rt]) pc = R[rd] [low bits 9:0]
13 ble if (R[rs] <= R[rt]) pc = R[rd] [low bits 9:0]
14 bge if (R[rs] >= R[rt]) pc = R[rd] [low bits 9:0]
15 jal R[15] = next instruction address, pc = R[rd][9:0]
16 lw R[rd] = DMEM[R[rs]+R[rt]]
17 sw DMEM[R[rs]+R[rt]] = R[rd]
18 reti PC = IORegister[7]
19 in R[rd] = IORegister[R[rs] + R[rt]]
20 out IORegister [R[rs]+R[rt]] = R[rd]
21 halt Halt execution, exit simulator
*/

static char dmem[MEMSIZE][9] = { NULL };
const static char hex_vals[22][3] = { "0","1","2","3","4","5","6","7" ,"8","9","A","B","C","D","E","F", "10", "11", "12", "13", "14", "15" };
const static char  registers[REGSNUM][6] = { "$zero", "$imm", "$v0", "$a0", "$a1", "$t0", "$t1", "$t2", "$t3", "$s0", "$s1","$s2", "$gp ", "$sp", "$fp", "$ra" };
const static char opcodes[OPCODESNUM][5] = { "add", "sub", "and", "or", "xor", "mul", "sll", "sra", "srl", "beq", "bne", "blt", "bgt", "ble", "bge", "jal", "lw", "sw", "reti", "in", "out", "halt" };
static char label_table[1024][2][MAXLABEL + 1];//max 1024 labels, 2 options for the name(pc and name), max len of label
static int labels_num = 0;


int get_reg_num(char *reg_name) {//receives name of a reg and returnes its decimal value
	int i = 0;
	if (reg_name == NULL) { return -1; }
	for (int i = 0; i < REGSNUM; i++) {
		if (!strcmp(reg_name, registers[i]))
			return i;
	}
	return -1;
}

int get_opcode_num(char *opcode) {//receives an opcode and returns its decimal value 
	int i = 0;
	for (int i = 0; i < OPCODESNUM; i++) {
		if (!strcmp(opcode, opcodes[i]))
			return i;
	}
	return -1;
}

//check if the const value is in hex rep.
bool is_hex(char* value) {
	if (strlen(value) > 2 && (value[1] == 'x' || value[1] == 'X') && value[0] == '0')//we are in Hex rep.
		return true;
	return false;
}

int dec_from_string(char* str)//get decimal value of a string
{
	if (is_hex(str))
	{
		return (int)strtol(str + 2, NULL, 16);
	}

	// we with decimal value
	return atoi(str);
}

//get Hex rep of a numbre including minus
void get_hex_from_int(unsigned int num, char* hex, int num_of_bytes) {

	hex[num_of_bytes] = '\0';
	int i = num_of_bytes - 1;

	do {
		hex[i] = "0123456789ABCDEF"[num % 16];
		i--;
		num /= 16;
	} while (i >= 0);
}

void remove_last_char(char* str) {//remove ':' from the label
	str[strlen(str) - 1] = '\0';
}


char* check_label(char * ptr) {//check if word ends with :, and return the label if it is a label
	char* label;
	label = strrchr(ptr, ':');
	if (label == NULL) {
		return NULL;
	}
	label = ptr;
	remove_last_char(label);
	return label;
}

int get_label_index(char *label) {//gets the labels table and a label and returns its PC
	int i = 0;
	for (i = 0; i < labels_num; i++) {
		if (!strcmp(label, label_table[i][1])) {
			return i;
		}
	}
	return -1;
}

void write_dmemin(char* file_name) {//writes the dmem_out file
	int i = 0;
	FILE *imem_file = fopen(file_name, "w");
	if (imem_file == NULL) {
		fprintf(stderr, "Can't open output file \n");
		exit(1);
	}
	for (i = 0; i < MEMSIZE; i++) {
		if (strlen(dmem[i]) <8) {
			fprintf(imem_file, "%s\n", "00000000");
		}
		else {
			fprintf(imem_file, "%s\n", dmem[i]);
		}
	}
	fclose(imem_file);
}


handle_word_cmd(char* address, char* data) {//method that updates the dmem array for .word command
	int index, val;
	index = dec_from_string(address);
	val = dec_from_string(data);
	sprintf(dmem[index], "%08X", val);
}



int main(int argc, char** argv) {
	char line[LINELEN];
	int  pc = 0;
	char delim[] = " ,\t\n";//the characters that break the line
	char pc_as_str[21];//the address of the pc as a string
	FILE *input, *output;
	input = fopen(argv[1], "r");
	if (input == NULL) {
		fprintf(stderr, "Can't open input file \n");
		exit(1);
	}
	char* ptr;
	//first run over the code for the labels
	while (fgets(line, LINELEN, input) != NULL) {
		ptr = strtok(line, delim);
		bool is_first = true;
		while (ptr != NULL) {
			if (is_first) {// if we are in the first word in the line 
				char *label = check_label(ptr);
				if (label != NULL) {//if we have a label
					sprintf(pc_as_str, "%d", pc);// pc_as_str = decimal value of PC as a string
					strcpy(label_table[labels_num][0], pc_as_str); // put the pc val in the place of the labels array
					strcpy(label_table[labels_num][1], label); // put the label itself in the place
					labels_num++;
					ptr = strtok(NULL, delim);// move to next word in line
				}
				if (ptr != NULL) {//check if it is an only label line
					if (strcmp(ptr, ".word") == 0) {// we have a .word cmd - skip to next line
						break;
					}
					else {//regular cmd line 
						pc++;
						ptr = strtok(NULL, delim);
					}
				}
				is_first = false;
			}
			else {
				//check if we need immediate
				if (!strcmp(ptr, "$imm")) {
					pc++;
					break;// I wan't to make sure I only increase PC once
				}
				ptr = strtok(NULL, delim);// move to next word
				if (ptr != NULL) {
					if (!strcmp(ptr, "#"))//skip this line we reached the # (comment)
						break;
				}
			}
		}
	}
	rewind(input);
	output = fopen(argv[2], "w");
	if (output == NULL) {
		fprintf(stderr, "Can't open output file \n");
		exit(1);
	}
	//2nd iteration over the code
	while (fgets(line, LINELEN, input) != NULL) {
		ptr = strtok(line, delim);
		bool is_first = true, has_imm=false;
		while (ptr != NULL) {
			if (is_first) {// if we are in the first word in the line 
				char *label = check_label(ptr);
				if (label != NULL) {//if we have a label
					ptr = strtok(NULL, delim);//move to the next word
					if (ptr == NULL || ptr[0] == '#') {// check if it an only label line
						break;
					}
					is_first = false;
				}
			}
			if (!strcmp(ptr, ".word")) {//we have a .word cmd
				ptr = strtok(NULL, delim);
				char *address = ptr;
				ptr = strtok(NULL, delim);
				char *data = ptr;
				handle_word_cmd(address, data);
				break;
			}
			//now we have a  pointer to a valid cmd
			int temp = get_opcode_num(ptr);
			char opcode_hex[20];
			sprintf(opcode_hex, "%02X", temp);
			fputs(opcode_hex, output);// enter opcode
			ptr = strtok(NULL, delim);//move to next word
			temp = get_reg_num(ptr);// get rd
			if (temp == 1)
				has_imm = true;
			fputs(hex_vals[temp], output);// enter rd
			ptr = strtok(NULL, delim);//move to next word
			temp = get_reg_num(ptr);
			if (temp == 1)
				has_imm = true;
			fputs(hex_vals[temp], output);// enter rs
			ptr = strtok(NULL, delim);//move to next word
			temp = get_reg_num(ptr);
			if (temp == 1)
				has_imm = true;
			fputs(hex_vals[temp], output);// enter rt
			fputs("\n", output);//down one line
			ptr = strtok(NULL, delim);//move to next word
			if (has_imm) {// check what to do with the imm value
				int label_index = get_label_index(ptr);
				char imm_val[21];
				if (label_index >= 0) {	//we have a label
					sprintf(imm_val, "%05X", dec_from_string(label_table[label_index][0]));
					fputs(imm_val, output);
				}
				else {
					int num = dec_from_string(ptr);
					char imm_val2[6];
					get_hex_from_int(num, imm_val2, 5);
					fputs(imm_val2, output);
				}
				fputs("\n", output);//down one line
			}
			break;
		}
	}
	write_dmemin(argv[3]);
	fclose(input);
	fclose(output);
	return 1;
}

