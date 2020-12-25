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
#define HWREGS 22// number of HW Registers
#define MEMSIZE 4096
#define MAXPC 1024 // max val of PC

static int pc = 0;
static int instructions_count = 0;
static int proc_regs[REGSNUM] = { 0 };//updates the values of the processor registers
static int hw_regs[HWREGS];//updates the values of the hardware registers
static char instructions[MAXPC][6]={ NULL } ;

//get Hex rep of a numbre including negative
void get_hex_from_int(unsigned int num,  int num_of_bytes, char* hex) {

	hex[num_of_bytes] = '\0';
	int i = num_of_bytes - 1;

	do {
		hex[i] = "0123456789ABCDEF"[num % 16];
		i--;
		num /= 16;
	} while (i >= 0);
}

void update_trace(char *inst, char* res) {// creates a string for the trace_out file according to the format needed
	int i=0;
	char regs_hex[15][12];
	for (i; i < REGSNUM; i++) {
		char temp[12];
		get_hex_from_int(proc_regs[i], 8, temp);
		strcpy(regs_hex[i], temp);
	}
	char temp[6];
	get_hex_from_int(pc, 3, temp);
	sprintf(res, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s", temp, inst, regs_hex[0],
		regs_hex[1], regs_hex[2], regs_hex[3], regs_hex[4], regs_hex[5], regs_hex[6], regs_hex[7],
		regs_hex[8], regs_hex[9], regs_hex[10], regs_hex[11], regs_hex[12], regs_hex[13], regs_hex[14],
		regs_hex[15]);
}

bool is_immediate(char* inst) {//checks if an instruction is an immediate type
	return (inst[2] == "1" || inst[3] == "1" || inst[4] == "1");
}

void update_instructions(char* file_name) {//updates the instructions array - puts the instruction in the place ndexed by the PC
	FILE *input;
	int i = 0;
	char line[LINELEN];
	input = fopen(file_name, "r");
	if (input == NULL) {
		fprintf(stderr, "Can't open input file \n");
		exit(1);
	}
	while (fgets(line, LINELEN, input) != NULL) {
		strcpy(instructions[i], line);
		i++;
	}
	fclose(input);
}

int main(int argc, char** argv[]) {
	char res[160];
	update_trace("00012", res);
	printf(res);
	return 0;
}



// checking if any interupt is on - function called every clock cycle
static int check_signal() {
	int irq;
	irq = (hw_regs[0] && hw_regs[3]) || (hw_regs[1] && hw_regs[4]) || (hw_regs[2] && hw_regs[5]);
	return irq;
}


/* setting registers properly before moving into interput routine
function is called if we are not in interput routine  allready
or the current PC instruction is not imm
*/
static void move_to_interrupt_Routine() {
	char* curr_inst = instructions[pc];
	if (!is_immediate(curr_inst)) {	// curr instruction is not imm -> move to interrput routine
		hw_regs[7] = pc;
		pc = hw_regs[6];
	}
		// need to add a check if we are in interrupt routine and act properly 
}