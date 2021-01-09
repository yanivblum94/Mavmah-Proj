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
#define SECTOR_SIZE 128 // 4 bytes each - 
#define SECTOR_NUMBER 128 // as defined in the project
#define PIXELS_X 352
#define PIXELS_Y 288
#define IMM_MASK 0xFFFFF
#define PC_MASK 0x3FF
#define MAX_CLOCK 4294967295 // 0xffffffff as defined in clks HWreg - limit
#define HWREG_NUM 22
#define HWREG_MAX_LENGTH 20


static unsigned int pc = 0;//static count of the PC
static int tot_instructions_done = 0;//how many instructions we did
static int total_lines = 0;//how many lines we got in the imemin file
static int proc_regs[REGSNUM] = { 0 };//updates the values of the processor registers
static unsigned int hw_regs[HWREGS];//updates the values of the hardware registers
static char instructions[MAXPC][6]={ NULL } ;
//static int instructions_mapping[MAXPC] = { 0 };//puts 0 in the array if the line is an instruction, 1 if immediate
static int memory[MEMSIZE];
//const static char hex_vals[22][3] = { "0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F", "10", "11", "12", "13", "14", "15" };
static int interrupt_routine = 0; // bit to represent if the simulator is currently in interrupt routine or note (1 or 0 )
static int disk[SECTOR_NUMBER][SECTOR_SIZE];
static int disk_timer;
static int monitor[PIXELS_X][PIXELS_Y];
static int next_irq2 = -1;
FILE *irq2in;
FILE *hwRegTraceFile;
FILE *leds_file;
FILE *trace_file;
const static char hwReg[HWREG_NUM][HWREG_MAX_LENGTH] = { "irq0enable" ,"irq1enable" ,"irq2enable" ,"irq0status" ,"irq1status" ,"irq2status" ,"irqhandler" ,"irqreturn" ,"clks" ,"leds" ,"reserved" ,"timerenable" ,"timercurrent" ,"timermax" ,"diskcmd" ,"disksector" ,"diskbuffer" ,"diskstatus" ,"monitorcmd" ,"monitorx" ,"monitory" ,"monitordata" };

void write_hwRegTrace(char cmd, int ioReg, int value);


//======================helpers======================
// function to get IOreg name from number

//void get_IOreg_name(int r, char* res) {
//	switch (r) {
//	case 0:
//		res =  "irq0enable";
//	case 1:
//		res =  "irq1enable";
//	case 2:
//		res =  "irq2enable";
//	case 3:
//		res =  "irq0status";
//	case 4:
//		res =  "irq1status";
//	case 5:
//		res =  "irq2status";
//	case 6:
//		res =  "irqhandler";
//	case 7:
//		res =  "irqreturn";
//	case 8:
//		res =  "clks";
//	case 9:
//		res =  "leds";
//	case 10:
//		res =  "reserved";
//	case 11:
//		res =  "timerenable";
//	case 12:
//		res =  "timercurrent";
//	case 13:
//		res =  "timermax";
//	case 14:
//		res =  "diskcmd";
//	case 15:
//		res =  "disksector";
//	case 16:
//		res =  "diskbuffer";
//	case 17:
//		res =  "diskstatus";
//	case 18:
//		res =  "monitorcmd";
//	case 19:
//		res =  "monitorx";
//	case 20: 
//		res =  "monitory";
//	case 21:
//		res =  "monitordata";
//
//	}
//}

bool is_immediate(char* inst) {//checks if an instruction is an immediate type
	return (inst[2] == '1' || inst[3] == '1' || inst[4] == '1');
}

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

//int get_basic_hex_val(char* hex) {
//	for (int i = 0; i < 22; i++) {
//		if (!strcmp(hex, hex_vals[i])) { return i; }
//	}
//	return -1;
//}
bool is_positive_imm(char* imm_hex) {
	return!(imm_hex[0] == '8' || imm_hex[0] == '9' || imm_hex[0] == 'A' || imm_hex[0] == 'B' ||
		imm_hex[0] == 'C' || imm_hex[0] == 'D' || imm_hex[0] == 'E' || imm_hex[0] == 'F');
}

int update_imm(char* imm_hex) {
	int imm = (int)strtol(imm_hex, NULL, 16);
	if (!is_positive_imm(imm_hex)) {
		imm = ((imm & IMM_MASK) << 12) >> 12;
	}
	return imm;
}

