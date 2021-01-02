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

static int pc = 0;//static count of the PC
static int clock_cycles = 0;//stitc counter of clockcycles
static int tot_instructions_done = 0;//how many instructions we did
static int total_lines = 0;//how many lines we got in the imemin file
static int proc_regs[REGSNUM] = { 0 };//updates the values of the processor registers
static int hw_regs[HWREGS];//updates the values of the hardware registers
static char instructions[MAXPC][6]={ NULL } ;
static int instructions_mapping[MAXPC] = { 0 };//puts 0 in the array if the line is an instruction, 1 if immediate
static int memory[MEMSIZE];
const static char hex_vals[22][3] = { "0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F", "10", "11", "12", "13", "14", "15" };

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

int get_basic_hex_val(char* hex) {
	for (int i = 0; i < 22; i++) {
		if (!strcmp(hex, hex_vals[i])) { return i; }
	}
	return -1;
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
	return (inst[2] == '1' || inst[3] == '1' || inst[4] == '1');
}

void handle_cmd(int pc_index, bool is_imm) {
	char cmd[7] = instructions[pc_index];
	char op[3] = { cmd[0],cmd[1],'\0' };
	char rd[2] = { cmd[2], '\0' };
	char rs[2] = { cmd[3], '\0' };
	char rt[2] = { cmd[4], '\0' };
	int op_num = get_basic_hex_val(op);
	int rd_num = get_basic_hex_val(rd);
	int rs_num = get_basic_hex_val(rs);
	int rt_num = get_basic_hex_val(rt);
	//operations
	if (op_num == 0) {//add
		proc_regs[rd_num] = proc_regs[rs_num] + proc_regs[rt_num];
		return;
	}
	if (op_num == 1) {//sub
		proc_regs[rd_num] = proc_regs[rs_num] - proc_regs[rt_num];
		return;
	}
	if (op_num == 2) {//and
		proc_regs[rd_num] = proc_regs[rs_num] & proc_regs[rt_num];
		return;
	}
	if (op_num == 3) {//or
		proc_regs[rd_num] = proc_regs[rs_num] | proc_regs[rt_num];
		return;
	}
	if (op_num == 4) {//xor
		proc_regs[rd_num] = proc_regs[rs_num] ^ proc_regs[rt_num];
		return;
	}
	if (op_num == 5) {//mul
		proc_regs[rd_num] = proc_regs[rs_num] * proc_regs[rt_num];
		return;
	}
	if (op_num == 6) {//sll
		proc_regs[rd_num] = proc_regs[rs_num] << proc_regs[rt_num];
		return;
	}
	if (op_num == 7) {//sra
		proc_regs[rd_num] = (int)proc_regs[rs_num] >> (int)proc_regs[rt_num];
		return;
	}
	if (op_num == 8) {//srl
		proc_regs[rd_num] = proc_regs[rs_num] >> proc_regs[rt_num];
		return;
	}
	if (op_num == 9) {//beq
		if (proc_regs[rs_num] == proc_regs[rt_num]) { pc = proc_regs[rd_num] & 0x3FF; }//taking te lowest 10 bits (3FF in hex is 1023 in dec an 10 1's in binary)
		return;
	}
	if (op_num == 10) {//bne
		if (proc_regs[rs_num] != proc_regs[rt_num]) { pc = proc_regs[rd_num] & 0x3FF; }
		return;
	}
	if (op_num == 11) {//blt
		if (proc_regs[rs_num] < proc_regs[rt_num]) { pc = proc_regs[rd_num] & 0x3FF; }
		return;
	}
	if (op_num == 12) {//bgt
		if (proc_regs[rs_num] > proc_regs[rt_num]) { pc = proc_regs[rd_num] & 0x3FF; }
		return;
	}
	if (op_num == 13) {//ble
		if (proc_regs[rs_num] <= proc_regs[rt_num]) { pc = proc_regs[rd_num] & 0x3FF; }
		return;
	}
	if (op_num == 14) {//bge
		if (proc_regs[rs_num] >= proc_regs[rt_num]) { pc = proc_regs[rd_num] & 0x3FF; }
		return;
	}
	if (op_num == 15) {//jal
		if (is_imm) { proc_regs[15] = pc_index + 2; }
		else { proc_regs[15] = pc_index + 1; }
		pc = proc_regs[rd_num] & 0x3FF;
		return;
	}
	if (op_num == 16) {//lw
		proc_regs[rd_num] = memory[(proc_regs[rs_num] + proc_regs[rt_num])%MEMSIZE];
		return;
	}
	if (op_num == 17) {//sw
		memory[(proc_regs[rs_num] + proc_regs[rt_num])%MEMSIZE] = proc_regs[rd_num] ;
		return;
	}
	/*todo op 18, 19, 20
	if (op_num == 18) {//reti
		pc = 
		return;
	}
	if (op_num == 19) {//in
		proc_regs[rd_num] = 
		return;
	}
	if (op_num == 209) {//ou
		 = proc_regs[rd_num];
		return;
	}
	*/
	if (op_num == 21) {//halt
		pc = total_lines + 1;
			return;
	}
}

