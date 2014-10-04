/**
 * output.c
 * Author: Jan Viktorin <xvikto03 (at) stud.fit.vutbr.cz>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include "pico.h"
#include "output.h"
#include "assembler.h"
#include "stab.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

struct output {
	FILE *file;
	code_t code[PROGRAM_LEN];
};

bool output_init(struct pico *p, char *filename)
{
	FILE *out = stdout;

	if(filename != NULL) {
		out = fopen(filename, "w");
		if(out == NULL) 
			return error(NULL, "Can not open the output file");
	}

	struct output *ins = (struct output *) calloc(1, sizeof(struct output));
	if(ins == NULL) {
		fclose(out);
		return error(NULL, "Memory allocation error");
	}

	ins->file = out;
	p->output = ins;
	return true;
}

void output_destroy(struct pico *p)
{
	if(p->output == NULL)
		return;

	if(p->output->file != NULL)
		fclose(p->output->file);

	free(p->output);
	p->output = NULL;
}

bool output_code(struct pico *p, progaddr_t address, code_t ins)
{
	debug_here();
	if(address >= PROGRAM_LEN)
		return error(p, "Unexpected program address, max 1023");

	p->output->code[address] = ins;
	return true;
}

void output_flush(struct pico *p)
{
	debug_here();
		//using namespace std;
	const char HEX_CHARS_PER_LINE = 64;
	const char SHORTS_PER_LINE = HEX_CHARS_PER_LINE/4;
	const char LINE_LENGTH = SHORTS_PER_LINE*4+3;
	char line[LINE_LENGTH];

	for(int i=0; i<LINE_LENGTH; i++)
		line[i]='0';
	line[LINE_LENGTH-1] = 0;
	//ostringstream file;
	fprintf(p->output->file, "`timescale 1 ps / 1ps\r\n");
	fprintf(p->output->file, "module program (address, instruction, clk);\r\n");
	fprintf(p->output->file, "  input [9:0] address;\r\n");
	fprintf(p->output->file, "  input clk;\r\n");
	fprintf(p->output->file, "  output [17:0] instruction;\r\n");
	fprintf(p->output->file, "  RAMB16_S18 ram_1024_x_18(\r\n");
	fprintf(p->output->file, "    .DI  (16'h0000),\r\n");
	fprintf(p->output->file, "    .DIP  (2'b00),\r\n");
	fprintf(p->output->file, "    .EN (1'b1),\r\n");
	fprintf(p->output->file, "    .WE (1'b0),\r\n");
	fprintf(p->output->file, "    .SSR (1'b0),\r\n");
	fprintf(p->output->file, "    .CLK (clk),\r\n");
	fprintf(p->output->file, "    .ADDR (address),\r\n");
	fprintf(p->output->file, "    .DO (instruction[15:0]),\r\n");
	fprintf(p->output->file, "    .DOP (instruction[17:16]));\r\n");

	for(int i = 0; i < PROGRAM_LEN; i++) {
	char lineNumber = i/SHORTS_PER_LINE;
	char byte_number = i%(SHORTS_PER_LINE);
	char line_index = LINE_LENGTH-1-4*(1+byte_number);
		char intString[6] = "0000";
		sprintf(intString, "%.4X", (0xFFFF&p->output->code[i]));
		for(int j=0;j<4;j++) {
			line[line_index+j] = intString[j]; 
		}
		//fprintf(p->output->file,line);
		if(byte_number == SHORTS_PER_LINE-1)
					fprintf(p->output->file, "  defparam ram_1024_x_18.INIT_%.2X = 256'h%s;\r\n",lineNumber,line);
	}

	// No idea what this stuff is:
	fprintf(p->output->file, "  // No idea what this stuff is: \r\n");
	fprintf(p->output->file, "  defparam ram_1024_x_18.INITP_00 = 256'h0000000000000000000000000000000000000000000000000000000000003A5F;\r\n");
	fprintf(p->output->file, "  defparam ram_1024_x_18.INITP_01 = 256'h0000000000000000000000000000000000000000000000000000000000000000;\r\n");
	fprintf(p->output->file, "  defparam ram_1024_x_18.INITP_02 = 256'h0000000000000000000000000000000000000000000000000000000000000000;\r\n");
	fprintf(p->output->file, "  defparam ram_1024_x_18.INITP_03 = 256'h0000000000000000000000000000000000000000000000000000000000000000;\r\n");
	fprintf(p->output->file, "  defparam ram_1024_x_18.INITP_04 = 256'h0000000000000000000000000000000000000000000000000000000000000000;\r\n");
	fprintf(p->output->file, "  defparam ram_1024_x_18.INITP_05 = 256'h0000000000000000000000000000000000000000000000000000000000000000;\r\n");
	fprintf(p->output->file, "  defparam ram_1024_x_18.INITP_06 = 256'h0000000000000000000000000000000000000000000000000000000000000000;\r\n");
	fprintf(p->output->file, "  defparam ram_1024_x_18.INITP_07 = 256'hC000000000000000000000000000000000000000000000000000000000000000;\r\n");

	fprintf(p->output->file, "endmodule ");

	/*
	// create a file-reading object
	ifstream file_in;
	file_in.open("output.hex"); // open a file
	if (!file_in.good()) {
		cout << "Error finding output.hex" << endl;
		return 1; // exit if file not found
	}
	char index = 0;
	while(!file_in.eof()) {
		ostringstream line;
		file << "defparam ram_1024_x_18.INIT_" << setw(2) << setfill('0') << hex << (int)index << " = " ;
		int i;
		for(i=0; i<NUMBER_OF_HEX_PER_VERILOG_LINE && !file_in.eof(); i+=4) {
			char buf[MAX_CHARS_PER_LINE];
			file_in.getline(buf, MAX_CHARS_PER_LINE);
			const string &temp = line.str();
			line.seekp(0);
			line << buf[1] << buf[2] << buf[3] << buf[4] << temp;
		}
		while(i<NUMBER_OF_HEX_PER_VERILOG_LINE) {
			i++;
			line << "0";
		}
		file << "256'h" << line.str() << ";" << endl << endl;
		index++;
	}
	cout << file.str();*/
}


