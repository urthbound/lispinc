/*
	ENV

	The evaluation environment is a lookup
	table wherein names (variables) are bound
	to values. The heart of Lisp is lambdas,
	and lambdas are evaluated by extending
	the enviroment with the lambdas variables
	bound to its arguments.

	Typewise, an Env is a struct containing 
	a pointer to another Env (its 'enclosing
	enviroment' or 'enclosure') and a pointer
	to a Frame. A Frame is a linked list of
	key / value pairs. For convenience, both
	keys and values are wrapped in the Obj
	type (see objects.h for definitions).

	[ add a diagram here? ]

	Functions

	makeBaseEnv returns a pointer to an
	enviroment with basic arithmetic operations
	defined. More primitive functions can be
	added later.

	lookup takes two Objs as arguments, the
	first of type NAME and the second of type
	ENV. It looks up the name in the env and
	returns the bound value.

	defineVar and setVar each take three Objs as
	arguments, with the first of type NAME and 
	the third of type ENV (the second can be
	anything.) setVar sets the first occurence
	of the name in the env to the value (raising
	an error if the name is unbound), while
	defineVar sets the name to the value in the
	'topmost' level of the environment, adding
	a new Frame binding if the name is unboind.

	extendEnv takes two List Objs (the first being
	a List of NAME Objs) and an Env Obj and adds
	a returns a new Env Obj, the frame of which is
	the two Lists zipped together and the enclosure
	of which is the first Env.

	Note that these functions all take Objs as
	arguments: this is because they have to
	interface the registers of the evaluator,
	which are all of type Obj. In general, the
	functions in this file that operate on types
	other than Obj are 'private' functions, and
	the Obj-valued functions are 'public'.
	EDIT: changed makeBaseEnv type
*/

/*
	TODO:
		-- markings for distinct envs
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "objects.h"
#include "env.h"
#include "flags.h"
#include "mem.h"

/* base_env established separately to 
	persist through repl */

Env* base_env;

/* primitive functions */

int add_func(int a, int b) {
	return a + b;
}

int sub_func(int a, int b) {
	return a - b;
}

int mul_func(int a, int b) {
	return a * b;
}

int div_func(int a, int b) {
	return a / b; // floor division
}

int eq_func(int a, int b) {
	if (a == b)
		return 1;
	else
		return 0;
}

intFunc add_ = add_func;
intFunc sub_ = sub_func;
intFunc mul_ = mul_func;
intFunc div_ = div_func;
intFunc eq_ = eq_func;

/* env builders */

#define PRIM_ADD "+"
#define PRIM_SUB "-"
#define PRIM_MUL "*"
#define PRIM_DIV "/"
#define PRIM_EQ "=" 

// returns pointer to base_env
Env* makeBaseEnv(void) {
	List* function_vars = 
		makeList(NAMEOBJ(PRIM_ADD), 
			makeList(MKOBJ(NAME, name, PRIM_SUB), 
				makeList(MKOBJ(NAME, name, PRIM_MUL), 
					makeList(MKOBJ(NAME, name, PRIM_DIV), 
						makeList(MKOBJ(NAME, name, PRIM_EQ), NULL)))));

	List* function_vals = 
		makeList(MKOBJ(FUNC, func, add_), 
			makeList(MKOBJ(FUNC, func, sub_), 
				makeList(MKOBJ(FUNC, func, mul_), 
					makeList(MKOBJ(FUNC, func, div_), 
						makeList(MKOBJ(FUNC, func, eq_), NULL)))));

	Frame* primitives = makeFrame(function_vars, function_vals);

	Env* env = makeEnv(primitives, NULL);

	append_to_envs(env);

	return env;
}

// returns new env obj with vars bound to vals
Obj extendEnv(Obj vars_obj, Obj vals_obj, Obj base_env_obj) {
	List* vars = vars_obj.val.list;
	List* vals = vals_obj.val.list;
	Env* base_env = base_env_obj.val.env;

	Frame* frame = makeFrame(vars, vals);
	Env* ext_env = makeEnv(frame, base_env);

	append_to_envs(ext_env);

	return ENVOBJ(ext_env);
}


/* lookup in env */

Obj lookup(Obj var_obj, Obj env_obj) {
	if (DEBUG) printf("looking up \"%s\"\n", var_obj.val.name);
	char* var = var_obj.val.name;
	Env* env = env_obj.val.env;

	return lookup_in_env(var, env);
}

/* modify env */

/* adds new var/val binding to env
(doesn't check for existing binding) */
void defineVar(Obj var_obj, Obj val_obj, Obj* env_obj) {
	char* var = var_obj.val.name;
	Env* env = (*env_obj).val.env;

	Frame* frame = malloc(sizeof(Frame));
	frame->key = var;
	frame->val = val_obj;
	frame->next = env->frame;
	env->frame = frame;
	return;
}

// sets first occurence of var to val
void setVar(Obj var_obj, Obj val_obj, Obj env_obj) {
	char* var = var_obj.val.name;
	Env* env = env_obj.val.env;

	if (env == NULL) {
		printf("unbound variable -- setVar\n");
		return;
	}

	Frame* frame = env->frame;

	while (frame != NULL) {

		if (strcmp(var, frame->key) == 0) {
			frame->val = val_obj;
			return;
		}

		frame = frame->next;
	}

	Obj enclosure = ENVOBJ(env->enclosure);

	setVar(var_obj, val_obj, enclosure);
}

/* constructors */

Env* makeEnv(Frame* frame, Env* enclosure) {
	Env* env = malloc(sizeof(Env));
	env->frame = frame;
	env->enclosure = enclosure;
	return env;
}

// zip-like
Frame* makeFrame(List* vars, List* vals) {
	if (vars == NULL)
		return NULL;

	char* key = vars->car.val.name;
	Obj val = vals->car;

	Frame* frame = malloc(sizeof(Frame));

	frame->key = key;
	frame->val = val;

	frame->next = makeFrame(vars->cdr, vals->cdr);

	return frame;
}

// cons-like
List* makeList(Obj car, List* cdr) {
	List* list = malloc(sizeof(List));
	list->car = car;
	list->cdr = cdr;
	return list;
}

/* lookup helpers */

Obj lookup_in_env(char* var, Env* env) { // lookup in env
			if (DEBUG) printf("%s\n", "looking up in env...");
	if (env == NULL) {
			if (DEBUG) printf("%s\n", "null env, returning DUMMY");
		return DUMMYOBJ;
	}

	Frame* frame = env->frame;
	Obj checkFrame = lookup_in_frame(var, frame);

	if (checkFrame.tag != DUMMY)
		return checkFrame;
	else
		return lookup_in_env(var, env->enclosure);
}

Obj lookup_in_frame(char* var, Frame* frame) { // helper for lookup
			if (DEBUG) printf("%s\n", "looking up in frame...");
	if (frame == NULL)
		return DUMMYOBJ;

	char* key = frame->key;

	if (strcmp(var, key) == 0)
		return (frame->val);
	else
		return lookup_in_frame(var, frame->next);
}