int update_instructions(char* file_name) {//updates the instructions array - puts the instruction in the place ndexed by the PC, returns num of PC's
	int i = 0;
	char line[LINELEN];
	bool has_imm = false;
	FILE *input = fopen(file_name, "r");
	if (input == NULL) {
		fprintf(stderr, "Can't open input file \n");
		exit(1);
	}
	while (fgets(line, LINELEN, input) != NULL) {
		strcpy(instructions[i], line);
		has_imm = is_immediate(line);
		if (has_imm) {
			i++;
			instructions_mapping[i] = 1;
			if (fgets(line, LINELEN, input) == NULL) { break; }
			strcpy(instructions[i], line);
		}
		i++;
	}
	fclose(input);
	return i;
}

//initiate the memoty array of the SIMP
void init_memory(char* file_name) {
	char line[10];
	int curr_line = 0;
	FILE* dmemin = fopen(file_name, "r");
	if (dmemin == NULL) {
		fprintf(stderr, "Can't open input file \n");
		exit(1);

	}
	while (fgets(line, 10, dmemin) != NULL) {
		memory[curr_line] = strtoul(line, NULL, 16);
		curr_line++;
	}
	while (curr_line < MEMSIZE) {//in case the file we got does not have all the 0 lines
		memory[curr_line] = 0;
		curr_line++;
	}
	fclose(dmemin);
}

void write_dmem_out(char* file_name) {
	FILE *dmemout = fopen(file_name, 10, "w");
	if (dmemout == NULL) {
		fprintf(stderr, "Can't open input file \n");
		exit(1);
	}
	for (int i = 0; i < MEMSIZE; i++) {
		fprintf(dmemout, "%08X\n", memory[i]);
	}
	fclose(dmemout);
}

void write_cycles(char* file_name) {//write the cycles output files
	FILE *cycles = fopen(file_name, 10, "w");
	if (cycles == NULL) {
		fprintf(stderr, "Can't open input file \n");
		exit(1);
	}
	fprintf(cycles, "%d\n", clock_cycles);
	fprintf(cycles, "%d\n", tot_instructions_done);
	fclose(cycles);
}

void write_regout(char* file_name) {//write the regout output file
	FILE *regout = fopen(file_name, 10, "w");
	if (regout == NULL) {
		fprintf(stderr, "Can't open input file \n");
		exit(1);
	}
	for (int i = 2; i < REGSNUM; i++) {
		fprintf(regout, "08X\n", proc_regs[1]);
	}
	fclose(regout);
}


int main(int argc, char** argv[]) {
	char trace[160];	//update_trace("00012", res);
	init_memory(argv[2]);
	total_lines = update_instructions(argv[1]);
	//printf(res);
	//main loop
	FILE *trace = fopen(argv[7], "w");
	while (pc < total_lines) {
		bool is_imm = is_immediate(instructions[pc]);
		if (is_imm) { proc_regs[1] = strtoul(instructions[pc + 1], NULL, 16); }//update imm value
		update_trace(instructions[pc], trace);
		fprintf(trace, "%s\n", trace);
		if (is_imm) { pc++; }
		pc++;
		/*TODO
		Handle IO
		Handle interrupts
		?handle monitor?
		*/

		tot_instructions_done++;
	}
	write_dmem_out(argv[5]);
	write_regout(argv[6]);
	write_cycles(argv[9]);
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