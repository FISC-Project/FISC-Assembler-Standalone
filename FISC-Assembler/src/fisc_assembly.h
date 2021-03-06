/*
 * fisc_assembly.h
 *
 *  Created on: 08/10/2016
 *      Author: Miguel
 */

#ifndef SRC_FISC_ASSEMBLY_H_
#define SRC_FISC_ASSEMBLY_H_

#include <utility>
#include <vector>
#include <string>
#include <map>
#include <bitset>

typedef struct argument {
	char arg_type;       /* Is it a register or immediate? */
	char is_offset;      /* Is this argument inside an offset? example: [X0, #0], X0 and #0 are both inside an offset */
	long long value;     /* The meaning of the value depends on the arg_type */
	char * label;        /* This is only used by labels, and its purpose is for the program to find AFTER parsing what the label value is */
	char shift_quadrant; /* The label's value will be inserted into the value field, but needs a shift value to do so */
} argument_t;

typedef struct argfmt {
	char arg_type;
	char is_offset;
} afmt;

typedef struct instruction_fmt {
	std::string mnemonic;
	char fmt;
	unsigned int pseudo_opcode; /* If non zero, then this instruction is a pseudo instruction */
	char width; /* How many bits does this instruction handle? (if 0 then it will be 64 by default) */
	char nullargs; /* Is the instruction made out of just the opcode? (0: No. 1: Yes) Example: RETI */
} ifmt;

typedef struct arglist {
	argument_t ** arguments;
	unsigned int argcount;
} arglist_t;

typedef struct instruction {
	char       * mnemonic;
	unsigned int opcode;
	arglist_t  * args;
} instruction_t;

typedef struct ifmt_r {
	unsigned rd : 5;
	unsigned rn : 5;
	unsigned shamt : 6;
	unsigned rm : 5;
	unsigned opcode : 11;
} ifmt_r_t;

typedef struct ifmt_i {
	unsigned rd : 5;
	unsigned rn : 5;
	unsigned alu_immediate : 12;
	unsigned opcode : 10; // The lower 1 bit is discarded, it's like an X (don't care)
} ifmt_i_t;

typedef struct ifmt_d {
	unsigned rt : 5;
	unsigned rn : 5;
	unsigned op : 2;
	unsigned dt_address : 9;
	unsigned opcode : 11;
} ifmt_d_t;

typedef struct ifmt_b {
	unsigned br_address : 26;
	unsigned opcode : 6; // The lower 5 bits are discarded, they are like X's (don't cares)
} ifmt_b_t;

typedef struct ifmt_cb {
	unsigned rt : 5;
	unsigned cond_br_address : 19;
	unsigned opcode : 8; // The lower 3 bits are discarded, they are like X's (don't cares)
} ifmt_cb_t;

typedef struct ifmt_iw {
	unsigned rt : 5;
	unsigned mov_immediate : 16;
	unsigned opcode : 11;
} ifmt_iw_t;

extern std::vector<instruction_t> program;
extern std::vector<std::bitset<32> > program_bin;
extern std::string program_str;
extern std::string program_str_ihex;
extern std::map<unsigned int, std::pair<ifmt, std::vector<afmt> > > instruction_lookup;
extern std::map<std::string, unsigned long long> label_lst;

extern char make_instruction(char * mnemonic, arglist_t * args);
extern arglist_t * make_argument_list(unsigned int argcount, ...);
extern argument_t * make_argument(char arg_type, char is_offset, long long value);
extern argument_t * make_argument(char arg_type, char is_offset, char * label);
extern argument_t * make_argument(char arg_type, char is_offset, char * label, char shift_quadrant);
extern void free_argument_list(arglist_t * args);
extern void add_label(std::string label_name, unsigned int line_number);
extern void resolve_labels();
extern void adjust_labels_offset(unsigned int offset_amount);
extern std::bitset<32> instruction_to_binary(instruction_t * instr);

#endif /* SRC_FISC_ASSEMBLY_H_ */
