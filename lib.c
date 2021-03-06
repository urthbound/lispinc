#include "lib.h"

/* list operations */

#define list_count 4

#define cons \
	"("DEF_KEY" cons \
		("FUN_KEY" (x y) \
			("FUN_KEY" (s) \
				(s x y))))"

#define car \
	"("DEF_KEY" car \
		("FUN_KEY" (p) \
			(p ("FUN_KEY" (x y) \
					x))))"

#define cdr \
	"("DEF_KEY" cdr \
		("FUN_KEY" (p) \
			(p ("FUN_KEY" (x y) \
					y))))"

#define nil \
	"("DEF_KEY" nil ("QUOTE_KEY" ()))"

/* arithmetic operations */

#define arith_count 8

#define zero_ \
	"("DEF_KEY" zero? \
		("FUN_KEY" (n) \
			(= n 0)))"

#define add1 \
	"("DEF_KEY" add1 \
		("FUN_KEY" (n) \
			(+ n 1)))"

#define sub1 \
	"("DEF_KEY" sub1 \
		("FUN_KEY" (n) \
			(- n 1)))"

#define triangular \
	"("DEF_KEY" triangular \
		("FUN_KEY" (n) \
			("IF_KEY" (zero? n) \
				0 \
				(+ n (triangular (sub1 n))))))"

#define tetrahedral \
	"("DEF_KEY" tetrahedral \
		("FUN_KEY" (n) \
			("IF_KEY" (zero? n) \
				0 \
				(+ (triangular n) \
					(tetrahedral (sub1 n))))))"

#define supertetrahedral \
	"("DEF_KEY" supertetrahedral \
		("FUN_KEY" (n) \
			("IF_KEY" (zero? n) \
				0 \
				(+ (tetrahedral n) \
					(supertetrahedral (sub1 n))))))"

#define fact_rec \
	"("DEF_KEY" recursive_factorial \
		("FUN_KEY" (n) \
			("IF_KEY" (zero? n) \
				1 \
				(* n (recursive_factorial (sub1 n))))))"

#define fact_iter \
	"("DEF_KEY" iterative_factorial \
		("FUN_KEY" (n) \
			("BEGIN_KEY" \
				("DEF_KEY" loop \
					("FUN_KEY" (count total) \
						("IF_KEY" (zero? count) \
							total \
							(loop (sub1 count) \
								(* total count))))) \
				(loop n 1))))"

/* newlines are needed because of some quirk in the
	parsing process (see read.c and parse.c) */

char* library[] = {cons"\n",
					 car"\n", 
					 cdr"\n", 
					 nil"\n", 
					 zero_"\n",
					 add1"\n", 
					 sub1"\n", 
					 triangular"\n", 
					 tetrahedral"\n", 
					 supertetrahedral"\n", 
					 fact_rec"\n", 
					 fact_iter"\n"};

/* lib_len should match the length of library
	(this has to be handled manually?) */

int lib_len = list_count + arith_count;


/* library loading */

int lib_counter = 0;

char* load_library(void) {	
			if (DEBUG) print_lib();
	char* lib_entry = library[lib_counter];
	lib_counter++;
	return lib_entry;
}

void print_lib(void) {
	char* lib_entry = library[lib_counter];
	printf("loading library entry: %s\n", lib_entry);
}

bool lib_loaded(void) {
	return lib_counter >= lib_len;
}
