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

static int pc = 0;
static int instructions_count = 0;
static int proc_regs[REGSNUM];//updates the values of the processor registers
static int hw_regs[HWREGS];//updates the values of the hardware registers

static char* update_trace(char *inst) {
	int i = 0;
	char res[160];
}