//=========================init fuctions=============================
int update_instructions(char* file_name) {//updates the instructions array - puts the instruction in the place indexed by the PC, returns num of PC's
	int i = 0;
	char line[LINELEN];
	bool has_imm = false;
	FILE *input = fopen(file_name, "r");
	if (input == NULL) {
		fprintf(stderr, "Can't open input file \n");
		exit(1);
	}
	while (fgets(line, LINELEN, input) != NULL) {
		line[5] = '\0';
		strcpy(instructions[i], line);
		has_imm = is_immediate(line);
		if (has_imm) {
			i++;
			//instructions_mapping[i] = 1;
			if (fgets(line, LINELEN, input) == NULL) { break; }
			line[5] = '\0';
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~` FILE WRITES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void write_hwRegTrace(char cmd, int ioReg, int value) {
	char temp[10];
	//char reg_name[HWREG_MAX_LENGTH] = hwReg[ioReg];
	get_hex_from_int(value, 8, temp);
	printf("temp = %s \n", temp);
	if (ioReg == 9 && cmd == 'w') {//update leds file
		fprintf(leds_file, "%d %08X\n", hw_regs[8]+1, hw_regs[9]);
	}
	switch (cmd) {
	case 'w':
		fprintf(hwRegTraceFile, "%d %s %s %s\n", hw_regs[8] +1, "WRITE", hwReg[ioReg], temp);
		break;
	case 'r':
		fprintf(hwRegTraceFile, "%d %s %s %s\n", hw_regs[8]+1 , "READ", hwReg[ioReg], temp);
		break;
	default:
		break;
	}
}

void write_dmem_out(char* file_name) {
	FILE *dmemout = fopen(file_name, "w");
	if (dmemout == NULL) {
		fprintf(stderr, "Can't open input file \n");
		exit(1);
	}
	for (int i = 0; i < MEMSIZE; i++) {
		fprintf(dmemout, "%08X\n", memory[i]);
	}
	fclose(dmemout);
}

void update_trace() {// updates the trace file according to format
	//char temp[100];
	fprintf(trace_file, "%03X %s", pc, instructions[pc]);
	//fputs(trace_file, temp);
	for (int i = 0; i < REGSNUM; i++) {
		char temp[20];
		get_hex_from_int(proc_regs[i], 8, temp);
		fprintf(trace_file, " %s", temp);
	}
	fprintf(trace_file, "%s", "\n");
}

void write_cycles(char* file_name) {//write the cycles output files
	FILE *cycles = fopen(file_name, "w");
	if (cycles == NULL) {
		fprintf(stderr, "Can't open input file \n");
		exit(1);
	}
	fprintf(cycles, "%d\n", hw_regs[8]);
	fprintf(cycles, "%d\n", tot_instructions_done);
	fclose(cycles);
}

void write_regout(char* file_name) {//write the regout output file
	FILE *regout = fopen(file_name, "w");
	if (regout == NULL) {
		fprintf(stderr, "Can't open input file \n");
		exit(1);
	}
	for (int i = 2; i < REGSNUM; i++) {
		fprintf(regout, "%08X\n", proc_regs[i]);
	}
	fclose(regout);
}

void timer_handler() {
	if (hw_regs[11] == 1) { hw_regs[12]++; }
	if (hw_regs[12] == hw_regs[13]) {
		hw_regs[3] = 1; 
		hw_regs[12] = 0;

	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~DISK ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//initiate the memoty array of the HARD DISK
void init_disk(char* file_name) {
	int sector;
	int offset;
	char line[10];
	int curr_line = 0;
	FILE* diskin = fopen(file_name, "r");
	if (diskin == NULL) {
		fprintf(stderr, "Can't open input file for disk \n");
		exit(1);
	}

	while (fgets(line, 10, diskin) != NULL) {
		sector = curr_line / SECTOR_NUMBER;
		offset = curr_line % SECTOR_SIZE;
		disk[sector][offset] = strtoul(line, NULL, 16);
		curr_line++;
	}
	while (curr_line < MEMSIZE) {//in case the file we got does not have all the 0 lines
		sector = curr_line / SECTOR_NUMBER;
		offset = curr_line % SECTOR_SIZE;
		disk[sector][offset] = 0;
		curr_line++;
	}
	fclose(diskin);
}

void write_diskout(char* file_name) {
	FILE *diskout = fopen(file_name, "w");
	if (diskout == NULL) {
		fprintf(stderr, "Can't open disk output file \n");
		exit(1);
	}
	for (int sector = 0; sector < SECTOR_NUMBER; sector++){
		for (int offset = 0; offset < SECTOR_SIZE; offset++) {
			fprintf(diskout, "%08X\n", disk[sector][offset]);
		}
	}

}

void write_sector() {
	int sector = hw_regs[15];
	int diskBuffer = hw_regs[16];

	for (int offset = 0; offset < SECTOR_NUMBER; offset++){
		disk[sector][offset] = memory[diskBuffer + offset];
	}
}

void read_sector() {
	int sector = hw_regs[15];
	int diskBuffer = hw_regs[16];

	for (int offset = 0; offset < SECTOR_NUMBER; offset++) {
		memory[diskBuffer + offset] = disk[sector][offset] ;
	}
}

void disk_handler() {
	if (hw_regs[17] == 0) {// check if the disk available for new instruction
		switch (hw_regs[14]) {
			case 1:	//read sector
				read_sector();
				break;
			case 2 :// writhe sector 
				write_sector();
				break;
			default:
				break;
		}
		disk_timer = 0;
	}

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`MONITOR ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//init monitor array to zeros
void init_monitor() {
	for (int x = 0; x < PIXELS_X; x++){
		for (int y = 0; y < PIXELS_Y; y++){
			monitor[x][y] = 0;

		}

	}
}

void monitor_cmd() {// update monitor pixel by definition
	monitor[hw_regs[19]][hw_regs[20]] = hw_regs[21];
	printf("changed monitor value to %d", hw_regs[21]);
}

void write_monitor_file(char* file_name) {
	FILE *monitorFile = fopen(file_name, "w");
	for (int y = 0; y < PIXELS_Y; y++) {
		for (int x = 0; x < PIXELS_X; x++) {
			fprintf(monitorFile, "%02X\n", monitor[x][y]);
		}

	}


}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ INTERRUPTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// checking if any interupt is on - function called every clock cycle
static int check_signal() {
	int irq;
	irq = (hw_regs[0] && hw_regs[3]) || (hw_regs[1] && hw_regs[4]) || (hw_regs[2] && hw_regs[5]);
	return irq;
}

//===========================interrupts=========================

/* setting registers properly before moving into interput routine
function is called if we are not in interput routine  allready
or the current PC instruction is not imm
*/
static void move_to_interrupt_Routine() {
	char* curr_inst = instructions[pc];
	if ( (interrupt_routine == 0)) {	// curr instruction is not imm -> move to interrput routine given we not handeling interrupt allready
		hw_regs[7] = pc;
		pc = hw_regs[6];
		interrupt_routine = 1;
	}
}

void irq2_handler() {
	char line[6];
	hw_regs[5] = 0;
	if (next_irq2 == -1) {
		if (fgets(line, 6, irq2in) != NULL) {
			next_irq2 = atoi(line);
		}
	}
	if (next_irq2 == hw_regs[8] ) {
		hw_regs[5] = 1;
		if (fgets(line, 6, irq2in) != NULL) {
			next_irq2 = atoi(line);
		}
	}
	else if (next_irq2 == hw_regs[8]-1 && interrupt_routine == 0) {
		hw_regs[5] = 1;
		if (fgets(line, 6, irq2in) != NULL) {
			next_irq2 = atoi(line);
		}
		if (check_signal() == 1) {
			move_to_interrupt_Routine();
		}
	}
}

void interrupt_handler() {
	if (hw_regs[17] == 1) { disk_timer++; }
	if (hw_regs[18] == 1) {
		monitor_cmd();
		hw_regs[18] = 0;
	}

	if (disk_timer >= 1024) {//enought time past from last disk command 
			hw_regs[17] = 0;
			hw_regs[14] = 0;
			hw_regs[4] = 1;
	}
	if (check_signal() == 1) {
		move_to_interrupt_Routine();
	}
	irq2_handler();

}

void clock_counter() {
	if (hw_regs[8] <= MAX_CLOCK) {
		hw_regs[8]++;
	}
	else { hw_regs[8] = 0; }
}

//========================HANDLE===========================
bool handle_cmd(int pc_index, bool is_imm) { // returns True if branch command and taken
	//char cmd[7] = instructions[pc_index];
	char op[3] = { instructions[pc_index][0],instructions[pc_index][1],'\0' };
	char rd[2] = { instructions[pc_index][2], '\0' };
	char rs[2] = { instructions[pc_index][3], '\0' };
	char rt[2] = { instructions[pc_index][4], '\0' };
	int op_num = strtol(op, NULL, 16);
	int rd_num = strtol(rd, NULL, 16);
	int rs_num = strtol(rs, NULL, 16);
	int rt_num = strtol(rt, NULL, 16);
	//operations
	if (op_num == 0) {//add
		proc_regs[rd_num] = proc_regs[rs_num] + proc_regs[rt_num];
		return false;
	}
	if (op_num == 1) {//sub
		proc_regs[rd_num] = proc_regs[rs_num] - proc_regs[rt_num];
		return false;
	}
	if (op_num == 2) {//and
		proc_regs[rd_num] = proc_regs[rs_num] & proc_regs[rt_num];
		return false;
	}
	if (op_num == 3) {//or
		proc_regs[rd_num] = proc_regs[rs_num] | proc_regs[rt_num];
		return false;
	}
	if (op_num == 4) {//xor
		proc_regs[rd_num] = proc_regs[rs_num] ^ proc_regs[rt_num];
		return false;
	}
	if (op_num == 5) {//mul
		proc_regs[rd_num] = proc_regs[rs_num] * proc_regs[rt_num];
		return false;
	}
	if (op_num == 6) {//sll
		proc_regs[rd_num] = proc_regs[rs_num] << proc_regs[rt_num];
		return false;
	}
	if (op_num == 7) {//sra
		proc_regs[rd_num] = (int)proc_regs[rs_num] >> (int)proc_regs[rt_num];
		return false;
	}
	if (op_num == 8) {//srl
		proc_regs[rd_num] = proc_regs[rs_num] >> proc_regs[rt_num];
		return false;
	}
	if (op_num == 9) {//beq
		if (proc_regs[rs_num] == proc_regs[rt_num]) { 
			pc = proc_regs[rd_num] & PC_MASK;//taking te lowest 10 bits (3FF in hex is 1023 in dec an 10 1's in binary)
			return true;
		}
		return false;
	}
	if (op_num == 10) {//bne
		if (proc_regs[rs_num] != proc_regs[rt_num]) { 
			pc = proc_regs[rd_num] & PC_MASK;
			return true;
		}
		return false;
	}
	if (op_num == 11) {//blt
		if (proc_regs[rs_num] < proc_regs[rt_num]) { 
			pc = proc_regs[rd_num] & PC_MASK;
			return true;
		}
		return false;
	}
	if (op_num == 12) {//bgt
		if (proc_regs[rs_num] > proc_regs[rt_num]) { 
			pc = proc_regs[rd_num] & PC_MASK;
			return true;
		}
		return false;
	}
	if (op_num == 13) {//ble
		if (proc_regs[rs_num] <= proc_regs[rt_num]) { 
			pc = proc_regs[rd_num] & PC_MASK;
			return true;
		}
		return false;
	}
	if (op_num == 14) {//bge
		if (proc_regs[rs_num] >= proc_regs[rt_num]) { 
			pc = proc_regs[rd_num] & PC_MASK;
			return true;
		}
		return false;
	}
	if (op_num == 15) {//jal
		if (is_imm) { proc_regs[15] = pc_index + 2; }
		else { proc_regs[15] = pc_index + 1; }
		pc = proc_regs[rd_num] & PC_MASK;
		return true;
	}
	if (op_num == 16) {//lw
		proc_regs[rd_num] = memory[(proc_regs[rs_num] + proc_regs[rt_num]) % MEMSIZE];
		return false;
	}
	if (op_num == 17) {//sw
		memory[(proc_regs[rs_num] + proc_regs[rt_num]) % MEMSIZE] = proc_regs[rd_num];
		return false;
	}
	if (op_num == 18) {//reti
		pc = hw_regs[7];
		interrupt_routine = 0;
		return true;
	}

	if (op_num == 19) {//in
		proc_regs[rd_num] = hw_regs[proc_regs[rs_num + rt_num]];
		write_hwRegTrace('r', proc_regs[rs_num + rt_num], hw_regs[proc_regs[rs_num + rt_num]]);
		return false;
	}
	if (op_num == 20) {//out
		hw_regs[proc_regs[rs_num + rt_num]] = proc_regs[rd_num];
		write_hwRegTrace('w', proc_regs[rs_num + rt_num], proc_regs[rd_num]);
		return false;
	}

	if (op_num == 21) {//halt
		pc = total_lines + 1;
		return true;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MAIN ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char** argv[]) {
	init_memory(argv[2]);
	init_disk(argv[3]);
	total_lines = update_instructions(argv[1]);
	//printf(res);
	//main loop
	trace_file = fopen(argv[7], "w+");
	hwRegTraceFile = fopen(argv[8], "w+");
	leds_file = fopen(argv[10], "w+");
	irq2in = fopen(argv[4], "r");
	while (pc < total_lines) {
		printf("%03X   %d\n", pc, hw_regs[8]);
		
 		interrupt_handler();	
		bool is_imm = is_immediate(instructions[pc]);

		if (is_imm) { proc_regs[1] = update_imm(instructions[pc+1]); }//update imm value
		update_trace();
		bool branch  = handle_cmd(pc, is_imm);
		if (is_imm && !branch) {
			pc++;
		}
		if(is_imm){
			clock_counter();
		}
		if(!branch){ pc++; }
		tot_instructions_done++;
		clock_counter();
	}
	write_dmem_out(argv[5]);
	write_regout(argv[6]);
	write_cycles(argv[9]);
	write_diskout(argv[13]);
	write_monitor_file(argv[11]);
	fclose(leds_file);
	fclose(hwRegTraceFile);
	fclose(irq2in);
	fclose(trace_file);
	return 0;
}



