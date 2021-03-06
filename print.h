/*
	PRINT

	Some print functions, mostly used for debugging.
	In general, the low-level print functions 
	print_obj and print_list shouldn't be called
	directly, debug_register being preferred.
*/

/*
	TODO:
		-- figure out how to mark envs
			so that printing them is informative
*/

#ifndef PRINT_GUARD
#define PRINT_GUARD

#include <stdio.h>

#include "keywords.h"
#include "objects.h"
#include "flags.h"
#include "registers.h"
#include "stack.h"

#define NL printf("\n");
#define TAB printf("\t");
#define PRDIV printf("--------------------\n");

/* evaluator info printing */

void print_info(void);
void print_final_val(void);
void print_stats(void);

void print_stack(void);
void print_registers(void);

void print_obj(Obj obj);
void print_list(List* list);
void print_label(Label label);
char* lookup_prim_name(Obj func_obj);

/* user interface */

void print_intro(void);
void print_help(void);
void print_flags(void);

#endif