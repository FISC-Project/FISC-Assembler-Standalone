/*
 * fisc_assembly.cpp
 *
 *  Created on: 08/10/2016
 *      Author: Miguel
 */
#include <stdarg.h>
#include <string.h>
#include <algorithm>
#include <fisc_assembly.h>
#include <fisc_opcodes.h>

extern void yyerror(const char * str);

std::vector<instruction_t> program;
std::map<std::string, unsigned int> label_lst;

char validate_instruction(char * mnemonic, unsigned int opcode, arglist_t * args) {
	/* Grab instruction argument formats: */
	std::vector<afmt> * argfmts = 0;
	for (auto it = instruction_lookup.begin(); it != instruction_lookup.end(); ++it) {
		std::string mnemonic_str(mnemonic);

		std::transform(mnemonic_str.begin(), mnemonic_str.end(), mnemonic_str.begin(), ::tolower);
		std::transform(it->second.first.mnemonic.begin(), it->second.first.mnemonic.end(), it->second.first.mnemonic.begin(), ::tolower);

		if (!strcmp(it->second.first.mnemonic.c_str(), mnemonic_str.c_str())) {
			argfmts = &it->second.second;
			break;
		}
	}

	if(!argfmts) return 0;

	/* Check if it's valid: */
	if(argfmts->size() != args->argcount) return 0; /* Argument count does not match */
	for(int i = 0; i < argfmts->size(); i++) {
		afmt fmt = (*argfmts)[i];
		/* Check if fields match: */
		if(fmt.arg_type != args->arguments[i]->arg_type)   return 0;
		if(fmt.is_offset != args->arguments[i]->is_offset) return 0;
	}
	return 1;
}

char make_instruction(char * mnemonic, arglist_t * args) {
	char success = 0;
	if(!args) return success;
	instruction_t instr;

	for (auto it = instruction_lookup.begin(); it != instruction_lookup.end(); ++it) {
		std::string mnemonic_str(mnemonic);

		std::transform(mnemonic_str.begin(), mnemonic_str.end(), mnemonic_str.begin(), ::tolower);
		std::transform(it->second.first.mnemonic.begin(), it->second.first.mnemonic.end(), it->second.first.mnemonic.begin(), ::tolower);

		if (!strcmp(it->second.first.mnemonic.c_str(), mnemonic_str.c_str())) {
			success = 1;
	    	instr.opcode = it->first;
	    	break;
	    }
	}

	if(!success) {
		char msg[100];
		sprintf(msg, "Instruction '%s' is non existant.",  mnemonic);
		free_argument_list(args);
		free(mnemonic);
		yyerror(msg);
	}

	if((success = validate_instruction(mnemonic, instr.opcode, args))) {
		instr.mnemonic = mnemonic;
		instr.args = args;
		program.push_back(instr);
	} else {
		char msg[100];
		sprintf(msg, "Instruction '%s' is malformed.",  mnemonic);
		free_argument_list(args);
		free(mnemonic);
		yyerror(msg);
	}
	return success;
}

void free_argument_list(arglist_t * args) {
	for(int i=0;i<args->argcount;i++) {
		argument_t * arg = args->arguments[i];
		free(arg);
	}
	free(args->arguments);
	free(args);
}

argument_t * make_argument(char arg_type, char is_offset, long long value) {
	argument_t * arg = (argument_t*)malloc(sizeof(argument_t));
	arg->arg_type = arg_type;
	arg->is_offset = is_offset;
	arg->value = value;
	arg->label = 0;
	return arg;
}

argument_t * make_argument(char arg_type, char is_offset, char * label) {
	argument_t * arg = (argument_t*)malloc(sizeof(argument_t));
	arg->arg_type = arg_type;
	arg->is_offset = is_offset;
	arg->value = -1;
	arg->label = label;
	return arg;
}

arglist_t * make_argument_list(unsigned int argcount, ...) {
	if(!argcount) return 0;

	arglist_t * ret = (arglist_t*)malloc(sizeof(arglist_t));
	ret->arguments = (argument_t**)malloc(sizeof(argument_t**) * argcount);
	ret->argcount = argcount;

	va_list vl;
	va_start(vl, argcount);
	for(int i = 0; i < argcount; i++)
		ret->arguments[i] = (argument_t*)va_arg(vl, argument_t*);
	return ret;
}

void add_label(std::string label_name, unsigned int line_number) {
	if(label_lst.find(label_name) == label_lst.end())
		label_lst.insert(std::pair<std::string, unsigned int>(label_name, line_number));
}

void resolve_labels() {
	for(int i = 0; i < program.size(); i++) {
		for(int j = 0; j < program[i].args->argcount; j++) {
			argument_t * arg = program[i].args->arguments[j];
			if(arg->label) {
				/* Search in the label map for this label: */
				char found = 0;
				for (auto it = label_lst.begin(); it != label_lst.end(); ++it) {
					if (!strcmp(it->first.c_str(), arg->label)) {
				    	arg->value = it->second;
				    	found = 1;
				    	break;
				    }
				}
				if(!found) {
					char msg[100];
					sprintf(msg, "Could not find label '%s'.", arg->label);
					yyerror(msg);
				}
			}
		}
	}
}